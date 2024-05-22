// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

/** Forward declaration */
class USphereComponent;

UCLASS()
class SLASH_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameters")
	float Amplitude = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameters")
	float TimeConstant = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	float RotationRate = 100.f;

	UFUNCTION(BlueprintPure)
	float TransformedSin();

	// This function along with TransformedSin() can modify an actor's location in different axis giving a nice impression of periodic circled movement for example 
	UFUNCTION(BlueprintPure)
	float TransformedCos();

	// Create a function that can be bound to the OnComponentBeginOverlap delegate on a Primitive Component (from which USphereComponent derives)
	// For that we need to go to the PrimitiveComponent.h file, find that delegate and check the list of parameters to see which parameters we need in our callback function.
	// The function can only bind to the delegate if its signature matches with the delegate!!!
	// After the name of the delegate (OnComponentBeginOverlap), we can see what kinds of parameters our callback function needs to have.
	// With the proper list of parameters, we're capable of binding this function to the delegate (specifically a dynamic multicaste delegate)!!
	// As for being a dynamic multicaste delegate we need to expose the function to bind to the reflection system, since this type of delegate is capable of being exposed to blueprint	(and we can have event bound to that kind of delegate - that's why we have a OnComponentBeginOverlap event!)

	UFUNCTION() // Expose to the reflection system
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Variable used to add DeltaTime to it every frame
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float RunningTime;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* Sphere;
};
