// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Soul.generated.h"

/**
 * 
 */

UCLASS()
class SLASH_API ASoul : public AItem
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Soul Properties")
	int32 Souls;

	UPROPERTY(EditAnywhere, Category = "Soul Properties")
	double DesiredZ = 0.;

	UPROPERTY(EditAnywhere, Category = "Soul Properties")
	double DriftRate = -20.;

protected:
	virtual void BeginPlay() override;

	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

public:
	virtual void Tick(float DeltaTime) override;
	/** Getters and Setters */
	FORCEINLINE int32 GetSouls() const { return Souls; }
	FORCEINLINE void SetSouls(int32 NumberOfSouls) { Souls = NumberOfSouls; }
};
