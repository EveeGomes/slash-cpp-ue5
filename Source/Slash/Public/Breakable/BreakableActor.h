// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/HitInterface.h"
#include "BreakableActor.generated.h"

/** For the geometry collection */
class UGeometryCollectionComponent;

UCLASS()
class SLASH_API ABreakableActor : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABreakableActor();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UGeometryCollectionComponent> GeometryCollection;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<class UCapsuleComponent> Capsule;
private:
	/** 
	* We gotta use TSubclassOf<ATreasure> to restrict which class this variable can be set in BP!
	* TSubclassOf works as a pointer that can wrap that pointer for us.
	* They're usually templates so that's why they start with T.
	* 
	* So, TSubclassOf<ATreasure> gives us a UClass variable that is wrapped in a TSubclassOf pointer
	*  that enforces the restriction that it can only be derived from the ATreasure C++ class or below.
	* ps: we can forward declare here too.
	* 
	* Use a TArray of this variable instead as we're now creating many BP_Treasures and we want to spawn them randomly
	*  when an object is broken.
	*/

	UPROPERTY(EditAnywhere, Category = "Breakable Properties")
	TArray <TSubclassOf<class ATreasure>> TreasureClasses;

	// Use this bool to toggle when we have broken our breakable objects
	bool bBroken = false;
};
