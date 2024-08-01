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
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	/**
	* Set the health bar percentage
	* In order to access this: HealthBarWidget->SetHealthPercent();
	*  we have to change HealthBarWidget type in the header file to the same type as where SetHealthPercent is define:
	*   UMyHealthBarComponent.
	*/
	if (HealthBarWidget) // check if the widget component is valid
	{
		HealthBarWidget->SetHealthPercent(1.f);
	}
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

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	DirectionalHitReact(ImpactPoint);

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
	* 
	*/

	return 0.0f;
}

