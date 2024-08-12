// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

class USoundBase;
class UBoxComponent;

/**
 * 
 */
UCLASS()
class SLASH_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();
	// Attaches the Item mesh to the character's Skeletal mesh and set the Item state to equipped.
	// Called in character class once the E key is pressed.
	void Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator);
	void DeactivateEmbers();
	void DisableSphereCollision();
	void PlayEquipSound();
	void AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName);

	// Get track of the actors hit
	TArray<AActor*> IgnoreActors;

	/** 
	* Getter and Setter
	*/
	FORCEINLINE UBoxComponent* GetWeaponBox() const { return WeaponBox; }
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBoxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	/** 
	* Create some Transient Field.
	* @param to know where that field should be
	* 
	* It'll be called from C++ but handled in BP (its definition)
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);

private:
	void BoxTrace(FHitResult& BoxHit);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	FVector BoxTraceExtent = FVector{ 5.f };

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bShowBoxDebug = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TObjectPtr<USoundBase> EquipSound;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	TObjectPtr<UBoxComponent> WeaponBox;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> TraceStart;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> TraceEnd;

	/** 
	* Variable to store the damage a weapon might cause. This will be passed to as a param ApplyDamage() 
	*  in OnBoxOverlap().
	* This way we can set different damage values for different kinds of weapons as we create BP versions
	*  that inherit from this class.
	* We can give a default value.
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Damage = 20.f;
};
