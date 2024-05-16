// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"

/** Enhanced Input System */
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

/** Camera and Spring Arm */
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	ViewCamera->SetupAttachment(SpringArm);
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
	
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	// Using the following implementation for a directional movement
	const FRotator Rotation = Controller->GetControlRotation(); // Once we get the Controller, we store it in a FRotator since the Controller only has a Rotator
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f); // Then, we get the Yaw rotation and set it in a FRotator because that's the only rotation we care about for it's the rotation which will change the direction which the character will be able to move (we can't move up or down)

	// Since we need to find the direction the controller is pointing to, in other words we need to get the direction out of an FRotator, we should use matrices and for that we use the built in type FRotatorMatrix:
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); // GetUnitAxis is a function from the FRotationMatrix type that returns a unit vector (which has a length value of 1 = unit; so it's a normalized vector). Since we've specify X, that corresponds to the X axis of the yaw rotation! The FVector returned by this function, represents the direction that the controller is looking in.
	AddMovementInput(ForwardDirection, MovementVector.Y); // Since we're using the direction where the controller is facing to, the character now moves to the same direction the controller points to
	
	// We've found before which way was forward and now we should find which way is right:
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); // Y is the right vector corresponding to the yaw rotation, while X is the forward vector
	AddMovementInput(RightDirection, MovementVector.X);
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
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
	}

}

