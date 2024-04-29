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
	// It's also possible to add ; after those macros. Without it VS might suggest indentation as if the code isn't finished yet (but again it's optional!)

	if (World)
	{
		DrawDebugPoint(World, Location + ForwardVec * 100.f, 15.f, FColor::Black, true);
	}


}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

