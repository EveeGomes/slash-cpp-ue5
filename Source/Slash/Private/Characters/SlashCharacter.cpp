// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"

/** Hair and Eyebrow  */
#include "GroomComponent.h"

/** Setup movement */
#include "GameFramework/CharacterMovementComponent.h"

/** Enhanced Input System */
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

/** Camera and Spring Arm */
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

/** Set collision profile */
#include "Components/StaticMeshComponent.h"

/** Naive approach to attach the weapon */
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"

/** For using Animation Montage in Attack() */
#include "Animation/AnimMontage.h"

/** Pawn Sensing component */
#include "Perception/PawnSensingComponent.h"

/** Check if it has detected the combat target */
#include "Slash/DebugMacros.h"

/** To rotate the camera to Combat Tagert */
#include "Kismet/KismetMathLibrary.h"

/** To use SphereTraceMultiObjects */
#include "Kismet/KismetSystemLibrary.h"

#include "Enemy/Enemy.h"

/** Used in InitializeSlashOverlay() to access and modify the HUD */
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Components/AttributeComponent.h"

void ASlashCharacter::InitializeSlashOverlay(APlayerController* PlayerController)
{
	ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());
	if (SlashHUD)
	{
		SlashOverlay = SlashHUD->GetSlashOverlay();
		if (SlashOverlay && Attributes)
		{
			SlashOverlay->SetHealthPercent(Attributes->GetHealthPercent());
			// hardcoding until we add it to Attributes
			SlashOverlay->SetStaminaPercent(1.f);
			SlashOverlay->SetGold(0);
			SlashOverlay->SetSouls(0);
		}
	}
}

void ASlashCharacter::SetHUDHealth()
{
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

bool ASlashCharacter::CanJump()
{
	return ActionState <= EActionState::EAS_Unoccupied;
}

void ASlashCharacter::SphereTrace()
{
	const FVector SlashLocation = GetActorLocation();
	FVector CameraFwd = ViewCamera->GetForwardVector(); // a bit different from YT
	FVector End = (CameraFwd * 500.f) + SlashLocation;

	// Objects to trace against
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn)); // WorlDynamic or Pawn?

	TArray<AActor*> ActorsToIgnore;
	FHitResult HitActor;

	UKismetSystemLibrary::SphereTraceSingleForObjects(
		this,
		SlashLocation,
		End,
		125.f,
		ObjectTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitActor,
		true
	);
	// add the hit actor to the ActorsToIgnore? Or it's not necessary as this function is called only 
	//  when TAB is pressed?

	CombatTarget = HitActor.GetActor();
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController()); //Controller);
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashContext, 0);
		}

		InitializeSlashOverlay(PlayerController);
	}

	/** 
	* Use the Tags variable from the Character and Add method to add a tag which can get any name we want.
	*/
	Tags.Add(FName("EngageableTarget"));

}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState > EActionState::EAS_Unoccupied) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();

	// Using the following implementation for a directional movement
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);	
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void ASlashCharacter::Jump()
{
	if (CanJump())
	{
		Super::Jump();
	}
}

void ASlashCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		EquipWeapon(OverlappingWeapon);
	}
	else
	{
		if (CanDisarm())
		{
			Disarm();
		}
		else if (CanArm())
		{
			Arm();
		}
	}
}

