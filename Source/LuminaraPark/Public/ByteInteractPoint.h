// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractInterface.h"
#include "ByteInteractPoint.generated.h"

// Enum for the various
UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	Basic    UMETA(DisplayName = "Basic Trigger"),
	Connect  UMETA(DisplayName = "Connect"),
	Tune     UMETA(DisplayName = "Tune")
};

class AActivatableBase;
class UArrowComponent;
class UStaticMeshComponent;
class USceneComponent;
class UTimelineComponent;
class UCurveFloat;

UCLASS()
class LUMINARAPARK_API AByteInteractPoint : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AByteInteractPoint();

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> Pivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> ByteAttachPoint;

	// Select the interaction type.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	EInteractionType InteractionType = EInteractionType::Basic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Tune")
	float RotationThreshold = -45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Tune")
	float RotationMin = 10.0f;

	// Target object to notify on successful activation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	AActivatableBase* TargetedObject;

	// Core Execution
	virtual void Interact_Implementation(ALuminaraCharacter* Interactor) override;
	virtual void Tune_Implementation(float Value) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Delegate signature for the function which will handle our events.
	FOnTimelineFloat ResetTimelineUpdateDelegate;

	FTimeline ResetTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	TObjectPtr<UCurveFloat> ResetCurve = nullptr;

	FRotator InitialResetRotation;


	UFUNCTION()
	void ResetTimelineUpdate(float Alpha);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void ActivatePoint();

	UFUNCTION()
	void ResetInteractionPoint();

};
