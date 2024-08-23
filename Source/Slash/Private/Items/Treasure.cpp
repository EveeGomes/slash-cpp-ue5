// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Treasure.h"

/** For casting AActor pointer in OnSphereOverlap */
#include "Characters/SlashCharacter.h"


void ATreasure::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
   /** 
   * We don't need to call Super because we don't want what's implemented in the parent class.
   * The SetOverlappingItem() serves for weapon. We actually want to pick up the treasure as soon as the 
   *  character overlaps with it.
   * Therefore we'll cast OtherActor to SlashCharacter!
   */

   ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
   if (SlashCharacter)
   {
      SpawnPickupSound();
      Destroy();
   }
}
