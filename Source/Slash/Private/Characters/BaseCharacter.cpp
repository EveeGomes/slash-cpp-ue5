// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"

/** To set the weapon's collision enabled */
#include "Components/BoxComponent.h"
#include "Items/Weapons/Weapon.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
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

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::PlayAttackMontage()
{
}

void ABaseCharacter::Attack()
{
}

void ABaseCharacter::AttackEnd()
{
}

bool ABaseCharacter::CanAttack()
{
	return false;
}

void ABaseCharacter::Die()
{
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


