// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "MyHealthBarComponent.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API UMyHealthBarComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	/** Access the user widget used by this component and set its percentage */
	void SetHealthBarPercent(float Percent);

private:
	UPROPERTY()
	TObjectPtr<class UHealthBar> HealthBarWidget;
};
