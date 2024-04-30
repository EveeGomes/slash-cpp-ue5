// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"
#include "Slash/DebugMacros.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	UWorld* World = GetWorld();

	// Set location before using Location since it'll get the current actor location and use on the macros to draw the debug shapes
	SetActorLocation(FVector(0.f, 0.f, 50.f));
	SetActorRotation(FRotator(0.f, 45.f, 0.f));
	FVector Location = GetActorLocation();
	FVector ForwardVec = GetActorForwardVector();
	FRotator Rotator = GetActorRotation();

	DRAW_SPHERE(Location, FColor::Black);
	DRAW_VECTOR(Location, Location + ForwardVec * 100.f);
	DRAW_CONE(Location, Rotator);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

