// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"

/** Use in OnBoxOverlap */
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/HitInterface.h"

/** Deactivate the niagara system upon equip */
#include "NiagaraComponent.h"

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
   if (EmbersEffect)
   {
      EmbersEffect->Deactivate();
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

void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
   const FVector Start = TraceStart->GetComponentLocation();
   const FVector End = TraceEnd->GetComponentLocation();
   
   TArray<AActor*> ActorsToIgnore;
   ActorsToIgnore.Add(this);

   for (AActor* Actor : IgnoreActors)
   {
      ActorsToIgnore.AddUnique(Actor);
   }

   FHitResult BoxHit;

   UKismetSystemLibrary::BoxTraceSingle(
      this,
      Start,
      End,
      FVector(5.f, 5.f, 5.f),
      TraceStart->GetComponentRotation(),
      ETraceTypeQuery::TraceTypeQuery1,
      false,
      ActorsToIgnore,
      EDrawDebugTrace::None,
      BoxHit,
      true
   );

   /** 
   * Call GetHit() using the ImpactPoint that is placed in BoxHit.
   */
   if (BoxHit.GetActor())
   {

      IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor());
      if (HitInterface)
      {
         /** 
         * Now, we can't call this function HitInterface->GetHit(BoxHit.ImpactPoint); directly anymore. We have to allow
         *  UE reflection system to handle this interface a little different for us.
         * As we made this function a BlueprintNativeEvent in the Interface class, we now have different ways of calling it,
         *  and we'll choose the Execute_GetHit()
         * @UObject* the object to execute this event on
         * @const FVector& the GetHit original parameter
         */
         HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint);
      }
      // As soon as we hit the actor, add it to the TArray (it'll be removed from this TArray by the end of the attack animation)
      IgnoreActors.AddUnique(BoxHit.GetActor());

      CreateFields(BoxHit.ImpactPoint);

      /** 
      * This is where we'd like to cause damage to the actor that's been hit by the weapon.
      * So, the actor that's received the damage, can override the TakeDamage function, which exist in the Actor class.
      * As we want the Enemy to take the damage, we'll override that function in the Enemy class!
      * For the EventInstigator which is a Controller pointer, there's a way to access that because as soon as we
      *  equip this weapon, we should probably set the instigator for this actor. And the thing about actors is that we
      *  can designate an instigator to be associated with that actor.
      * Therefore, as soon as we equip this actor, we should also set some information on it so that we can access 
      *  who is causing the damage here. (We go to the function that calls Equip in SlashCharacter which is EKeyPressed())
      * 
      */

      UGameplayStatics::ApplyDamage(
         BoxHit.GetActor(),
         Damage,

      )
   }

   
}
