// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include <LuminaraCharacter.h>
#include "InteractInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LUMINARAPARK_API IInteractInterface
{
	GENERATED_BODY()

public:
	// Main interaction trigger
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void Interact(ALuminaraCharacter* Interactor);

	// Returns the name/prompt string to display on HUD (e.g., "Open Door", "Inspect")
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	FText GetInteractableName();

	// Tune, to send over a value with the event.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void Tune(float Value);

	// Fetch dock transform/component data for inspection or lock-in mechanics
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void GetDockTransformAndComponent(FTransform& OutTransform, USceneComponent*& OutComponent);

	// Reset interaction point state
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void ResetInteractPoint();

};
