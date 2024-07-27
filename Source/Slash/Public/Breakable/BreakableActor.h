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

	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGeometryCollectionComponent> GeometryCollection;

	/** 
	* We gotta use TSubclassOf<ATreasure> to restrict which class this variable can be set in BP!
	* TSubclassOf works as a pointer that can wrap that pointer for us.
	* They're usually templates so that's why they start with T.
	* 
	* So, TSubclassOf<ATreasure> gives us a UClass variable that is wrapped in a TSubclassOf pointer
	*  that enforces the restriction that it can only be derived from the ATreasure C++ class or below.
	* ps: we can forward declare here too.
	*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ATreasure> TreasureClass;
};
