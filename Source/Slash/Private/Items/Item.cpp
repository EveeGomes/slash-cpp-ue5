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
	
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	// Local variable that receives the result value of using sin function with RunningTime
	// To change the curve amplitude: multiply the function by a value (in this case by 0.25f to make it shorter)
	// To change/manipulate how fast the up and down transition should happen (speed up the sin wave), multiply the value passed to the function by another value (in this case 5.f)
	float DeltaZ = 0.25f * FMath::Sin(RunningTime * 5.f);

	// Add the change to the actor's location
	AddActorWorldOffset(FVector(0.f, 0.f, DeltaZ));


	DRAW_SPHERE_SingleFrame(GetActorLocation());
	DRAW_VECTOR_SingleFrame(GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 100.f);

}

