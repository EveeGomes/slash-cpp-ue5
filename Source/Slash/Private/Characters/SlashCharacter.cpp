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

//void ASlashCharacter::PawnSeen(APawn* SeenPawn)
//{
//	// Check SeenPawn is valid too?
//	if (SeenPawn->ActorHasTag(FName("Enemy")))
//	{
//		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, TEXT("Pawn is valid"));
//
//		// TODO: Set the combat target 
//		CombatTarget = SeenPawn;
//
//		//DRAW_SPHERE(CombatTarget->GetActorLocation());
//	}
//}

void ASlashCharacter::SphereTrace()
{
	const FVector SlashLocation = GetActorLocation();
	FVector CameraFwd = ViewCamera->GetForwardVector(); // a bit different from YT
	FVector End = (CameraFwd * 500.f) + SlashLocation;

	// Objects to trace against
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic)); // or pawn?

	TArray<AActor*> ActorsToIgnore;
	TArray<FHitResult> ActorsHit;

	UKismetSystemLibrary::SphereTraceMultiForObjects(
		this,
		SlashLocation,
		End,
		125.f,
		ObjectTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		ActorsHit,
		true
	);
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}

	///** Bind the callback function to the delegate */
	//if (PawnSensing) PawnSensing->OnSeePawn.AddDynamic(this, &ASlashCharacter::PawnSeen);

	/** 
	* Use the Tags variable from the Character and Add method to add a tag which can get any name we want.
	*/
	Tags.Add(FName("EngageableTarget"));
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;

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
	if (ActionState == EActionState::EAS_Attacking) return;
	Super::Jump();

	//if (bCanJump)
	//{
	//	Super::Jump();
	//	bCanJump = false;
	//}
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
	/*GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Emerald, TEXT("LockTagert() called"));*/ // OK!!!!
	// Recriate the Flip Flop node by having a boolean that toggles T and F every time this function is called.
	if (!bLocked)
	{
		// Engange lock

		// do whatever is needed in this function
		bLocked = true;
		GetCharacterMovement()->bOrientRotationToMovement = false; // change values to a boolean variable?
		GetCharacterMovement()->bUseControllerDesiredRotation = true;

		SphereTrace();

		if (CombatTarget && CombatTarget->ActorHasTag(FName("Enemy")))
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Combat target is enemy")); // OK!!!!
		}

	}
	else
	{
		// Disangaged lock

		// when it's locked, and TAB is pressed again, set to false to unlock after removing combat target and
		//  the particle and whatever is more necessary
		bLocked = false;
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Emerald, FString::Printf(TEXT("2nd bLocked: %d"), bLocked));
		CombatTarget = nullptr; // ok (need to always check before using it!)

		GetCharacterMovement()->bOrientRotationToMovement = true; // change values to a boolean variable?
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}

	/** 
	* TODO:
	* [] Only allow this function to be called if the pawn is in sight's radius!
	* [] Lock camera rotation to the target's movement instead of the mouse
	* [] Use a widget or niagara system to show it's targeted?
	* [] Check slash states?
	*/
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
	return ActionState == EActionState::EAS_Unoccupied &&
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
	return ActionState == EActionState::EAS_Unoccupied && 
		 CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		 CharacterState == ECharacterState::ECS_Unequipped &&
		 EquippedWeapon; // check if it's not a null pointer (meaning we had gotten a weapon already)
}

void ASlashCharacter::Disarm()
{
	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
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

	//// Construct Pawn Sensing component (give a native text name of PawnSensing)
	//PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	//PawnSensing->SightRadius = 2000.f;
	//PawnSensing->SetPeripheralVisionAngle(45.f);
	//PawnSensing->bOnlySensePlayers = false; // IMPORTANT TO ALLOW NON-PLAYERS PAWNS TO BE SEEN!!!!!
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

	// apparently it's not checking if CombatTarget is valid, because when the enemy dies, it remains locked
	if (bLocked && CombatTarget) 
	{
		FVector SlashLocation = GetActorLocation();
		FVector LockedTargetLocation = CombatTarget->GetActorLocation();

		Controller->SetControlRotation(UKismetMathLibrary::FindLookAtRotation(SlashLocation, LockedTargetLocation));
	}
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