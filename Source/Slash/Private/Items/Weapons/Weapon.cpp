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

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
   ItemState = EItemState::EIS_Equipped;

   /** Set the owner and instigator here. That way the actor won't have to worry about it. */
   SetOwner(NewOwner);
   SetInstigator(NewInstigator);
   AttachMeshToSocket(InParent, InSocketName);
   DisableSphereCollision();

   PlayEquipSound();
   DeactivateEmbers();
}

void AWeapon::DeactivateEmbers()
{
   if (EmbersEffect)
   {
      EmbersEffect->Deactivate();
   }
}

void AWeapon::DisableSphereCollision()
{
   if (Sphere)
   {
      Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
   }
}

void AWeapon::PlayEquipSound()
{
   if (EquipSound)
   {
      UGameplayStatics::PlaySoundAtLocation(
         this,
         EquipSound,
         GetActorLocation()
      );
   }
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
   FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
   ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
}

void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
   /** 
   * Enemies ignore each other. This can also be more generic if we want not only Enemies to "know" each other,
   *  and in that case we use the same concept of Keys in print string node or debug message.
   */
   if (GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"))) return;

   FHitResult BoxHit;
   BoxTrace(BoxHit);

   if (BoxHit.GetActor())
   {
      /**
      * We need the damage to be applied before we play the montage, so that when it calls Execute_GetHit,
      *  and there it calls the montage to play, it'll play either the hit or death montage (by checking if the
      *  enemy still has health).
      */
      UGameplayStatics::ApplyDamage(
         BoxHit.GetActor(),
         Damage,
         GetInstigator()->GetController(),
         this,
         UDamageType::StaticClass()
      );

      ExecuteGetHit(BoxHit);
      CreateFields(BoxHit.ImpactPoint);
   }
}

void AWeapon::ExecuteGetHit(FHitResult& BoxHit)
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
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
   const FVector Start = TraceStart->GetComponentLocation();
   const FVector End = TraceEnd->GetComponentLocation();

   TArray<AActor*> ActorsToIgnore;
   ActorsToIgnore.Add(this);

   for (AActor* Actor : IgnoreActors)
   {
      ActorsToIgnore.AddUnique(Actor);
   }

   UKismetSystemLibrary::BoxTraceSingle(
      this,
      Start,
      End,
      BoxTraceExtent,
      TraceStart->GetComponentRotation(),
      ETraceTypeQuery::TraceTypeQuery1,
      false,
      ActorsToIgnore,
      bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
      BoxHit,
      true
   );

   IgnoreActors.AddUnique(BoxHit.GetActor());
}
