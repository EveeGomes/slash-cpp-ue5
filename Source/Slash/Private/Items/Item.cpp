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

	// (cm/s), while DeltaTime is (s/frame)
	float MovementRate = 50.f;

	// (cm/s) * (s/frame) = cm/frame -> when multiplying two products and in one unit we have s dividing while in the other unit s is multiplying. So, in that situation we cancel out the seconds!
	AddActorWorldOffset(FVector(MovementRate * DeltaTime, 0.f, 0.f));
	DRAW_SPHERE_SingleFrame(GetActorLocation());

}

