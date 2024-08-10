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
	virtual void PlayAttackMontage() override;

	/** Attack */
	virtual void Attack() override;
	virtual bool CanAttack() override;

	FName& IdlePatrolSectionName();
	// have a FName member variable to return a FName& instead of a copy?
	FName IdleSectionName = FName();
	
	/** Called once IdlePatrolEnd section name is reached in Anim Montage */
	UFUNCTION(BlueprintCallable)
	void FinishIdlePatrol();

	/** 
	* States
	*/
	/** Start with the alive pose
	* We'll now use EnemyState instead, as we added a new state: Dead.
	* So, with EnemyState exposed to BP, in ABP_Paladin we'll create a new variable of type EEnemyState and set it in
	*  Blueprint Thread Safe Update Animation.
	* Therefore, to play Dead pose we'll check, in the transition rule, if the enemy is in Dead state instead of 
	*  NOT in Alive state.
	* Remove the default value of Alive and only set it when the Enemy dies.
	* We can also remove the enum constant Alive from the EDeathPose enum.
	*/
	UPROPERTY(BlueprintReadOnly) // Only access what the variable is. No need to expose to the details panel either
	EDeathPose DeathPose;

	UPROPERTY(BlueprintReadOnly) // This specifier only works for non-private variables!
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

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
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
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

	/** 
	* AI Behavior
	*/
	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	void ClearPatrolTimer();

	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();
	bool IsIdlePatrolling();

	/** Checking enemy states */
	bool IsChasing();
	bool IsAttacking();
	bool IsDead();
	bool IsEngaged();

	/** Combat */
	UPROPERTY(EditAnywhere, Category = "Combat")
	float PatrollingSpeed = 125.f;
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ChasingSpeed = 300.f;

	// Handling function used to start the timer, set state to Attacking, and call Attack()
	void StartAttackTimer();
	void ClearAttackTimer();

	virtual void HandleDamage(float DamageAmount) override;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMin = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMax = 1.f;
};
