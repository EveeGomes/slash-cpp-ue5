// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"

/** To set the weapon's collision enabled */
#include "Components/BoxComponent.h"
#include "Items/Weapons/Weapon.h"

/** Use our custom actor component */
#include "Components/AttributeComponent.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Construct Attributes component
	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));

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

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	/** 
	* Find Theta
	*/

	const FVector Forward = GetActorForwardVector();
	// Lower Impact Point to the enemy's actor location Z
	const FVector ImpactLowered{ ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z };
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	// Forward * ToHit = |Forward| * |ToHit| * cos(theta)
	// |Forward| = 1, |ToHit| = 1, so Forward * ToHit = cos(theta)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);

	// Take the inverse cosine (arc-cosine) os cos(theta) to get theta. Then convert from radians to degrees
	double Theta = FMath::Acos(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);

	// If CrossProduct points down, Theta should be negative.
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	/** 
	* Set Section name according to Theta, then call to play the montage
	*/
	FName Section{ "FromBack" };
	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName{ "FromFront" };
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName{ "FromLeft" };
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName{ "FromRight" };
	}

	PlayHitReactMontage(Section);
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


