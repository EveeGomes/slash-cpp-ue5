// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Book.h"
#include "Interfaces/PickupInterface.h"

void ABook::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
   IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
   if (PickupInterface)
   {
      PickupInterface->AddBook(this); // virtual function that's implemented in the children of PickupInterface the way they need

      // Sounds and effects?

      Destroy();
   }
}
