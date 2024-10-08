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

/** Items to pick up */
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Items/Health.h"
#include "Items/Book.h"

/** Used in SpeedUp */
#include "Kismet/KismetMathLibrary.h"

void ASlashCharacter::InitializeSlashOverlay(APlayerController* PlayerController)
{
	ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());
	if (SlashHUD)
	{
		SlashOverlay = SlashHUD->GetSlashOverlay();
		if (SlashOverlay && Attributes)
		{
			SetHUDHealth();
			// hardcoding until we add it to Attributes
			SlashOverlay->SetStaminaBarPercent(1.f);
			SlashOverlay->SetGold(0);
			SlashOverlay->SetSouls(0);
		}
	}
}

void ASlashCharacter::SetHUDHealth()
{
	//if (SlashOverlay && Attributes) // the functions that calls this one already checks Attributes && SlashOverlay
	//{
	//	
	//}
	SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
}

void ASlashCharacter::SetHUDStamina()
{
	//if (Attributes && SlashOverlay) // the functions that calls this one already checks Attributes && SlashOverlay
	//{
	//	SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	//}
	SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
}

bool ASlashCharacter::CanJump()
{
	return ActionState <= EActionState::EAS_Unoccupied;
}

void ASlashCharacter::SphereTrace()
{
	const FVector SlashLocation = GetActorLocation();
	FVector CameraFwd = ViewCamera->GetForwardVector();
	FVector End = (CameraFwd * 500.f) + SlashLocation;

	// Objects to trace against
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

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

void ASlashCharacter::Dodge()
{
	if (!CanDodge() || !HasEnoughStamina()) return;

	PlayDodgeMontage();
	ActionState = EActionState::EAS_Dodge;
	if (Attributes && SlashOverlay)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
		// Update the HUD
		SetHUDStamina();
	}
}

void ASlashCharacter::SpeedUp(const FInputActionValue& Value)
{
	if (Attributes && Attributes->GetStamina() < Attributes->GetMaxStamina() * .10)
	{
		SetMaxWalkSpeed(600);
		return;
	}

	SetMaxWalkSpeed(1200);
	if (Attributes && SlashOverlay)
	{
		Attributes->UseStamina(Attributes->GetSpeedUpCost());
		SetHUDStamina();
	}
}

