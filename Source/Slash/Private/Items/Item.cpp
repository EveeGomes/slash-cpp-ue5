// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"

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
	
	UE_LOG(LogTemp, Warning, TEXT("Begin play called"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Cyan, FString("Item OnScreen Message!"));
	}
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GEngine)
	{
		FString Name = GetName();
		FString Message = FString::Printf(TEXT("DeltaTime is : % f"), DeltaTime);
		GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Cyan, Message);
		GEngine->AddOnScreenDebugMessage(2, 60.f, FColor::Black, Name);

		FString FormattedMessage = FString::Printf(TEXT("Item name is: %s"), *Name);
		GEngine->AddOnScreenDebugMessage(3, 60.f, FColor::Emerald, FormattedMessage);
	}

}

