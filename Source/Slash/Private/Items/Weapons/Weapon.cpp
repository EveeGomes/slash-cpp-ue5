// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"

AWeapon::AWeapon()
{
   // Create the WeaponBox
   WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
   WeaponBox->SetupAttachment(GetRootComponent());

   // Set the collision presets settings
   // If we wanted to ensure the collision enabled is set to query only (which is set by default for box components, 
   //  but we can set it anyways to make it clear that we definitely want that collision set):
   WeaponBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

   // Responses can also be set to various types
   WeaponBox->SetCollisionResponseToChannels(ECollisionResponse::ECR_Overlap);
   // Ignore only the Pawn
   WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
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
