// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

/** Skeletal mesh component (AEnemy()) */
#include "Components/SkeletalMeshComponent.h"

/** Our custom actor component (HandleDamage()) */
#include "Components/AttributeComponent.h"

/** Our HealthBarComponent */
#include "HUD/MyHealthBarComponent.h"

/** Add AI movement */
#include "AIController.h"
#include "Perception/PawnSensingComponent.h"
// Setup movement orientation bool (AEnemy()) and check VSizeXY for Idel Patrol (CheckPatrolTarget())
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

/** Attach the weapon in BeginPlay() */
#include "Items/Weapons/Weapon.h"
#include "Items/Soul.h"

#include "NiagaraComponent.h"

#include "HUD/LockedTargetComponent.h"

#include "Slash/DebugMacros.h"


void AEnemy::InitializeEnemy()
{
	/** Move the enemy for the first time here (in BeginPlay) */
	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);
	HideHealthBar();
	SpawnDefaultWeapon();
	HideLockedEffect();
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	// Return false in case Target is invalid so in Tick we can remove some other validations
	if (Target == nullptr)
	{
		return false;
	}

	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();

	DRAW_SPHERE_SingleFrame(GetActorLocation());
	DRAW_SPHERE_SingleFrame(Target->GetActorLocation());

	return DistanceToTarget <= Radius;
}

