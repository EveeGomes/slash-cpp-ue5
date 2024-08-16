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
class UPawnSensingComponent;

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter
{
	GENERATED_BODY()

private:

	// Callback function to use with OnSeePawn delegate (UPawnSensingComponent). Bound in BeginPlay()
	UFUNCTION() // to be bound to a delegate
	void PawnSeen(APawn* SeenPawn);

	/** 
	* 
	*/

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

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPawnSensingComponent> PawnSensing;

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
	void LockTarget();

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

	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

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

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LockOnTarget;

	bool bLocked = false;

public:
	ASlashCharacter();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	/** <AActor> */
	virtual void Tick(float DeltaTime) override;


	/** <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/** </IHitInterface> */

	/** 
	* Getters and Setters
	*/
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
};
