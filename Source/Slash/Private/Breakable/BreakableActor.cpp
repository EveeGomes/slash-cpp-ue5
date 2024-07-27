// Fill out your copyright notice in the Description page of Project Settings.


#include "Breakable/BreakableActor.h"

#include "GeometryCollection/GeometryCollectionComponent.h"

// Sets default values
ABreakableActor::ABreakableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it. <-
	PrimaryActorTick.bCanEverTick = false;

	// Construct the UGeometryCollectionComponent
	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	// As it derives from USceneComponent, we can make it the root component
	SetRootComponent(GeometryCollection);
	// Set Generate Overlap Events here as it becomes the default setting
	GeometryCollection->SetGenerateOverlapEvents(true);
	// In class 147 Q&A, suggestion to make the BP_Breakable work again
	GeometryCollection->bUseSizeSpecificDamageThreshold = true;
	// Ignore the camera channel to avoid glitching when a piece flies toward the camera
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

// Called when the game starts or when spawned
void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); 

}

void ABreakableActor::GetHit(const FVector& ImpactPoint)
{

}

