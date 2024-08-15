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

private:
	/** 
	* AI Behavior
	*/
	void InitializeEnemy();
	bool InTargetRange(AActor* Target, double Radius);
	AActor* ChoosePatrolTarget();
	void CheckPatrolTarget();
	void PatrolTimerFinished();
	void StartPatrolling();
	void MoveToTarget(AActor* Target);
	void SpawnDefaultWeapon();

	void ClearPatrolTimer();
	void CheckCombatTarget();
	void ChaseTarget();
	void ShowHealthBar();
	void LoseInterest();
	void HideHealthBar();

	// Callback function to use with OnSeePawn delegate (UPawnSensingComponent). Bound in BeginPlay()
	UFUNCTION() // to be bound to a delegate
	void PawnSeen(APawn* SeenPawn);

	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();
	bool IsIdlePatrolling();

	/** Checking enemy states */
	bool IsChasing();
	bool IsAttacking();
	bool IsDead();
	bool IsEngaged();

	/** Idle Patrol Animation */
	void PlayIdlePatrolMontage(const FName& SectionName);
	FName& IdlePatrolSectionName();
	// Called once IdlePatrolEnd section name is reached in Anim Montage
	UFUNCTION(BlueprintCallable)
	void FinishIdlePatrol();

	/** Combat */
	// Handling function used to start the timer, set state to Attacking, and call Attack()
	void StartAttackTimer();
	void ClearAttackTimer();

	UPROPERTY()
	TObjectPtr<class AAIController> EnemyController;

	/** Components */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UMyHealthBarComponent> HealthBarWidget;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPawnSensingComponent> PawnSensing;

	// Spawn a weapon
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> WeaponClass;

	// Threshold to check Distance To Target
	UPROPERTY(EditAnywhere)
	double CombatRadius = 1000.f;

	// Threshold to start attacking the player
	UPROPERTY(EditAnywhere)
	double AttackRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ChasingSpeed = 300.f;

	// Threshold to patrol
	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;

	// Current patrol target
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TObjectPtr<AActor> PatrolTarget;

	// Once it reaches the PatrolTarget, it should change to a new PatrolTarget
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<TObjectPtr<AActor>> PatrolTargets;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float PatrollingSpeed = 125.f;

	/** Idle Patrol */
	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> IdlePatrolMontage;
	// Used in Tick() to check if IdlePatrol animation can be played
	float EnemyVelocity = 0.0f;
	// have a FName member variable to return a FName& instead of a copy?
	FName IdleSectionName = FName();


	/** Timer Handle */
	/** Patrol */
	FTimerHandle PatrolTimer;
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMin = 9.5f;
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMax = 10.5f;

	/** Attack */
	FTimerHandle AttackTimer;
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMin = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMax = 1.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DeathLifeSpan = 4.f;

protected:
	/** <AActor> */
	virtual void BeginPlay() override;
	/** </AActor> */

	/** <ABaseCharacter> */
	virtual void Die() override;
	virtual int32 PlayDeathMontage() override;
	/** Attack */
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void AttackEnd() override;
	virtual void HandleDamage(float DamageAmount) override;
	/** </ABaseCharacter> */

	/** States */
	UPROPERTY(BlueprintReadOnly) // Only access what the variable is. No need to expose to the details panel either
	TEnumAsByte<EDeathPose> DeathPose;

	UPROPERTY(BlueprintReadOnly) // This specifier only works for non-private variables!
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

public:
	AEnemy();

	/** <AActor> */
	virtual void Tick(float DeltaTime) override;
	/**
	* This function is already inherented from Actor class.
	* We don't really need to implement it in the BaseCharacter, unless there's something here we want to do
	*  in both classes.
	* Something that would not be unique (probably) is taking the Attributes component and call ReceiveDamage on it.
	* But since SlashCharacter might take damage in a slighly different way, we'll leave as is and override it
	*  separately in SlashCharacter!
	*/
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser
	) override;
	/** Destroy the weapon as soon as the enemy instance is also destroyed */
	virtual void Destroyed() override;
	/** </AActor> */

	/** <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/** </IHitInterface> */
};