void ASlashCharacter::SetMaxWalkSpeed(const int32 Speed)
{
	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void ASlashCharacter::EndSpeedUp(const FInputActionValue& Value)
{
	SetMaxWalkSpeed(600);
}

void ASlashCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		/** 
		* Destroy current weapon if trying to equip/get another weapon when overlapping with it.
		* As we might be unequipped but with a weapon on our back, we need to destroy it but also return to equipped state!
		* That change to equipped state happens in EquipWeapon already.
		*/
		if (EquippedWeapon)
		{
			EquippedWeapon->Destroy();
		}

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

void ASlashCharacter::LeftButtonAttack()
{
	Super::Attack();

	if (CanAttack())
	{
		PlaySingleAttackMontage(FName("ClickAttack"));
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::OneKeyAttack()
{
	Super::Attack();

	if (CanAttack())
	{
		PlaySingleAttackMontage(FName("Attack1"));
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::TwoKeyAttack()
{
	Super::Attack();

	if (CanAttack())
	{
		PlaySingleAttackMontage(FName("Attack2"));
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::ThreeKeyAttack()
{
	Super::Attack();

	if (CanAttack())
	{
		PlaySingleAttackMontage(FName("Attack3"));
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::LockTarget()
{
	if (CanLock())
	{
		// Engage lock
		SphereTrace(); // CombatTarget set to enemy
		LockToTarget();
	}
	else
	{
		// Disangaged lock
		UnlockFromTarget(); // Enemy set to nullptr
		ActionState = EActionState::EAS_Unoccupied;
	}
}

void ASlashCharacter::LockToTarget()
{
	if (IsTargetEnemy())
	{
		// add a state so it can be used in transition rule from unlocked to locked locomotion
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

	if (Enemy)
	{
		Enemy->HideLockedEffect();
		Enemy = nullptr;
	}

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

void ASlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();

	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
	return (ActionState == EActionState::EAS_Unoccupied || 
			 ActionState == EActionState::EAS_Locked) &&
			 CharacterState != ECharacterState::ECS_Unequipped;
}

void ASlashCharacter::Die_Implementation()
{
	Super::Die_Implementation();

	ActionState = EActionState::EAS_Dead;
	DisableMeshAndCapsuleCollision();
}

bool ASlashCharacter::HasEnoughStamina()
{
	return Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost();
}

bool ASlashCharacter::CanDodge()
{
	return ActionState <= EActionState::EAS_Unoccupied;
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
		EnhancedInputComponent->BindAction(LeftAttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::LeftButtonAttack);
		EnhancedInputComponent->BindAction(OneKeyAttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::OneKeyAttack);
		EnhancedInputComponent->BindAction(TwoKeyAttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::TwoKeyAttack);
		EnhancedInputComponent->BindAction(ThreeKeyAttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::ThreeKeyAttack);
		EnhancedInputComponent->BindAction(LockOnTarget, ETriggerEvent::Started, this, &ASlashCharacter::LockTarget);
		EnhancedInputComponent->BindAction(DodgeIA, ETriggerEvent::Started, this, &ASlashCharacter::Dodge);
		EnhancedInputComponent->BindAction(SpeedUpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::SpeedUp);
		EnhancedInputComponent->BindAction(SpeedUpAction, ETriggerEvent::Completed, this, &ASlashCharacter::EndSpeedUp);
	}
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Attributes && SlashOverlay)
	{
		Attributes->RegenStamina(DeltaTime);
		SetHUDStamina();
	}
	if (bLocked && Enemy && !Enemy->IsDead())
	{
		FVector SlashLocation = GetActorLocation();
		FVector LockedTargetLocation = CombatTarget->GetActorLocation();

		Controller->SetControlRotation(UKismetMathLibrary::FindLookAtRotation(SlashLocation, LockedTargetLocation));
		
		//SpringArm->SetRelativeRotation(UKismetMathLibrary::FindLookAtRotation(SlashLocation, LockedTargetLocation));
		//SpringArm->SocketOffset = LockedTargetLocation;
		//ViewCamera->SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(ViewCamera->GetComponentLocation(), LockedTargetLocation));
		//ViewCamera->bUsePawnControlRotation = false;
	}
	if ((Enemy && Enemy->IsDead()) || IsOutOfRange())
	{
		UnlockFromTarget();
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	if (Attributes && SlashOverlay)
	{
		SetHUDHealth();
	}

	return DamageAmount;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

	// Only change the state if the character isn't dead
	if (Attributes && Attributes->GetHealthPercent() > 0.f)
	{
	/**
	* The player shouldn't be able to attack while playing the Hit Reaction, ie while in HitReaction state!
	* So when it call Attack() and check CanAttack, the states won't allow the character to attack :)
	*
	* We also need a way to change from HitReaction state. For that we'll use a BlueprintCallable function
	*  that'll be called from a AM.
	*/
		ActionState = EActionState::EAS_HitReaction;
	}
	
}

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void ASlashCharacter::AddSouls(ASoul* Soul) // Override from IPickupInterface; it'll be called from Soul class when it's picked up
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddSouls(Soul->GetSouls());
		SlashOverlay->SetSouls(Attributes->GetSouls());
	}
}

void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		SlashOverlay->SetGold(Attributes->GetGold());
	}
}

void ASlashCharacter::AddHealth(AHealth* Health)
{
	if (Attributes && SlashOverlay) // should check Health too?
	{
		// Add to attributes
		Attributes->AddHealth(Health->GetHealth());
		// Update HUD
		SetHUDHealth();
	}
}

void ASlashCharacter::AddBook(ABook* Book)
{
	if (SlashOverlay)
	{
		SlashOverlay->SetBooks(Book->GetBooks());
	}
}

bool ASlashCharacter::IsOutOfRange()
{
	if (CombatTarget)
	{
		const FVector SlashLocation = GetActorLocation();
		const FVector LockedTargetLocation = CombatTarget->GetActorLocation();

		double Distance = FVector::Dist(LockedTargetLocation, SlashLocation);

		return Distance > Range;
	}

	return false;
}
