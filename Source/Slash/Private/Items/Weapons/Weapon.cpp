// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName)
{
   AttachMeshToSocket(InParent, InSocketName);
   ItemState = EItemState::EIS_Equipped;

   // To play sounds, we need gameplay statics.
   // If we search on the documentation for "PlaySoundAtLocation" we can find the gameplay statics
   // Once #include "Kismet/GameplayStatics.h" is added, check if EquipSound pointer was set in BP so we can use
   if (EquipSound)
   {
      UGameplayStatics::PlaySoundAtLocation(
         this,
         EquipSound,
         GetActorLocation()
      );
   }

   /** 
   * We have to disable the weapon's sphere collision settings because it keeps overlapping with the character's sphere collision and that makes the weapon to be set over and over again even though in EKeyPressed() (SlashCharacter class) we set it to nullptr.
   * That belongs to Item.h. We move to protected section so it can be accessed here.
   */

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
