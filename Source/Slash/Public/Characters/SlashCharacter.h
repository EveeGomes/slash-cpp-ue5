// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BaseCharacter.h"

#include "InputActionValue.h"
#include "CharacterTypes.h"

#include "SlashCharacter.generated.h"

/** Forward declaration */
class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UGroomComponent;
class AItem;
class UAnimMontage;
class AWeapon;

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter
{
	GENERATED_BODY()

private:
	/** 
	* Components
	*/
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> ViewCamera;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGroomComponent> Hair;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGroomComponent> Eyebrows;

	/** 
	* Items
	*/
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<AItem> OverlappingItem;

	/** 
	* Animation Montages 
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> EquipMontage;

	/**
	* States
	*/
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

protected:
	virtual void BeginPlay() override;

	/** Callbacks for input */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	virtual void Jump() override;
	void EKeyPressed();
	virtual void Attack() override;

	/** Combat */
	void EquipWeapon(AWeapon* Weapon);
	virtual void AttackEnd() override;
	virtual bool CanAttack() override;

	/** Equip / Unequip */
	void PlayEquipMontage(FName SectionName);
	bool CanDisarm();
	bool CanArm();
	void Disarm();
	void Arm();

	// Attach the weapon to the spine socket
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();

	// Attach the weapon to the right hand socket
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	/** Fix Jump animation after doing IK */
	//UPROPERTY(BlueprintReadOnly)
	//bool bCanJump = true;

	//UFUNCTION(BlueprintCallable)
	//void SetCanJump(bool bCan) { bCanJump = bCan; }

	/** Input Mapping Context */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> SlashContext;

	/** Input Actions */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MovementAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> EquipAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

public:
	ASlashCharacter();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** 
	* Getters and Setters
	*/
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
};
