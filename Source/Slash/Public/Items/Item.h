// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Variable used to add DeltaTime to it every frame
	float RunningTime;
	float Amplitude = 0.25f;
	float TimeConstant = 5.f; // In trigonometry, a period is the time required for the sin wave to go all the way up, down and go back to the initial location.
									  //	It's defined as period = 2*pi/K where K is a constant (which we use to multiply RunningTime)

};
