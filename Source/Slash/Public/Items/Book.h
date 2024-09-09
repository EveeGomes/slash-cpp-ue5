// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Book.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API ABook : public AItem
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Book Properties")
	int32 Books;

protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

public:
	/** Getters and Setters */
	FORCEINLINE int32 GetBooks() const { return Books; }
	FORCEINLINE void SetBooks(int32 NumberOfBooks) { Books = NumberOfBooks; }
};
