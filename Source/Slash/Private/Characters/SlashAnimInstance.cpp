// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashAnimInstance.h"
#include "Characters/SlashCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void USlashAnimInstance::NativeInitializeAnimation()
{
   Super::NativeInitializeAnimation();

   SlashCharacter = Cast<ASlashCharacter>(TryGetPawnOwner());
   if (SlashCharacter)
   {
      SlashCharacterMovement = SlashCharacter->GetCharacterMovement();
   }
}

void USlashAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
   Super::NativeUpdateAnimation(DeltaTime);

   if (SlashCharacterMovement)
   {
      GroundSpeed = UKismetMathLibrary::VSizeXY(SlashCharacterMovement->Velocity);

      const FVector SlashVelocity = SlashCharacter->GetVelocity();
      const FRotator SlashRotation = SlashCharacter->GetActorRotation();
      Direction = CalculateDirection(SlashVelocity, SlashRotation);

      bIsLocked = SlashCharacter->bLocked;

      IsFalling = SlashCharacterMovement->IsFalling();
      CharacterState = SlashCharacter->GetCharacterState();
      ActionState = SlashCharacter->GetActionState();
   }
}
