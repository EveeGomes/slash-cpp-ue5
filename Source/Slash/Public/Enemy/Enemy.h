// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/** Include the headers of the classes this one is inhereting from */
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"

#include "Enemy.generated.h"

/** For animation montage logic */
class UAnimMontage;

UCLASS()
class SLASH_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void DirectionalHitReact(const FVector& ImpactPoint);

	virtual void GetHit(const FVector& ImpactPoint) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Play Montage Functions */
	void PlayHitReactMontage(const FName& SectionName);

private:
	/** Animation Montages */
	UPROPERTY(EditDefaultsOnly, Category = "Montage") // EditDefaultsOnly: can be set in the defaults BP
	TObjectPtr<UAnimMontage> HitReactMontage;

};
