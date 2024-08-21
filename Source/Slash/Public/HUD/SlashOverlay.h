// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlashOverlay.generated.h"

/**
 * 
 */

class UProgressBar;
class UTextBlock;

UCLASS()
class SLASH_API USlashOverlay : public UUserWidget
{
	GENERATED_BODY()

private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CoinText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SoulsText;

public:
	/** Setters and Getters */
	void SetHealthPercent(float Percent);
	void SetStaminaPercent(float Percent);
};
