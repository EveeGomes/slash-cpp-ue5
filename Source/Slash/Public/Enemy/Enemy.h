// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/** Inheritance */
#include "Characters/BaseCharacter.h"

/** Use Action States */
#include "Characters/CharacterTypes.h"

#include "Enemy.generated.h"

/** 
* Class forward declaration 
*/
class UAnimMontage;
class UMyHealthBarComponent;
class UPawnSensingComponent;
class AWeapon;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
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

	/** Show health bar, play hit sound, spawn emmitter at location. If dead call Die() */
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	/** 
	* This function is already inherented from Actor class.
	* We don't really need to implement it in the BaseCharacter, unless there's something here we want to do
	*  in both classes.
	* Something that would not be unique (probably) is taking the Attributes component and call ReceiveDamage on it.
	* But since SlashCharacter might take damage in a slighly different way, we'll leave as is and override it
	*  separately in SlashCharacter!
	*/
	/** Public virtual function from AActor */
	virtual float TakeDamage(
		float DamageAmount, 
		struct FDamageEvent const& DamageEvent, 
		class AController* EventInstigator, 
		AActor* DamageCauser
	) override;

	/** Destroy the weapon as soon as the enemy instance is also destroyed */
	virtual void Destroyed() override;

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
	TObjectPtr<UAnimMontage> IdlePatrolMontage;

	/** 
	* Spawn a weapon
	*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> WeaponClass;

	/** 
	* Components
	*/
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
