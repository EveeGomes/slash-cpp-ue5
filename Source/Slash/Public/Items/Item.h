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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* ItemMesh;

	UFUNCTION(BlueprintPure)
	float TransformedSin();

	// This function along with TransformedSin() can modify an actor's location in different axis giving a nice impression of periodic circled movement for example 
	UFUNCTION(BlueprintPure)
	float TransformedCos();

	/** Callbacks to bind to multicast dynamic delegates */
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Variable used to add DeltaTime to it every frame
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float RunningTime;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* Sphere;
};
