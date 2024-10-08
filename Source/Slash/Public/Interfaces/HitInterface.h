// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHitInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SLASH_API IHitInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** 
	* As a BlueprintNativeEvent it can be implemented in BP just like BP implementable events, with the difference 
	*  of having 2 versions(C++ and BP).
	* Also, making it BlueprintNativeEvent it can't be virtual anymore and can't be set to zero either (both settings
	*  for UE system), but it's still overridable.
	* The C++ specific version of this function is virtual and its name has Implementation added to this function's name:
	*  GetHit_Implementation. So in Enemy class for example, its virtual version needs to have _Implementation added to its name!
	*/
	UFUNCTION(BlueprintNativeEvent)
	void GetHit(const FVector& ImpactPoint, AActor* Hitter);
};
