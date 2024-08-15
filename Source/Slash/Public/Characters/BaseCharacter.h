// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/** 
* This class is going to be used in SlashCharacter and Enemy classes as they inherent it from.
* Therefore, this class is going to be designed to only be inhereted from.
*/

#include "CoreMinimal.h"

/** Inheritance that also goes to children classes */
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"

#include "BaseCharacter.generated.h"

/** Forward declaration */
class AWeapon;
class UAnimMontage;
class UAttributeComponent;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

private:
	/**
	* Play Montage Functions
	*/
	// Generic function to play any montage!
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	/**
	* Play a random montage section. It also returns the selected index that represents the section name that will
	*  be played.
	* This will be called from functions that will pass the corresponding animation montage to play along with
	*  their section names.
	*/
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UParticleSystem> HitParticles;

	/** Section names arrays */
	/**
	* Array of section names that can have different amount of elements in each children.
	* So with that array we can choose at random a name for the section and send it to PlayMontageSection.
	* Then, setting it up in BP we can add or remove elements (section names) and the function will accommodate
	*  for that.
	*/
	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> AttackMontageSections;

	// TODO: HAVE TARRAYS FOR OTHER ANIMATION MONTAGES TO ADD/REMOVE THEIR SECTION NAMES IN BP!
	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> DeathMontageSections;


	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> DeathMontage;

protected:
	/** <AActor> */
	virtual void BeginPlay() override;
	/** </AActor> */

	/** <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/** </IHitInterface> */

	/** Combat */
	virtual void Attack();
	virtual bool CanAttack();
	bool IsAlive();
	virtual void Die();
	void DirectionalHitReact(const FVector& ImpactPoint);
	virtual void HandleDamage(float DamageAmount);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	void DisableCapsule();

	/** Montage */
	void PlayHitReactMontage(const FName& SectionName);
	// Choose a section name from AttackMontageSections array
	virtual int32 PlayAttackMontage();
	/** 
	* The return value we'll use in the transition to a Death pose in ABP.
	* We'll override this function in the enemy class so it can use return value (the index).
	* This is definitely going to be a virtual function and as optional we can make PlayAttackMontage() virtual
	*  as well just in case we need it later.
	*/
	virtual int32 PlayDeathMontage();
	void StopAttackMontage();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	/** Called in response to an Anim notify. The ABP calls this function to enable/disable collision on our weapon */
	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
	
	// Variable for our currently equipped weapon
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<AWeapon> EquippedWeapon;

	/** Our custom Attribute Component */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeComponent> Attributes;

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

};
