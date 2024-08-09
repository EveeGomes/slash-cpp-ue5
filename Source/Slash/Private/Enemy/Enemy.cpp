// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

/** In order to use the skeletal mesh and capsule components */
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

/** To draw debug shapes in GetHit*/
#include "Slash/DebugMacros.h"
#include "Kismet/KismetSystemLibrary.h"

/** Play sound, Spawn Cascade Particles emitter */
#include "Kismet/GameplayStatics.h"

/** Use our custom actor component */
#include "Components/AttributeComponent.h"

/** To use the HealthBarComponent */
#include "HUD/MyHealthBarComponent.h"

/** Setup movement orientation bool */
#include "GameFramework/CharacterMovementComponent.h"

/** Add AI movement */
#include "AIController.h"
#include "Perception/PawnSensingComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

/** Attach the weapon in BeginPlay() */
#include "Items/Weapons/Weapon.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup mesh component collision
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetGenerateOverlapEvents(true);

	// Ignore the camera
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

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

	// Starts as Patrolling by default
	if (EnemyVelocity == 0.f && EnemyState == EEnemyState::EES_IdlePatrol)
	{
		FName SectionName = IdlePatrolSectionName();
		PlayIdlePatrolMontage(SectionName);
	}

	if (EnemyState > EEnemyState::EES_Patrolling) // if the state is "more serious" than Patrolling
	{
		CheckCombatTarget();
	}

	CheckPatrolTarget(); // Set to EAS_IdlePatrol
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

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

		const float WaitTime = FMath::RandRange(WaitMin, WaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

void AEnemy::CheckCombatTarget()
{
	if (IsOutsideCombatRadius())
	{
		LoseInterest();
		StartPatrolling();
	}
	else if (IsOutsideAttackRadius() && !IsChasing())
	{
		ChaseTarget();
	}
	else if (IsInsideAttackRadius() && !IsAttacking())
	{
		StartAttackTimer();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	// Show health bar widget
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true);
	}

	if (Attributes && Attributes->IsAlive())
	{
		DirectionalHitReact(ImpactPoint);
	}
	else
	{
		// Play death montage using a function that handles the enemy death montage
		Die();
	}

	// Play sound as soon as the enemy gets hit
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			ImpactPoint
		);
	}

	/** 
	* Spawn an Emitter at location, using our HitParticles
	*/
	if (HitParticles && GetWorld())
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitParticles,
			ImpactPoint
			/** 
			* UnrealEditor_Slash_patch_3!AEnemy::~AEnemy() [C:\Users\evepg\Documents\Udemy\UE5-Cpp-Game-Developer\Slash\Intermediate\Build\Win64\UnrealEditor\Inc\Slash\UHT\Enemy.gen.cpp:366]
			* UnrealEditor_Slash_patch_3!AEnemy::`vector deleting destructor'()
			* Unhandled Exception: EXCEPTION_ACCESS_VIOLATION reading address 0x0000000400000070
			*/

			/** 
			* ImpactPoint comes from BoxHit variable of type FHitResult which we get data in Weapon class after
			*  calling BoxTraceSingle. Then we call Execute_GetHit and pass that variable.
			*/
		);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Attributes && HealthBarWidget)
	{
		Attributes->ReceiveDamage(DamageAmount);
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}
	// Get the pawn that's being controlled by this controller (EventInstigator) and set as the CombatTarget
	CombatTarget = EventInstigator->GetPawn();
	
	// Make the enemy chase and attack (from CheckCombatTarget()) the instigator
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	MoveToTarget(CombatTarget);

	return DamageAmount;
}

void AEnemy::Destroyed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	// Hide the health bar at the beginning of the game
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}

	/** Move the enemy for the first time here (in BeginPlay) */
	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);

	/** 
	* Bind the callback function to the delegate
	*/
	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}

	/**
	* Spawn an actor.
	* We use UWorld because SpawnActor exists in that class.
	* We'll do similar to what we did in BreakableActor class in GetHit_Implementation.
	* There's no need to pass a location or rotation because we'll attach it.
	*
	* Then, we'll check the EKeyPressed() and Equip() functions from SlashCharacter.
	* SpawnActor returns a AWeapon so we create a local AWeapon pointer and set to it.
	* After that we use OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this); changing
	*  OverlappingWeapon to DefaultWeapon.
	* So after we spwan the DefaultWeapon, we can set our EquippedWeapon
	*/
	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

void AEnemy::Die()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);

		/** Choose the animation to play randomly like we did in SlashCharacter.cpp */
		const int32 Selection = FMath::RandRange(0, 5);
		FName SectionName = FName();
		switch (Selection)
		{
		case 0:
			SectionName = FName("Death1");
			DeathPose = EDeathPose::EDP_Death1;
			break;
		case 1:
			SectionName = FName("Death2");
			DeathPose = EDeathPose::EDP_Death2;
			break;
		case 2:
			SectionName = FName("Death3");
			DeathPose = EDeathPose::EDP_Death3;
			break;		
		case 3:
			SectionName = FName("Death4");
			DeathPose = EDeathPose::EDP_Death4;
			break;
		case 4:
			SectionName = FName("Death5");
			DeathPose = EDeathPose::EDP_Death5;
			break;
		case 5:
			SectionName = FName("Death6");
			DeathPose = EDeathPose::EDP_Death6;
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName, DeathMontage);
	}

	// Hide the widget once the enemy dies
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}

	// Disable the capsule component collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Destroy the enemy after 3s of dying
	SetLifeSpan(3.f);
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

void AEnemy::PlayAttackMontage()
{
	Super::PlayAttackMontage();

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);

		const int32 Selection = FMath::RandRange(0, 2);
		FName SectionName = FName();
		switch (Selection)
		{
		case 0:
			SectionName = FName("Attack1");
			break;
		case 1:
			SectionName = FName("Attack2");
			break;
		case 2:
			SectionName = FName("Attack3");
			break;
		default:
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
	}
}

void AEnemy::Attack()
{
	Super::Attack();

	PlayAttackMontage();
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

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	// Return false in case Target is invalid so in Tick we can remove some other validations
	if (Target == nullptr) return false;

	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();

	return DistanceToTarget <= Radius;
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return; // this removes the need for the next if statement

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(50.f);
	EnemyController->MoveTo(MoveRequest);
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
		SeenPawn->ActorHasTag(FName("SlashCharacter"));
	
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

/** When the timer has elapsed, call MoveToTarget */
void AEnemy::PatrolTimerFinished()
{	
	MoveToTarget(PatrolTarget);
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
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

void AEnemy::StartPatrolling()
{
	/** Return to Patrolling: state and velocity, and move to target (PatrolTarget) */
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(PatrolTarget);
}

bool AEnemy::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

void AEnemy::ChaseTarget() 
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

bool AEnemy::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}
