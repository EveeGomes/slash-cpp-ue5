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

	// Construct Attributes component
	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	// it doesn't need to be attached to anything as it doesn't have a location or mesh or anything.

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

void AEnemy::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
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

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	// Return false in case Target is invalid so in Tick we can remove some other validations
	if (Target == nullptr) return false;

	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	// Draw some debug spheres here as it's gonna be called every frame. These spheres are to show the locations
	//  of the enemy and its target location.
	DRAW_SPHERE_SingleFrame(GetActorLocation());
	DRAW_SPHERE_SingleFrame(Target->GetActorLocation());

	return DistanceToTarget <= Radius;
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return; // this removes the need for the next if statement

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(15.f);
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

void AEnemy::FinishIdlePatrol()
{
	if (EnemyState == EEnemyState::EES_IdlePatrol)
	{
		EnemyState = EEnemyState::EES_Patrolling;
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{



	/** 
	* We're transitioning to Attack state many times and that's why it's being called again and again.
	* As we change the state here to Chasing, in Tick it goes back to Attacking.So we're changing to Attacking
	*  every time this PawnSeen function is called and we need a way to prevent that from happening.
	* If we're in Attacking state, PawnSeen doesn't need to do anything, same thing if we're already in Chasing
	*  state.
	* So, if it's NOT in Attacking, we'll set the state to Chasing. Move to the target. Also, stop the
	*  IdlePatrol animation!
	* These have to be done after we set our combat target.
	*/

	if (EnemyState == EEnemyState::EES_Chasing) return;

	if (SeenPawn->ActorHasTag(FName("SlashCharacter")))
	{
		GetWorldTimerManager().ClearTimer(PatrolTimer);
		GetCharacterMovement()->MaxWalkSpeed = 300.f; // this value can also come from a variable		
		// Set the combat target
		CombatTarget = SeenPawn;

		if (EnemyState != EEnemyState::EES_Attacking)
		{
			EnemyState = EEnemyState::EES_Chasing;		
			MoveToTarget(CombatTarget);

			// It should also stop the IdlePatrol animation! x_x <<<<<<<<<
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Stop(0.f, IdlePatrolMontage);
			}

			UE_LOG(LogTemp, Warning, TEXT("Pawn Seen, Chase Player"));
		}
	}
}

/** When the timer has elapsed, call MoveToTarget */
void AEnemy::PatrolTimerFinished()
{	
	MoveToTarget(PatrolTarget);
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

void AEnemy::CheckCombatTarget()
{
	if (!InTargetRange(CombatTarget, CombatRadius))
	{
		// Outside CombatRadius, lose interest
		CombatTarget = nullptr;
		if (HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(false);
		}
		EnemyState = EEnemyState::EES_Patrolling;
		GetCharacterMovement()->MaxWalkSpeed = 125.f;
		MoveToTarget(PatrolTarget);

		// Place logs to see how often and when these functions are called
		UE_LOG(LogTemp, Warning, TEXT("Lose interest"));
	}
	// check if it's in the AttackRadius AND if it's not already in Chasing state to avoid setting it again
	else if (!InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Chasing) 
	{
		// Outside Attack range, chase character
		EnemyState = EEnemyState::EES_Chasing;
		// Move toward the target, chasing it
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
		MoveToTarget(CombatTarget);

		UE_LOG(LogTemp, Warning, TEXT("Chase player"));
	}
	// Check if we are in attack range and if we're already in Attacking state
	else if (InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Attacking)
	{
		// Inside Attack range, attack character
		EnemyState = EEnemyState::EES_Attacking;
		// TODO: Attack Montage 

		UE_LOG(LogTemp, Warning, TEXT("Attack"));
	}
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

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemy::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	// Lower Impact Point to the enemy's actor location Z
	const FVector ImpactLowered{ ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z };
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	// Forward * ToHit = |Forward| * |ToHit| * cos(theta)
	// |Forward| = 1, |ToHit| = 1, so Forward * ToHit = cos(theta)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);

	// Take the inverse cosine (arc-cosine) os cos(theta) to get theta. Then convert from radians to degrees
	double Theta = FMath::Acos(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);

	// If CrossProduct points down, Theta should be negative.
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	// Set Section name according to Theta
	FName Section{ "FromBack" };
	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName{ "FromFront" };
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName{ "FromLeft" };
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName{ "FromRight" };
	}

	// Then, call the function to play the montage accordingly
	PlayHitReactMontage(Section);
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
	/** 
	* This is where we have access to the actor that has hit the enemy, so this is a good place to implement
	*  the action of responding when getting attacked from the back.
	* So here along with the setting of CombatTarget to the pawn that's hit the enemy, we can also set the state
	*  to Chasing, increase the velocity to run, and than make it move to the target.
	* We have to go to CheckCombatTarget and see what will happen when we call MoveToTarget() here while being so
	*  close. We'll reach the third if check which passes the InTargetRange() and the state not being Attacking.
	*  Once we get in the state changes to Attacking where we'll play the Attack montage!
	*/

	if (Attributes && HealthBarWidget)
	{
		Attributes->ReceiveDamage(DamageAmount);
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}
	// Get the pawn that's being controlled by this controller (EventInstigator) and set as the CombatTarget
	CombatTarget = EventInstigator->GetPawn();
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	MoveToTarget(CombatTarget); // check what will 

	return DamageAmount;
}

