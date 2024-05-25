// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
   Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

   // Cast OtherActor to a SlashCharacter type. This cast will succeed only if OtherActor is a SlashCharacter object! :O If it's of any other type, the cast returns null!
   ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
   // Always a good practice to check if the pointer is valid before using it!
   if (SlashCharacter)
   {
      // Attach the ItemMesh on the weapon to the skeletal mesh on the SlashCharacter:

      // Create an FAttachmentTransformRules to pass it to the AttachToComponent
      FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
      ItemMesh->AttachToComponent(SlashCharacter->GetMesh(), TransformRules, FName("RightHandSocket"));
   }
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
   Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}