AActor* AEnemy::ChoosePatrolTarget() 
{
	/** Have an array filled with all patrol targets except the one we currently have. */
	TArray<AActor*> ValidTargets;
	for (AActor* Target : PatrolTargets)
	{
		if (Target != PatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}

	// select one of the patrol target at random, and return our patrol target
	if (ValidTargets.Num() > 0)
	{
		const int32 TargetSelection = FMath::RandRange(0, ValidTargets.Num() - 1);
		return ValidTargets[TargetSelection];
	}

	return nullptr;
}

// Set to EAS_IdlePatrol
void AEnemy::CheckPatrolTarget() // EEnemyState::EES_Patrolling
{
	EnemyVelocity = UKismetMathLibrary::VSizeXY(GetCharacterMovement()->Velocity);

	if (InTargetRange(PatrolTarget, PatrolRadius)) // sometimes returns false
	{
		PatrolTarget = ChoosePatrolTarget();

		// Also check if it's not in IdlePatrol state already? This prevents bugs for spamming setting the same state
		if (EnemyState == EEnemyState::EES_Patrolling) 
		{
			EnemyState = EEnemyState::EES_IdlePatrol;
		}

		const float WaitTime = FMath::RandRange(PatrolWaitMin, PatrolWaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

/** When the timer has elapsed, call MoveToTarget */
void AEnemy::PatrolTimerFinished()
{	
	MoveToTarget(PatrolTarget);
}

void AEnemy::StartPatrolling()
{
	/** Return to Patrolling: state and velocity, and move to target (PatrolTarget) */
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(PatrolTarget);
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	EnemyController->MoveTo(MoveRequest);
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("WeaponSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::CheckCombatTarget()
{
	/** 
	* IMPORTANT:
	* Although ClearAttackTimer() is called in every if statement below, it shouldn't be placed outside them.
	* That's because CheckCombatTarget() is called every frame and we don't want ClearAttackTimer() to be called
	*  that frequent since it would cause spamming.
	* These if statements don't handle all cases, they all might failed in some frames therefore ClearAttackTimer()
	*  won't be called.
	*/

	if (IsOutsideCombatRadius())
	{
		/** 
		* Although the enemy loses interest, we might start the attack timer and it might still be going.
		* If that's true and the enemy is here (Outside the combat radius), as the SlashCharacter moves further away
		*  as soon as that timer is up, the enemy will attack.
		* Therefore, we have to clear that attack timer before the enemy starts patrolling.
		* 
		* Another thing to check before star patrolling is if the enemy is Engaged to combat, because that means
		*  the enemy is swinging the sword currently. And if that's true, the enemy shouldn't start patrolling since
		*  that would made the enemy to slide!
		*/
		ClearAttackTimer();
		LoseInterest();
		if (!IsEngaged()) StartPatrolling();
	}
	else if (IsOutsideAttackRadius() && !IsChasing())
	{
		/** 
		* We shall clear the attack timer here as well to avoid calling the attack while chasing the target.
		* Then, we check if it's not engaged and only then, the enemy can chase the target.
		*/
		ClearAttackTimer();
		if (!IsEngaged()) ChaseTarget(); 
	}
	else if (CanAttack())
	{
		/** 
		* We'll also clear the attack timer here because there's a chance for the timer to still be running,
		*  then when that time is up, the enemy will attack prematurely.
		* He removed because in StartAttackTimer() the timer will be reset it, so there's no need to clear it before.
		* 
		* Now, IsInsideAttackRadius() && !IsAttacking() are the conditions the enemy must be to then be able
		*  to attack. Turns out there's a virtual function in the base class (BaseCharacter), CanAttack().
		*  We'll override it here in enemy class, placing these conditions and to be safe check if it's not
		*  in Dead state.
		*/
		StartAttackTimer();
	}
}

void AEnemy::ChaseTarget() 
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true);
	}
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	HideHealthBar();
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	/**
	* Create a local bool in order to refactor a code where we can join if statements,
	*  like in this case where we don't want to continue unless SeenPawn->ActorHasTag(FName("SlashCharacter")) 
	*  returns true and then EnemyState != EEnemyState::EES_Attacking returns true.
	* The bool name should describe what's gonna happen if both conditions are true. In the bottom line, 
	*  we want to chase the target should the conditions return true!
	* EnemyState < EEnemyState::EES_Attacking we'll be checking if it's less than Attacking or Engaged, 
	*  since we shouldn't be in either state.
	* Now we just check a single condition using bShouldChaseTarget.
	*/
	const bool bShouldChaseTarget =
		EnemyState != EEnemyState::EES_Dead &&
		EnemyState != EEnemyState::EES_Chasing &&
		EnemyState < EEnemyState::EES_Attacking &&
		SeenPawn->ActorHasTag(FName("EngageableTarget")) &&
		!SeenPawn->ActorHasTag(FName("Dead"));

	if (bShouldChaseTarget)
	{
		/** Set the combat target */
		CombatTarget = SeenPawn;
		ClearPatrolTimer();
		ChaseTarget();

		/** Stop the IdlePatrol animation montage */
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Stop(0.f, IdlePatrolMontage);
		}
	}
}

bool AEnemy::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsIdlePatrolling()
{
	return EnemyVelocity == 0.f && EnemyState == EEnemyState::EES_IdlePatrol;
}

bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

void AEnemy::ShowLockedEffect()
{
	if (LockedEffectWidget) LockedEffectWidget->SetVisibility(true);
}

void AEnemy::HideLockedEffect()
{
	if (LockedEffectWidget) LockedEffectWidget->SetVisibility(false);
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::PlayIdlePatrolMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && IdlePatrolMontage)
	{
		AnimInstance->Montage_Play(IdlePatrolMontage);
		AnimInstance->Montage_JumpToSection(SectionName, IdlePatrolMontage);
	}
}

FName& AEnemy::IdlePatrolSectionName()
{
	const int32 Selection = FMath::RandRange(0, 2);
	//FName SectionName = FName();
	switch (Selection)
	{
	case 0:
		IdleSectionName = FName("Patrol1");
		break;
	case 1:
		IdleSectionName = FName("Patrol2");
		break;
	case 2:
		IdleSectionName = FName("Patrol3");
	}

	return IdleSectionName;
}

void AEnemy::FinishIdlePatrol()
{
	if (EnemyState == EEnemyState::EES_IdlePatrol)
	{
		EnemyState = EEnemyState::EES_Patrolling;
	}
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	/** Bind the callback function to the delegate */
	if (PawnSensing) PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	InitializeEnemy();

	Tags.Add(FName("Enemy"));
}

void AEnemy::Die_Implementation()
{
	Super::Die_Implementation();

	EnemyState = EEnemyState::EES_Dead;
	ClearAttackTimer();
	HideHealthBar();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatTarget = nullptr;

	/**
	* To spawn an actor, we use a UWorld function.
	* Then, use the returned value from SpawnActor() (a soul object), to set its souls value equal to the souls in the enemy's
	*  Attribute component after we've spawned it.
	*/
	SpawnSoul();
}

void AEnemy::Attack()
{
	Super::Attack();
	if (CombatTarget == nullptr) return;

	/** Set the state to Engaged as it plays the attack montage */
	EnemyState = EEnemyState::EES_Engaged;
	PlayAttackMontage();
}

bool AEnemy::CanAttack()
{
	/** we should also check if the enemy's not already Engaged, to avoid attacking again and spam the attacks */
	bool bCanAttack =
		IsInsideAttackRadius() &&
		!IsAttacking() &&
		!IsEngaged() &&
		!IsDead();
	return bCanAttack;
}

void AEnemy::AttackEnd()
{
	/**
	* To implement this we gotta think of the following:
	* We want the enemy to get out of the Engaged state, but which state should we be in?
	* 1. if the character has exited the attack radius, then the enemy should be ready to chase the player, so it should enter
	*  the Chasing state;
	* 2. if the player is outside the patrol radius, then the enemy should go back to Patrolling;
	* 3. and if the player is inside the patrol radius, then we should start the attack timer again.
	* IE what we're doing is similar to CheckCombatTarget().
	* Therefore, we could just call CheckCombatTarget() if the enemy's not in Engaged state anymore.
	*  But what state should the enemy be in?
	*
	* We should add another state, called NoState and once we call CheckCombatTarget() the enemy won't be Engaged or Attacking
	*  and the enemy will be able to attack again.
	* So it'll be in NoState just for a small amount of time as right after we call CheckCombatTarget() and things will be
	*  set/done based on distance.
	*
	* This function will be called linked to a anim notify from AM_Attack.
	*/
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
	if (Attributes && HealthBarWidget)
	{
		HealthBarWidget->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void AEnemy::SpawnSoul()
{
	UWorld* World = GetWorld();
	if (World && SoulClass && Attributes)
	{
		const FVector SpawnLocation = GetActorLocation() + FVector{ 0.f, 0.f, 125.f };
		ASoul* SpawnedSoul = World->SpawnActor<ASoul>(SoulClass, SpawnLocation, GetActorRotation());
		if (SpawnedSoul)
		{
			SpawnedSoul->SetSouls(Attributes->GetSouls());
			/** Set the owner to later ignore it during the line trace avoiding collision with it */
			SpawnedSoul->SetOwner(this);
		}
	}
}

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup mesh component collision
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	// Construct the health bar widget
	HealthBarWidget = CreateDefaultSubobject<UMyHealthBarComponent>(TEXT("HealthBar"));
	// As it has a location in space, we can attach to the root component
	HealthBarWidget->SetupAttachment(GetRootComponent());

	// Makes enemy face to the direction it's moving
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Construct Pawn Sensing component (give a native text name of PawnSensing)
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 4000.f;
	PawnSensing->SetPeripheralVisionAngle(45.f);

	LockedEffectWidget = CreateDefaultSubobject<ULockedTargetComponent>(TEXT("LockedEffect"));
	LockedEffectWidget->SetupAttachment(GetRootComponent());
	LockedEffectWidget->SetWorldLocation(FVector{ GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 15});
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/** 
	* We need to make sure to not going into the checks if the enemy is in the Dead state.
	*/
	if (IsDead()) return;

	// Idle Patrol
	if (IsIdlePatrolling()) // only Paladin has IdlePatrolling...
	{
		if (IdlePatrolMontage != nullptr)
		{
			PlayIdlePatrolMontage(IdlePatrolSectionName());
		}
		else
		{
			FinishIdlePatrol();
		}
	}

	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
	}
	else
	{
		CheckPatrolTarget(); // Set to EAS_IdlePatrol
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();

	if (IsInsideAttackRadius())
	{
		EnemyState = EEnemyState::EES_Attacking;
	}
	else if (IsOutsideAttackRadius())
	{
		ChaseTarget();
	}

	return DamageAmount;
}

void AEnemy::Destroyed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	if (!IsDead()) ShowHealthBar();
	ClearPatrolTimer();
	ClearAttackTimer();
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

	StopAttackMontage();
	/** 
	* The Raptor hit react animation doesn't have root motion like the paladin so it won't move and go outside the attack
	*  radius (which puts the paladin into Chasing mode and then it chases the player character to then attack).
	* The raptor will remain in the same place, not triggering the chasing mode to go after the player and attack again.
	* To fix that (for the Raptor and any other enemy) we gotta check if it's still in the attack radius and then start the
	*  attack timer again!
	* Also, we gotta make sure the enemy isn't dead! Otherwise it would raise up and start attacking again after the timer
	*  is finished lol.
	*/

	if (IsInsideAttackRadius())
	{
		if (!IsDead()) StartAttackTimer();
	}
}