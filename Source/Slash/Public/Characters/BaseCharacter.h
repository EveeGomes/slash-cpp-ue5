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

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

	/** 
	* Since this function can be the same for both SlashCharacter and Enemy, we can only have it here and don't need to 
	*  declare/implement in those classes.
	*/
	/** Called in response to an Anim notify. The ABP calls this function to enable/disable collision on our weapon */
	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

protected:
	// Variable for our currently equipped weapon
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<AWeapon> EquippedWeapon;

	/**
	* Animation Montages
	*/

	/** 
	* Array of section names that can have different amount of elements in each children.
	* So with that array we can choose at random a name for the section and send it to PlayMontageSection.
	* Then, setting it up in BP we can add or remove elements (section names) and the function will accommodate
	*  for that.
	*/
	UPROPERTY(EditAnywhere,  Category = "Combat")
	TArray<FName> AttackMontageSections;

	// TODO: HAVE TARRAYS FOR OTHER ANIMATION MONTAGES TO ADD/REMOVE THEIR SECTION NAMES IN BP!
	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> DeathMontageSections;

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> DeathMontage;

	/**
	* Components
	*/
	/** Add our custom Attribute Component */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeComponent> Attributes;

	virtual void BeginPlay() override;

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

	// Choose a section name from AttackMontageSections array
	// Change to return a int32 which comes from the return of PlayRandomMontageSection.
	virtual int32 PlayAttackMontage();
	/** 
	* The return value we'll use in the transition to a Death pose in ABP.
	* We'll override this function in the enemy class so it can use return value (the index).
	* This is definitely going to be a virtual function and as optional we can make PlayAttackMontage() virtual
	*  as well just in case we need it later.
	*/
	virtual int32 PlayDeathMontage();

	void PlayHitReactMontage(const FName& SectionName);
	/** 
	* Set the animation section name according to the hit direction angle and call PlayHitReactMontage()
	*/
	void DirectionalHitReact(const FVector& ImpactPoint);

	/** @param:		ImpactPoint coming from children classes? */
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);

	/** Attack */
	virtual void Attack();
	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();
	virtual bool CanAttack();
	bool IsAlive();

	/** Damage */
	virtual void HandleDamage(float DamageAmount);

	/** 
	* Plays Death Montage 
	*/
	virtual void Die();

private:
	/**
	* Variable to set a sound when a character gets hit.
	* Having this variable allows for setting different sounds to different characters.
	*/
	UPROPERTY(EditAnywhere, Category = "Sounds")
	TObjectPtr<USoundBase> HitSound;

	/**
	* Particle system
	* UParticleSystem is the type for the Cascade Particle System.
	* To spawn this particle, we need to use the GamePlayStatics System.
	*/
	UPROPERTY(EditAnywhere, Category = "VisualEffects")
	TObjectPtr<UParticleSystem> HitParticles;
};
