// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BaseCharacter.h"
#include "Interfaces/PickupInterface.h"

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
class ASoul;
class ATreasure;
class UAnimMontage;
class AWeapon;
class UPawnSensingComponent;
class AEnemy;
class USlashOverlay;

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

private:
	void InitializeSlashOverlay(APlayerController* PlayerController);
	void SetHUDHealth();

	/** 
	* Create a Sphere box trace for objects function that uses the character location and the direction
	*  to which the camera is facing. So it'll detect enemies that are in front of this character!
	* This will be called in response to lock the enemy, ie bLocked = true
	*/
	void SphereTrace();

	bool CanJump();

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

	//UPROPERTY(VisibleAnywhere)
	//TObjectPtr<UPawnSensingComponent> PawnSensing;

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

	UPROPERTY()
	TObjectPtr<USlashOverlay> SlashOverlay;

protected:
	virtual void BeginPlay() override;

	/** Callbacks for input */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	virtual void Jump() override;
	void Dodge();

	void EKeyPressed();
	virtual void Attack() override;
	void LockTarget();
	void LockToTarget();

	bool IsTargetEnemy();

	bool CanLock();

	void UnlockFromTarget();

	/** Combat */
	void EquipWeapon(AWeapon* Weapon);
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual bool CanAttack() override;
	virtual void Die() override;

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

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> DodgeIA;

public:

	ASlashCharacter();
	/** <APawn> */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	/** </APawn> */

	/** <AActor> */
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser
	) override;

	/** </AActor> */

	/** <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/** </IHitInterface> */

	/** <IPickupInterface> */
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void AddSouls(ASoul* Soul) override;
	virtual void AddGold(ATreasure* Treasure) override;
	/** </IPickupInterface> */
	
	/** 
	* Getters and Setters
	*/
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }

	/** Used in LockTarget and in SlashAnimInstance */
	bool bLocked = false;
	bool bIsEnemy = false;

	/** Should Slash knows about enemy? */
	TObjectPtr<AEnemy> Enemy = nullptr;

	/** Range between Slash and target */
	UPROPERTY(EditAnywhere)
	float Range = 1000.f;

	bool IsOutOfRange();
};
