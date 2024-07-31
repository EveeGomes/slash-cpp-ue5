// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MyHealthBarComponent.h"

/** To cast to a HealthBar */
#include "HUD/HealthBar.h"

void UMyHealthBarComponent::SetHealthPercent(float Percent)
{
   // Cast to a HealthBar type which is a child of UUserWidget!
   UHealthBar* HealthBar = Cast<UHealthBar>(GetUserWidgetObject());
   if (HealthBar) // valid pointer if the cast succeeded
   {

   }
}
