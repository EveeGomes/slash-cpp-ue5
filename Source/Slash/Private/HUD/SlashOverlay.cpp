// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SlashOverlay.h"
#include "Components/ProgressBar.h"

void USlashOverlay::SetHealthPercent(float Percent)
{
   if (HealthProgressBar)
   {
      HealthProgressBar->SetPercent(Percent);
   }
}

void USlashOverlay::SetStaminaPercent(float Percent)
{
   if (StaminaProgressBar)
   {
      StaminaProgressBar->SetPercent(Percent);
   }
}
