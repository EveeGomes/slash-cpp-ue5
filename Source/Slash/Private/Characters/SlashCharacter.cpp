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

/** Naive approach to attach the weapon */
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"

/** For using Animation Montage in Attack() */
#include "Animation/AnimMontage.h"

/** To set the weapon's collision enabled */
#include "Components/BoxComponent.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	/** Movement */
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

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

	/** 
	* Use the Tags variable from the Character and Add method to add a tag which can get any name we want.
	*/
	Tags.Add(FName("SlashCharacter"));
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
		OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		/** 
		* Associate the actor who's equipping the weapon with the weapon itself.
		* SetOwner(this) can be used because actors have the concept of ownership and once we set it to this, 
		*  we'll be linking up this character with the weapon. Then, any actor can use the GetOwner and this will
		*  be returned as the designated owner.
		* Another way is to use SetInstigator(), but in this case it's more specific than SetOwner as it asks for
		*  a pawn as a param (pawn can be controlled by a person or AI). An actor can also be owned by any given pawn
		*  so we can pass this.
		* However, as we're doing all this associated with the fact of equipping the weapon, it'd be a better idea to
		*  pass in the ownner and instigator into the equip function for the weapon.
		*/


		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		OverlappingItem = nullptr;
		EquippedWeapon = OverlappingWeapon;
	}
	else
	{
		if (CanDisarm())
		{
			PlayEquipMontage(FName("Unequip"));
			CharacterState = ECharacterState::ECS_Unequipped;
			ActionState = EActionState::EAS_EquippingWeapon;
		}
		else if (CanArm())
		{
			PlayEquipMontage(FName("Equip"));
			CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
			ActionState = EActionState::EAS_EquippingWeapon;
		}
	}
}

void ASlashCharacter::Attack()
{
	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
	    CharacterState != ECharacterState::ECS_Unequipped;
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
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::Arm()
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

void ASlashCharacter::PlayAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);

		const int32 Selection = FMath::RandRange(0, 1);
		FName SectionName = FName();
		switch (Selection)
		{
		case 0:
			SectionName = FName("Attack1");
			break;
		case 1:
			SectionName = FName("Attack2");
			break;
		case 2:
			SectionName = FName("Attack3");
			break;
		default:
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
	}
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

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	}
}

void ASlashCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	// Check if the character has a weapon equipped to it
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox()) // Also check if WeaponBox isn't null
	{
		// Set the weapon's collision enabled
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		// Clear the TArray with actors to ignore!
		EquippedWeapon->IgnoreActors.Empty();
	}
}

