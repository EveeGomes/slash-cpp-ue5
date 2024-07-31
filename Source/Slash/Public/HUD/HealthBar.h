// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBar.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API UHealthBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** 
	* Using meta specifier as meta = (BindWidget) we're able to bind this variable to
	*  the health bar widget in WBP_HealthBar.
	* For that to work, this variable name and the name of the widget it needs to be bound must match!
	* In WEBP_HealthBar the progress bar widget is named HealthBar, matching the name of this variable.
	* They MUST match!! If using a meta with BindWidget and the names don't match it'll cause problems!!!!!!
	*/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> HealthBar;
};
