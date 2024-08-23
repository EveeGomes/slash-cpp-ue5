// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Treasure.generated.h"

/**
 * All we need to do in this class is override the OnSphereOverlap function from Item class.
 */
UCLASS()
class SLASH_API ATreasure : public AItem
{
	GENERATED_BODY()

private:
	// Use to set a gold amount to a treasure
	UPROPERTY(EditAnywhere, Category = "Treasure Properties")
	int32 Gold;

protected:
	// Children classes don't need to have UFUNCTION() as it's already inherited!
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

public:
	FORCEINLINE int32 GetGold() const { return Gold; }
};
