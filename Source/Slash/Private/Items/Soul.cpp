// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Soul.h"
#include "Interfaces/PickupInterface.h"
#include "Kismet/KismetSystemLibrary.h"

void ASoul::BeginPlay()
{
	Super::BeginPlay();

	const FVector Start = GetActorLocation();
	const FVector End = Start - FVector{ 0.f, 0.f, 2000.f };

	/** TEnumAsByte is just an enum in the form of an eight bit integer, an unsigned int. */
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner()); // so the owner of this soul is ignore by the trace

	FHitResult HitResult;

	UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		Start,
		End,
		ObjectTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResult,
		true
	);

	DesiredZ = HitResult.ImpactPoint.Z + 100.;
}

void ASoul::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		PickupInterface->AddSouls(this);

		SpawnPickupSystem();
		SpawnPickupSound();
		Destroy();
	}
}

void ASoul::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const double ZActorLocation = GetActorLocation().Z;

	if (ZActorLocation > DesiredZ)
	{
		AddActorWorldOffset(
			FVector{ 0., 0., DeltaTime * DriftRate }
		);
	}
}
