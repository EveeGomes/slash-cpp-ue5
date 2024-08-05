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
	if (ActionState == EActionState::EAS_Unoccupied)
	{
		//GEngine->AddOnScreenDebugMessage(1, 2.f, FColor::Red, FString("Unoccupied Begin Play"));
		MoveToTarget(PatrolTarget);
	}
	//MoveToTarget(PatrolTarget);

	/** 
	* Access the world timer manager in order to set the timer.
	* We'll pass the PatrolTimer, this as the user object on which our callback exists, then the address
	*  of our callback function, and finally the time.
	* 
	* So, first we'll call MoveToTarget and then the timer will be set to 5 seconds. When that time
	*  has elapsed, the callback will be called.
	*/
	//GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, 5.f);
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

FName AEnemy::IdlePatrolSectionName()
{
	const int32 Selection = FMath::RandRange(0, 2);
	FName SectionName = FName();
	switch (Selection)
	{
	case 0:
		SectionName = FName("Patrol1");
		break;
	case 1:
		SectionName = FName("Patrol2");
		break;
	case 2:
		SectionName = FName("Patrol3");
	}

	return SectionName;
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

	GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, FString("InTargetRange"));


	// THIS CAN'T BE HERE AS CheckCombatTarget() CHECKS THIS AS WELL AS CheckPatrolTarget() SO IT KEEPS SETTING THE STATE!
	
	//ActionState = EActionState::EAS_IdlePatrol;
	//
	//if (!GetVelocity().IsZero() && ActionState == EActionState::EAS_IdlePatrol) // FVector{ 0.f }
	//{
	//	GEngine->AddOnScreenDebugMessage(3, 1.f, FColor::Orange, FString::Printf(TEXT("EAS_IdlePatrol and Is zero: %d"), GetVelocity().IsZero()));
	//	
	//	// After it moves to target, play the animation montage:
	//	FName SectionName = IdlePatrolSectionName();
	//	PlayIdlePatrolMontage(SectionName);
	//}

	return DistanceToTarget <= Radius;
}

// enemy velocity always returns zero in this function
void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return; // this removes the need for the next if statement

	//// Set the state to patrolling
	//ActionState = EActionState::EAS_Patrolling;

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(15.f);

	//m_AcceptanceRadius = MoveRequest.GetAcceptanceRadius(); //<< NO NEED? REMOVE FROM HEADER FILE
	EnemyController->MoveTo(MoveRequest);

	if (ActionState == EActionState::EAS_Patrolling)
	{
		GEngine->AddOnScreenDebugMessage(2, 2.f, FColor::Yellow, FString("MoveToTarget Patrolling"));
	}
}

// It detects GetVelocity().IsZero() before the enemy velocity reaches zero
AActor* AEnemy::ChoosePatrolTarget() 
{
	//GEngine->AddOnScreenDebugMessage(5, 2.f, FColor::Yellow, FString::Printf(TEXT("Is zero: %d"), GetVelocity().IsZero()));

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
	ActionState = EActionState::EAS_Patrolling;
	//GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Cyan, FString("Unoccupied"));
}

/** When the timer has elapsed, call MoveToTarget */
void AEnemy::PatrolTimerFinished()
{	
	//// Finished the time, change to EAS_Patrolling and move
	ActionState = EActionState::EAS_Patrolling;
	GEngine->AddOnScreenDebugMessage(3, 2.f, FColor::Cyan, FString("PatrolTimerFinished Patrolling"));
	
	MoveToTarget(PatrolTarget);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckCombatTarget(); // EAS_IdlePatrol
	CheckPatrolTarget(); // in PatrolTimerFinished, set EAS_Patrolling

	// didn't work here
}

void AEnemy::CheckCombatTarget()
{
	//// Set the state to patrolling
	//ActionState = EActionState::EAS_IdlePatrol;
	//GEngine->AddOnScreenDebugMessage(4, 1.f, FColor::Blue, FString("IdlePatrol CheckCombatTarget"));

	//GEngine->AddOnScreenDebugMessage(3, 1.f, FColor::Orange, FString::Printf(TEXT("CheckCombatTarget and Is zero: %d"), GetVelocity().IsZero())); // TRUE!!!!

	if (GetVelocity().IsZero())
	{
		GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Orange, FString::Printf(TEXT("EAS_IdlePatrol and Is Zero: %d"), GetVelocity().IsZero()));
		ActionState = EActionState::EAS_IdlePatrol;

		//FName SectionName = IdlePatrolSectionName();
		//PlayIdlePatrolMontage(SectionName);
	}

	if (!InTargetRange(CombatTarget, CombatRadius))
	{
		CombatTarget = nullptr;
		if (HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(false);
		}
	}
}

void AEnemy::CheckPatrolTarget()
{
	//ActionState = EActionState::EAS_Patrolling;
	/*GEngine->AddOnScreenDebugMessage(3, 1.f, FColor::Cyan, FString("Patrolling CheckPatrolTarget"));*/

	//GEngine->AddOnScreenDebugMessage(4, 2.f, FColor::Orange, FString::Printf(TEXT("CheckPatrolTarget. IdlePatrol Is Zero: %d"), GetVelocity().IsZero()));

	//if (GetVelocity().IsZero() && ActionState == EActionState::EAS_IdlePatrol) // FVector{ 0.f }
	//{
	//	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Orange, FString::Printf(TEXT("EAS_IdlePatrol and Is zero: %d"), GetVelocity().IsZero()));
	//	
	//	// After it moves to target, play the animation montage:
	//	FName SectionName = IdlePatrolSectionName();
	//	PlayIdlePatrolMontage(SectionName);
	//}

	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();

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
		);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	/** 
	* Once we call ApplyDamage using GamePlayStatics, the Enemy TakeDamage will be called.
	* We can do all the necessary actions here that we want to do, like updating the health and the health bar.
	* Use the component to call the Receive Damage function passing the DamageAmount. And thanks to GameplayStatics,
	*  it'll make sure this function gets called whenever we apply damage.
	* We'll then set the Health percent here instead of BeginPlay. We do inside the if statement because we'll need
	*  the information of how much health we'll have and that info is stored on Attributes.
	* We can combine both if statements, because in this case it's ok to not get inside the if statement if either of
	*  them is false.
	* @return we return DamageAmount because TakeDamage technically returns the amount of damage caused
	*/

	if (Attributes && HealthBarWidget)
	{
		Attributes->ReceiveDamage(DamageAmount);
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}
	// Get the pawn that's being controlled by this controller (EventInstigator)
	CombatTarget = EventInstigator->GetPawn();

	return DamageAmount;
}

