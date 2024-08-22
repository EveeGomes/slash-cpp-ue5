// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SlashHUD.generated.h"

/**
 * 
 */

class USlashOverlay;

UCLASS()
class SLASH_API ASlashHUD : public AHUD
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Slash")
	TSubclassOf<USlashOverlay> SlashOverlayClass;

	UPROPERTY()
	TObjectPtr<USlashOverlay> SlashOverlay;

protected:
	virtual void BeginPlay() override;

public:
	FORCEINLINE USlashOverlay* GetSlashOverlay() const { return SlashOverlay; }
};
