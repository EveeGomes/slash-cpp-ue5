// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "InputActionValue.h"

#include "SlashCharacter.generated.h"

/** Forward declaration */
class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UGroomComponent;
class AItem;

// scoped enum (usage of class keyword!); another convention in UE to avoid umbiguity is to use an abbreviation of the enum name (normally using the capital letters) and prefix the constant names with it.
// using uint8 increases the optimization for this enum because it makes the integers assigned to the constants an unsigned integer type of 8 bits.
// Use ENUM() UE macro to expose the enum to the reflexion system. To allow the usage of this type in BP, we use the specifier BLueprintType
// Use another macro, UMETA to display a more friendly name of each constant in BP

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unequipped UMETA(DisplayName = "Unequipped"),
	ECS_EquippedOneHandedWeapon UMETA(DisplayName = "Equipped One-Handed Weapon"),
	ECS_EquippedTwoHandedWeapon UMETA(DisplayName = "Equipped Two-Handed Weapon")
};

UCLASS()
class SLASH_API ASlashCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASlashCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	/** Input Mapping Context */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* SlashContext;

	/** Input Actions */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* EquipAction;

	/** Callbacks for input */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	virtual void Jump() override;
	void EKeyPressed();

	/** Fix Jump animation after doing IK */
	//UPROPERTY(BlueprintReadOnly)
	//bool bCanJump = true;

	//UFUNCTION(BlueprintCallable)
	//void SetCanJump(bool bCan) { bCanJump = bCan; }

private:
	// Set the default character state as unequipped
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;
	
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere)
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere)
	UGroomComponent* Eyebrows;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;
	
	// Public section for getters and setters
public:
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
};
