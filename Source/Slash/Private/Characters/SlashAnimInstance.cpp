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
      IsFalling = SlashCharacterMovement->IsFalling();
      // Set the character's state here since this is updated every single frame and this variable will check the character state and be set accordingly
      CharacterState = SlashCharacter->GetCharacterState();
   }
}
