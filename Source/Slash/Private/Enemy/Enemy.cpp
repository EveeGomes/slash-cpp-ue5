// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

/** In order to use the skeletal mesh and capsule components */
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

/** To draw debug shapes in GetHit*/
#include "Slash/DebugMacros.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/** 
	* Setup the properties of the mesh component(which is inherented from the ACharacter class).
	* The collision object type of this mesh needs to be set to WorldDynamic because the Weapon class is set to ignore Pawn type.
	* Also, set the response to the visibility channel to be blocking it.
	* Set the mesh to generate overlap events, otherwise it won't do anything.
	* 
	* Ignore the camera channel so it won't zoom in when colliding with the enemy mesh.
	* Make the capsule ignore the camera as well (as it can block the camera too).
	*/

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetGenerateOverlapEvents(true);

	// Ignore the camera
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
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

void AEnemy::GetHit(const FVector& ImpactPoint)
{
	DRAW_SPHERE_COLOR(ImpactPoint, FColor::Orange);

	PlayHitReactMontage(FName("FromLeft"));

	/** 
	* We need the forward and ToHit vectors!
	* GetActorForwardVector() returns a normalized vector.
	* But ToHit won't be a normalized vector, since it's calculated from two points in space and those could be
	*  any distance away from each other.
	* const FVector ToHit = ImpactPoint - GetActorLocation();
	* 
	* As to use Dot Product, both vectors should be normalized. For that we call GetSafeNormal().
	*/

	const FVector Forward = GetActorForwardVector();
	const FVector ToHit = (ImpactPoint - GetActorLocation()).GetSafeNormal();
	
	// Forward * ToHit = |Forward| * |ToHit| * cos(theta)
	// |Forward| = 1, |ToHit| = 1, so Forward * ToHit = cos(theta)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	
	// Take the inverse cosine (arc-cosine) os cos(theta) to get theta. 
	double Theta = FMath::Acos(CosTheta);
	// convert from radians to degrees
	Theta = FMath::RadiansToDegrees(Theta);

	// Debugging:

	// Add a on-screen debug message
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Green, FString::Printf(TEXT("Theta: %f"), Theta));
	}

	// Draw the vectors to have more visualization of what's going on. Use another static library for it, which has to be included above.
	// Multiply Forward by 60.f because it's a normalized vector, so it's 1 and that wouldn't show.
	// This is the arrow from the enemy's location straight out
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + Forward * 60.f, 5.f, FColor::Red, 5.f);
	// Arrow from the enemy's location to the hit location
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + ToHit * 60.f, 5.f, FColor::Green, 5.f);
}

