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
	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> HitReactMontage;

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
	virtual void PlayAttackMontage();
	void PlayHitReactMontage(const FName& SectionName);
	/** 
	* Set the animation section name according to the hit direction angle and call PlayHitReactMontage()
	* 
	* As long as children classes have the same Section names for their own anim montage, we can use the implementation
	*  we already had in Enemy.cpp
	*/
	void DirectionalHitReact(const FVector& ImpactPoint);

	/** Attack */
	virtual void Attack();
	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();
	virtual bool CanAttack();

	/** 
	* Plays Death Montage 
	*/
	virtual void Die();

};
