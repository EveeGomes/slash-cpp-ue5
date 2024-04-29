// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"
#include "DrawDebugHelpers.h"
#include "Slash/Slash.h"



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
	FVector Location = GetActorLocation();
	FVector ForwardVec = GetActorForwardVector();

	DRAW_SPHERE(Location);
	DRAW_LINE(Location, Location + ForwardVec * 100.f);
	DRAW_POINT(Location + ForwardVec * 100.f);

}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

