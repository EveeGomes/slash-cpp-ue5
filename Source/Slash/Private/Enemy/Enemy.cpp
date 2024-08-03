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

	/** 
	* Move the enemy for the first time here (in BeginPlay)
	*/
	EnemyController = Cast<AAIController>(GetController());
	if (EnemyController && PatrolTarget)
	{
		// Set an FAIMoveRequest before sending as a param to MoveTo
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(PatrolTarget);
		MoveRequest.SetAcceptanceRadius(15.f); // the enemy will stop 15 units short
		// Local variable to pass as arg to MoveTo
		FNavPathSharedPtr NavPath;

		EnemyController->MoveTo(MoveRequest, &NavPath);
		/**
		* FNavPathSharedPtr type has path points. FNavPathPoint is a struct derived from FNavLocation which has
		*  a FVector member variable, Location.
		* Having a FNavPathSharedPtr allows us to access data on that NavPath after it's passed as arg to MoveTo.
		* 
		* We can use a reference to avoid making copy of it.
		* Check: https://chatgpt.com/share/fb6b1b33-0e80-424a-8b72-455247c98782
		*/
		TArray<FNavPathPoint>& PathPoints = NavPath->GetPathPoints();
		// Draw some debug spheres to see those NavPathPoints
		for (auto& Point : PathPoints)
		{
			const FVector& Location = Point.Location;
			DrawDebugSphere(GetWorld(), Location, 12.f, 12, FColor::Green, false, 10.f);
		}

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

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/** Calculate the distance from the enemy to the actor that's hit it, ie: the CombatTarget */
	if (CombatTarget)
	{
		/**
		* CombatTarget location - Enemy location = the vector from the enemy to the combat target
		* Then we can get the distance by getting the length of this vector:
		* (CombatTarget location - Enemy location).Size() or .Length()
		* We'll then check this DistanceToTarget against a threshold that we'll declare as a double variable
		* If the distance is greater than the combat radius, it means the target lost interest to the enemy,
		*  so that CombatTarget can be set to null! ie we can set the CombatTarget at any circumstances we want.
		*  If the CombatTarget is also too far away, we'll also hide the enemy health bar
		*/
		const double DistanceToTarget = (CombatTarget->GetActorLocation() - GetActorLocation()).Size();
		if (!InTargetRange(CombatTarget, CombatRadius)); // InTargetRange returns false so we use ! to make it true and enter
		{
			CombatTarget = nullptr;
			if (HealthBarWidget)
			{
				HealthBarWidget->SetVisibility(false);
			}
		}
	}

	if (PatrolTarget && EnemyController)
	{
		if (InTargetRange(PatrolTarget, PatrolRadius))
		{
			// select one of the patrol target at random, and set it as our patrol target
			const int32 TargetSelection = FMath::RandRange(0, PatrolTargets.Num() - 1);
			PatrolTarget = PatrolTargets[TargetSelection];

			// then, move to that target:

			// Set an FAIMoveRequest before sending as a param to MoveTo
			FAIMoveRequest MoveRequest;
			MoveRequest.SetGoalActor(PatrolTarget);
			MoveRequest.SetAcceptanceRadius(15.f); // the enemy will stop 15 units short
			// NavPath is an optional input, so we can live it out. We were using it in BeginPlay to actually
			//  check the PathPoints using debug spheres.
			EnemyController->MoveTo(MoveRequest);
		}
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

