// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/** 
* This class is going to be used in SlashCharacter and Enemy classes as they inherent it from.
* Therefore, this class is going to be designed to only be inhereted from.
*/

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

/** Forward declaration */
class AWeapon;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter
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
	virtual void BeginPlay() override;

	// Variable for our currently equipped weapon
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<AWeapon> EquippedWeapon;

	/**
	* Play Montage Functions
	*/
	/** Attack */
	virtual void PlayAttackMontage();
	/** 
	* Now in this case we'll have it as virtual function since each child will implement it as they need by overriding it.
	*/
	virtual void Attack();
	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();
	virtual bool CanAttack();

};
