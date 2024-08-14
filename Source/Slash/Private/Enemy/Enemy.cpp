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

void AEnemy::InitializeEnemy()
{
	/** Move the enemy for the first time here (in BeginPlay) */
	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);
	HideHealthBar();
	SpawnDefaultWeapon();
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	// Return false in case Target is invalid so in Tick we can remove some other validations
	if (Target == nullptr) return false;

	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();

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
void AEnemy::CheckPatrolTarget()
{
	EnemyVelocity = UKismetMathLibrary::VSizeXY(GetCharacterMovement()->Velocity);

	if (InTargetRange(PatrolTarget, PatrolRadius))
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
	if (EnemyController == nullptr || Target == nullptr) return; // this removes the need for the next if statement

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(50.f);
	EnemyController->MoveTo(MoveRequest);
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
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
	*  like in this case where we don't want to continue unless SeenPawn->ActorHasTag(FName("SlashCharacter")) returns true
	*  and then EnemyState != EEnemyState::EES_Attacking returns true.
	* The bool name should describe what's gonna happen if both conditions are true. In the bottom line, we want to chase
	*  the target should the conditions return true!
	* EnemyState < EEnemyState::EES_Attacking we'll be checking if it's less than Attacking or Engaged, since we shouldn't
	*  be in either state.
	* Now we just check a single condition using bShouldChaseTarget.
	*/
	const bool bShouldChaseTarget =
		EnemyState != EEnemyState::EES_Dead &&
		EnemyState != EEnemyState::EES_Chasing &&
		EnemyState < EEnemyState::EES_Attacking &&
		SeenPawn->ActorHasTag(FName("EngageableTarget"));

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

void AEnemy::Die()
{
	// although it's not a scoped enum anymore we can still reference it with the enum name
	EnemyState = EEnemyState::EES_Dead;
	PlayDeathMontage();
	// avoid attacking again if the attack timer is over or running by clearing out the attack timer
	ClearAttackTimer();
	HideHealthBar();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
}

int32 AEnemy::PlayDeathMontage()
{
	/**
	* When calling Super::PlayDeathMontage(); its execution will return a int32, so we'll save it in a variable.
	* Then we set the DeathPose state based on that index returned and saved in Selection!
	* Once we set that, after playing a specific death montage section, we need to go to the DeathPose that corresponds
	*  to that index, how to do it:
	*  we need an Enum based on the int32 Selection, for that we're going to use a enum wrapper (UE enums): TEnumAsByte<>()
	*   where we specify the enum type we need: TEnumAsByte<enum type>. We pass the int32 Selection to initialize it as
	*   there's an overload of the constructor of TEnumAsByte that takes an int32! It sets the TEnumAsByte wrapper to the value
	*   of the enum type based on the int32! ie it'll set Pose to the value of EDeathPose based on Selection.
	*  We know that in the enum type the first constant is associated to an integer value of 0, unless we give them
	*   other values.
	*   And since we've specified in EDeathPose to be int8, those constants are technically unsigned eight bits integers,
	*   therefore they're BYTES.
	*
	* Then, we set the DeathPose to Pose, but before we shall check if the int32 Selection isn't greater than the values
	*  of enum constants.
	* In order to do that, to know how many enum constants there are in an enum, we should add a final enum constant
	*  (at the end)
	*  of enum list, that we designate as the maximum: EnumName_MAX + UMETA. Having that, we can check to make sure that Pose
	*  isn't greater than or equal to max: ie we check if Pose < EDP_MAX before using it.
	* In need to add the section names in BP to the array that we created for our death sections.
	*
	* Compiling as is, we'll get the following warning:
	* 1>C:\Program Files\Epic Games\UE_5.2\Engine\Source\Runtime\Core\Public\Containers\EnumAsByte.h(20): warning C4996: 'TEnumAsByte_EnumClass<true>': TEnumAsByte is not intended for use with enum classes - please derive your enum class from uint8 instead. Please update your code to the new API before upgrading to the next release, otherwise your project will no longer compile.
	* That's because when we declare the enum in CharacterTypes.h, we make it fully qualified by adding the keyword class
	*  and making it : uint8.
	* To resolve that warning, we should remove the class keyword, : uint8 and in Enemy.h we should declare the EDeathPose
	*  variable wrapped in TEnumAsByte<>.
	*
	*  ---->>>>>> for the constructor, use {} instead of () ??
	*/
	const int32 Selection = Super::PlayDeathMontage();
	TEnumAsByte<EDeathPose> Pose{ Selection };
	if (Pose < EDeathPose::EDP_MAX)
	{
		DeathPose = Pose;
	}

	return Selection;
}

void AEnemy::Attack()
{
	/** Set the state to Engaged as it plays the attack montage */
	EnemyState = EEnemyState::EES_Engaged;
	Super::Attack();

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
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
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
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/** 
	* We need to make sure to not going into the checks if the enemy is in the Dead state.
	*/
	if (IsDead()) return;

	// Idle Patrol
	if (IsIdlePatrolling())
	{
		PlayIdlePatrolMontage(IdlePatrolSectionName());
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
	ChaseTarget();

	return DamageAmount;
}

void AEnemy::Destroyed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	Super::GetHit_Implementation(ImpactPoint);
	ShowHealthBar();
}