void ASlashCharacter::Attack()
{
	Super::Attack();

	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::LockTarget()
{
	// && ActionState != EActionState::EAS_Locked // it's not already locked?
	if (CanLock())
	{
		// Engage lock
		SphereTrace();
		LockToTarget();
	}
	else
	{
		// Disangaged lock
		UnlockFromTarget();
		// which ActionState to return to? Unoccupied is the default state.
		ActionState = EActionState::EAS_Unoccupied;
	}

	/** 
	* TODO:
	* [x] Only allow this function to be called if the pawn is in sight's radius!
	* [x] Lock camera rotation to the target's movement instead of the mouse
	* [] Use a widget or niagara system to show it's targeted?
	* [] Check slash states?
	*/
}

void ASlashCharacter::LockToTarget()
{
	if (IsTargetEnemy())
	{
		// add a state so it can be used in transition rule from unlocked to locked locomotion?
		ActionState = EActionState::EAS_Locked;
		bLocked = true;
		bIsEnemy = true;
		
		Enemy = Cast<AEnemy>(CombatTarget);
		if (Enemy) Enemy->ShowLockedEffect();

		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;

		Controller->SetIgnoreLookInput(bLocked);
	}
}

bool ASlashCharacter::IsTargetEnemy()
{
	return CombatTarget && CombatTarget->ActorHasTag(FName("Enemy"));
}

bool ASlashCharacter::CanLock()
{
	return !bLocked && CharacterState > ECharacterState::ECS_Unequipped;
}

void ASlashCharacter::UnlockFromTarget()
{
	bLocked = false;
	CombatTarget = nullptr;
	bIsEnemy = false;

	if (Enemy) Enemy->HideLockedEffect();
	// should set Enemy to nullptr as well?

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	Controller->ResetIgnoreLookInput();
}

void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	if (Weapon)
	{
		Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		OverlappingItem = nullptr;
		EquippedWeapon = Weapon;
	}
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
	return (ActionState == EActionState::EAS_Unoccupied || 
			 ActionState == EActionState::EAS_Locked) &&
			 CharacterState != ECharacterState::ECS_Unequipped;
}

void ASlashCharacter::PlayEquipMontage(FName SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

bool ASlashCharacter::CanDisarm()
{
	return (ActionState == EActionState::EAS_Unoccupied ||
			 ActionState == EActionState::EAS_Locked) &&
			 CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanArm()
{
	return (ActionState == EActionState::EAS_Unoccupied || 
			 ActionState == EActionState::EAS_Locked) &&
			 CharacterState == ECharacterState::ECS_Unequipped &&
			 EquippedWeapon; // check if it's not a null pointer (meaning we had gotten a weapon already)
}

void ASlashCharacter::Disarm()
{
	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
	UnlockFromTarget();
}

void ASlashCharacter::Arm()
{
	PlayEquipMontage(FName("Equip"));
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::AttachWeaponToBack()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	/** Movement */
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	/** Static Mesh Collision Presets */
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	/** Spring arm and camera */
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->SetRelativeLocation(FVector(0, 0, 100));
	SpringArm->SetRelativeRotation(FRotator(-20, 0, 0));
	SpringArm->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	ViewCamera->SetupAttachment(SpringArm);

	/** Hair and Eyebrow */
	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");
}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ASlashCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
		EnhancedInputComponent->BindAction(LockOnTarget, ETriggerEvent::Started, this, &ASlashCharacter::LockTarget);
	}
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bLocked && Enemy && !Enemy->IsDead())
	{
		FVector SlashLocation = GetActorLocation();
		FVector LockedTargetLocation = CombatTarget->GetActorLocation();

		Controller->SetControlRotation(UKismetMathLibrary::FindLookAtRotation(SlashLocation, LockedTargetLocation));
	}
	if ((Enemy && Enemy->IsDead()) || IsOutOfRange())
	{
		UnlockFromTarget();
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();

	return DamageAmount;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

	/** 
	* The player shouldn't be able to attack while playing the Hit Reaction, ie while in HitReaction state!
	* So when it call Attack() and check CanAttack, the states won't allow the character to attack :)
	* 
	* We also need a way to change from HitReaction state. For that we'll use a BlueprintCallable function
	*  that'll be called from a AM.
	*/
	ActionState = EActionState::EAS_HitReaction;
}

bool ASlashCharacter::IsOutOfRange()
{
	if (CombatTarget)
	{
		const FVector SlashLocation = GetActorLocation();
		const FVector LockedTargetLocation = CombatTarget->GetActorLocation();

		float Distance = FVector::Dist(LockedTargetLocation, SlashLocation);

		return Distance > Range;
	}

	return false;
}
