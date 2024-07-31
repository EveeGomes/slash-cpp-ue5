// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MyHealthBarComponent.h"

/** To cast to a HealthBar */
#include "HUD/HealthBar.h"

/** Access SetPercent */
#include "Components/ProgressBar.h"

void UMyHealthBarComponent::SetHealthPercent(float Percent)
{
   /** 
   * Like it was before, as this function will be called many times, the cast would also happened many many many times!
   * And that's not good since the cast is super expensive because it keeps checking if it's possible to cast through
   *  all other classes in the hierarchy.
   * So, we have HealthBarWidget as a member variable and before casting, we check if it's nullptr, meaning it was
   *  not casted before.
   * To make sure that HealthBarWidget is initialized to a null pointer, we can use UPROPERTY().
   */

   if (HealthBarWidget == nullptr)
   {
      // Set HealthBarWidget by casting UUserWidget* to a HealthBar* type which is a child of UUserWidget!
      HealthBarWidget = Cast<UHealthBar>(GetUserWidgetObject());
   }

   // valid pointer if the cast succeeded (NOT NULL). As we access HealthBar pointer, check it too
   if (HealthBarWidget && HealthBarWidget->HealthBar) // check from left to right
   {
      /** 
      * Set the percentage of the progress bar.
      * For that we can accesss the progress bar of WBP_HealthBar through HealthBar class that has a HealthBar
      *  UProgressBar pointer bound to the prograss bar in BP!
      */
      HealthBarWidget->HealthBar->SetPercent(Percent);
   }
}
