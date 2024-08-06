// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/** Include the headers of the classes this one is inhereting from */
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"

/** Use Action States */
#include "Characters/CharacterTypes.h"

#include "Enemy.generated.h"

/** For animation montage logic */
class UAnimMontage;

class UAttributeComponent;
class UMyHealthBarComponent;

UCLASS()
class SLASH_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void CheckPatrolTarget();

	void CheckCombatTarget();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void DirectionalHitReact(const FVector& ImpactPoint);

	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	/** Public virtual function from AActor */
	virtual float TakeDamage(
		float DamageAmount, 
		struct FDamageEvent const& DamageEvent, 
		class AController* EventInstigator, 
		AActor* DamageCauser
	) override;



	float EnemyVelocity = 0.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Play the death montage */
	void Die();

	/** 
	* Play Montage Functions 
	*/
	void PlayHitReactMontage(const FName& SectionName);
	void PlayIdlePatrolMontage(const FName& SectionName);
	FName IdlePatrolSectionName();
	// have a FName member variable to return a FName& instead of a copy?
	//FName IdleSectionName = FName();

	/** Start with the alive pose */
	UPROPERTY(BlueprintReadOnly) // Only access what the variable is. No need to expose to the details panel either
	EDeathPose DeathPose = EDeathPose::EDP_Alive;

	UPROPERTY(BlueprintReadOnly)
	EIdlePatrol IdlePatrolState = EIdlePatrol::EIP_Patrolling;

	/** Returns true if we're in range of that Target, based on a specified radius */
	bool InTargetRange(AActor* Target, double Radius);

	/** It calls MoveTo */
	void MoveToTarget(AActor* Target);

	/** Pick a new patrol target at random */
	AActor* ChoosePatrolTarget();

	UFUNCTION(BlueprintCallable)
	void FinishIdlePatrol();

private:
	/** 
	* Animation Montages 
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> IdlePatrolMontage;



	//bool IsPatrolling();

	/** 
	* Variable to set a sound when an enemy gets hit.
	* Having this variable allows for setting different sounds to different enemies.
	*/
	UPROPERTY(EditAnywhere, Category = "Sounds")
	TObjectPtr<USoundBase> HitSound;

	/** 
	* Particle system 
	* UParticleSystem is the type for the Cascade Particle System.
	* To spawn this particle, we need to use the GamePlayStatics System.
	*/
	UPROPERTY(EditAnywhere, Category = "VisualEffects")
	TObjectPtr<UParticleSystem> HitParticles;

	/** Add our custom Attribute Component */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeComponent> Attributes;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UMyHealthBarComponent> HealthBarWidget;

	/** Pointer to store what has hit the enemy */
	UPROPERTY() // ensures the pointer is set to null
	TObjectPtr<AActor> CombatTarget;

	/** Threshold to check the DistanceToTarget */
	UPROPERTY(EditAnywhere)
	double CombatRadius = 500.f;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;

	float m_AcceptanceRadius = 0.f;

	/** 
	* Navigation
	*/
	UPROPERTY()
	TObjectPtr<class AAIController> EnemyController;

	// Current patrol target
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TObjectPtr<AActor> PatrolTarget;

	// Once it reaches the PatrolTarget, it should change to a new PatrolTarget
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<TObjectPtr<AActor>> PatrolTargets;

	/**
	* TimerHandle is a struct that the world timer manager uses to keep track of various timers that we set.
	*/
	FTimerHandle PatrolTimer;
	/** 
	* In order to have a timer do something, we'll need a callback function.
	* When we set a timer, we specify the TimerHandle (PatrolTimer), which allows the world timer manager keep track
	*  of that timer. (Like an alarm clock with a function attached: PatrolTimerFinished()). 
	*/
	/** Moves the enemy to a target after a certain time */
	void PatrolTimerFinished();

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMin = 9.5f;
	float WaitMax = 10.5f;
};
