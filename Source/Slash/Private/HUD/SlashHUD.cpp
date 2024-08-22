// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"

void ASlashHUD::BeginPlay()
{
   Super::BeginPlay();

   /** 
   * Create widget
   * In CreateWidget, @OwningObject is our player controller which we can get using a World function:
   *  GetFirstPlayerController() which in a single player game will grab us that only controller that
   *  exist in the game.
   * After creating the widget we'd like to ADD it to the viewport. For that we have to store the
   *  returned value by CreateWidget() to finally call AddToViewport().
   */
   
   UWorld* World = GetWorld();
   if (World)
   {
      APlayerController* Controller = World->GetFirstPlayerController();

      if (Controller && SlashOverlayClass)
      {
         SlashOverlay = CreateWidget<USlashOverlay>(Controller, SlashOverlayClass);
         SlashOverlay->AddToViewport();
      }
   }
}
