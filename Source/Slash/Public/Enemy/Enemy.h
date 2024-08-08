// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/** Include the headers of the classes this one is inhereting from */
#include "GameFramework/Character.h"
#include "Characters/BaseCharacter.h"

#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"

/** Use Action States */
#include "Characters/CharacterTypes.h"

#include "Enemy.generated.h"

/** 
* Class forward declaration 
*/
class UAnimMontage;
class UAttributeComponent;
class UMyHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	/** 
	* Enemy Patrolling, Chasing and Attacking behavior
	*/
	void CheckPatrolTarget();
	void CheckCombatTarget();

	/** Set the animation section name according to the hit direction and call PlayHitReactMontage() */
	void DirectionalHitReact(const FVector& ImpactPoint);
	/** Show health bar, play hit sound, spawn emmitter at location. If dead call Die() */
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	/** Public virtual function from AActor */
	virtual float TakeDamage(
		float DamageAmount, 
		struct FDamageEvent const& DamageEvent, 
		class AController* EventInstigator, 
		AActor* DamageCauser
	) override;

	/** Used in Tick() to check if IdlePatrol animation can be played */
	float EnemyVelocity = 0.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** 
	* Plays death montage 
	*/
	virtual void Die() override;

	/** 
	* Play Montage Functions
	*/
	void PlayIdlePatrolMontage(const FName& SectionName);

	FName& IdlePatrolSectionName();
	// have a FName member variable to return a FName& instead of a copy?
	FName IdleSectionName = FName();
	
	/** Called once IdlePatrolEnd section name is reached in Anim Montage */
	UFUNCTION(BlueprintCallable)
	void FinishIdlePatrol();

	/** 
	* States
	*/
	/** Start with the alive pose */
	UPROPERTY(BlueprintReadOnly) // Only access what the variable is. No need to expose to the details panel either
	EDeathPose DeathPose = EDeathPose::EDP_Alive;

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling; // MOVE TO PRIVATE AS IN THE COURSE?

	/** 
	* Enemy Patrolling, Chasing and Attacking behavior
	*/
	/** Returns true if we're in range of that Target, based on a specified radius */
	bool InTargetRange(AActor* Target, double Radius);

	/** Calls MoveTo */
	void MoveToTarget(AActor* Target);

	/** Picks a new patrol target at random */
	AActor* ChoosePatrolTarget();

	/** Callback function to use with OnSeePawn delegate. Bound in BeginPlay() */
	UFUNCTION() // to be bound to a delegate
	void PawnSeen(APawn* SeenPawn);

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

	/** 
	* Components
	*/
	/** Add our custom Attribute Component */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeComponent> Attributes;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UMyHealthBarComponent> HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPawnSensingComponent> PawnSensing;

	/** 
	* Patrolling and Attacking
	*/
	/** Pointer to store what has hit the enemy */
	UPROPERTY() // ensures the pointer is set to null
	TObjectPtr<AActor> CombatTarget;

	/** Threshold to check Distance To Target */
	UPROPERTY(EditAnywhere)
	double CombatRadius = 500.f;

	/** Threshold to start attacking the player */
	UPROPERTY(EditAnywhere)
	double AttackRadius = 150.f;

	/** Threshold to patrol */
	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;

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

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMin = 9.5f;
	float WaitMax = 10.5f;
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
};
