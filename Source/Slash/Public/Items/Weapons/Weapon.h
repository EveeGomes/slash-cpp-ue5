// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

class USoundBase;

/**
 * 
 */
UCLASS()
class SLASH_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	// Attaches the Item mesh to the character's Skeletal mesh and set the Item state to equipped. Called in character class once the E key is pressed.
	void Equip(USceneComponent* InParent, FName InSocketName);
	void AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName);
	
protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	) override;
private:
	// Add a variable for the sound (which will be unique for each type of weapon - sword, hammer etc).
	// Use one of the classes from which Weapon is derived. He chose USoundBase because Sound Cues also derive from it, and if we decide to use Sound Cue instead of Meta Sounds we won't have any problem using this variable
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	USoundBase* EquipSound; // It'll be set in BP
};
