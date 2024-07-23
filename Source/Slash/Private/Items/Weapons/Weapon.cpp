// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"

/** Use in OnBoxOverlap */
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/HitInterface.h"

AWeapon::AWeapon()
{
   // Create the WeaponBox
   WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
   WeaponBox->SetupAttachment(GetRootComponent());

   // Set collision
   WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
   WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
   WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

   // Construct Scene components
   TraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
   TraceStart->SetupAttachment(GetRootComponent());

   TraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
   TraceEnd->SetupAttachment(GetRootComponent());
}

void AWeapon::BeginPlay()
{
   Super::BeginPlay();

   // Bind the callback function to the delegate
   WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName)
{
   AttachMeshToSocket(InParent, InSocketName);
   ItemState = EItemState::EIS_Equipped;

   // Play sound when attaching the weapon
   if (EquipSound)
   {
      UGameplayStatics::PlaySoundAtLocation(
         this,
         EquipSound,
         GetActorLocation()
      );
   }
   if (Sphere)
   {
      Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
   }
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
   FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
   ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
   Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
   Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

/** 
* The goal now is to call GetHit once when hitting another actor.
* In this OnBoxOverLap, we call BoxTraceSingle, and as soon as we get a hit, we check if the actor implements the interface
*  so we can call GetHit.
* The issue is that we keep calling GetHit as Weapon continues to overlap with the same actor and triggering overlap event.
* To solve that, we should use the TArray and add the actor to ignore as soon as we hit it, then as we disable the collision
*  using the notify, we also remove the actor from the list so it can hit the same actor again after the first attack is finished.
*/
void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
   /** 
   * Location of the first scene component(start) - to get its world location we call GetComponentLocation() in C++ and 
   *  GetWorlLocation in BP.
   * That's a global space rotation in world space (not local space, which we'd get by calling GetRelativeLocation()).
   */
   const FVector Start = TraceStart->GetComponentLocation();
   const FVector End = TraceEnd->GetComponentLocation();
   
   TArray<AActor*> ActorsToIgnore;
   ActorsToIgnore.Add(this);

   /** 
   * Now that we have the actors the weapon has hit, we can loop through it, add to ActorsToIgnore in order to ignore them
   *  on the next call of BoxTraceSingle.
   * For that reason, we'll have to clear IgnoreActors later. That should be done when we're disabling collision on the box.
   *  We do that in response to an anim notify (therefore we can't get any overlap response after that disabling!).
   *  The overlap response will be enabled again once we attack again (also enabled by another anim notify)!
   * ps: the notifies are implemented in the event graph of ABP_Echo!
   */
   for (AActor* Actor : IgnoreActors)
   {
      ActorsToIgnore.AddUnique(Actor);
   }

   FHitResult BoxHit;
   /** 
   * Trace with a box trace. We do that by using the KismetSystemLibrary static function.
   * BoxTraceSingle is basically tracing against the visibility channel, so for ETRaceTypeQuery we can put "any value".
   *  We could set up custom trace type queries in the engine and pass in the number corresponding to our custom type, for example.
   * Create a TArray to pass as the ActorsToIgnore parameter.
   * FHitResult is passed as reference. In BP that's an output variable but in C++, as soon as BoxTRaceSingle is called, it'll
   *  populate with data a FHitResult we just create for that purpose.
   */
   UKismetSystemLibrary::BoxTraceSingle(
      this,
      Start,
      End,
      FVector(5.f, 5.f, 5.f),
      TraceStart->GetComponentRotation(), // we can use this orientation
      ETraceTypeQuery::TraceTypeQuery1,
      false,
      ActorsToIgnore,
      EDrawDebugTrace::ForDuration,
      BoxHit,
      true // similar to what we did by adding this to the TArray of ActorsToIgnore (redundent, but required parameter)
   );

   /** 
   * We need to call GetHit() and use the ImpactPoint that is placed in BoxHit.
   * 
   * BoxHit after BoxTraceSingle call, is filled with data that we're going to use.
   * We can get the actor that has been hit, by calling GetActor().
   * Then, we can check if that actor can be cast to the HitInterface, which means it inherits from that interface.
   */
   if (BoxHit.GetActor()) // check if it returns a valid value or if it's a null pointer
   {
      // Include HitInterface header
      IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor());
      if (HitInterface)
      {
         HitInterface->GetHit(BoxHit.ImpactPoint);
      }
      // As soon as we hit the actor, add it to the TArray (it'll be removed from this TArray by the end of the attack animation)
      IgnoreActors.AddUnique(BoxHit.GetActor());
   }

}
