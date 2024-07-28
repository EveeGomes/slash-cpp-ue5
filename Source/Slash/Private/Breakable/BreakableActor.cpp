// Fill out your copyright notice in the Description page of Project Settings.


#include "Breakable/BreakableActor.h"

#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Components/CapsuleComponent.h"

/** Used in GetHit_Implementation */
#include "Items/Treasure.h"

// Sets default values
ABreakableActor::ABreakableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it. <-
	PrimaryActorTick.bCanEverTick = false;

	// Construct the UGeometryCollectionComponent
	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	// As it derives from USceneComponent, we can make it the root component
	SetRootComponent(GeometryCollection);
	// Set Generate Overlap Events here as it becomes the default setting
	GeometryCollection->SetGenerateOverlapEvents(true);
	// In class 147 Q&A, suggestion to make the BP_Breakable work again
	GeometryCollection->bUseSizeSpecificDamageThreshold = true;
	// Ignore the camera channel to avoid glitching when a piece flies toward the camera
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	// Ignore the pawn channel as part of solving the disabling collision for the pot pieces when they're broken
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	// Construct the capsule
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	// Attach to the root component
	Capsule->SetupAttachment(GetRootComponent());
	// Set its response to all other channels to ignore
	Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	// Set the capsule's collision response to the pawn channel to block (so the capsule blocks the pawn!)
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

}

// Called when the game starts or when spawned
void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); 

}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint)
{
	/** 
	* this will avoid the infinite loop by preventing the GetHit function from being called many times due to our
	*  many objects being thrown around from our Geometry Collection which triggered the infinite loop safeguard.
	*/
	if (bBroken) return;
	bBroken = true;

	/** 
	* We can't use GetWorld()->SpawnActor<ATreasure>() because we want to spawn an actor
	*  based on a BP class instead of a C++ class. The BP class has things set up already like the mesh
	*  and sound, while the C++ has only the functionality of the overlap event.
	* 
	* We can specify not only a C++ class but a BP class to the UClass* param. It's like the type we pass
	*  to the angle brackets in a template function/class!
	* If we want a regular C++ class, we can satisfy a UClass input parameter by taking any class that's derived from
	*  the UClass class, and calling it's inhereted Static function like so: ATreasure::StaticClass(). That will return
	*  a UClass pointer that represents a type of that class. So, we can think that as a type rather than a pointer to
	*  an object.
	* StaticClass() gives us a raw C++ class in the form of a UClass pointer.
	* Now for specifying a BP class we could use a UClass type variable.
	* That'll be a pointer that we set as UPROPERTY(EditAnywhere) and in the Details panel we can set to either a C++
	*  or BP class. That way we'll have a variable in C++ that carries all data of a BP class/obj (the variable represents
	*  a BP we've created in the editor)!
	* So, in the header file we declare a UClass Pointer:
	* UPROPERTY(EditAnywhere)
	* TObjectPtr<UClass> TreasureClass;
	*  and then in BP we set it as the class we want to spawn (in this case BP_Treasure)
	* 
	* Then, we can do the spawn logic here:
	*/
	
	UWorld* World = GetWorld();
	if (World)
	{
		// For the location param we use the location of the breakable actor and rise it up by 75 units
		FVector Location = GetActorLocation();
		Location.Z += 75.f;
		
		World->SpawnActor<ATreasure>(TreasureClass, Location, GetActorRotation());
	}
}

