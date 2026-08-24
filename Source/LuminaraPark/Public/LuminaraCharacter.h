// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Components/TimelineComponent.h"
#include "LuminaraCharacter.generated.h"

// Forward declarations, saves compile time.
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UStaticMesh;
class AByteInteractPoint;
class UTimelineComponent;
class UCurveFloat;

UCLASS()
class LUMINARAPARK_API ALuminaraCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALuminaraCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Components for the editor

	// Spring Arm for BYTE
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> ByteSpring;

	// The anchor point component on the character where BYTE attached/sits
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> ByteAnchorPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ByteMesh;

	FTimeline ByteShake;

	FTimeline ByteDock;

	// Main Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> MainCamera;


	// Enhanced Input

	// The Default Mapping Context (IMC) set in BeginPlay
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// The Inspection Mapping Context, set this whenever we tune.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InspectionMappingContext;

	// Movement Input Action (IA_Move)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	// Look Input Action (IA_Look)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	// Interact Input Action (IA_Interact)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	// Activate Point Input Action (IA_ActivatePoint)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ActivatePointAction;

	// Tune Input Action (IA_Tune)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> TuneAction;

	//Input Functions

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void ActivatePoint();
	void Tune(const FInputActionValue& Value);


	// UI & Variables

	// UI Widget Class to spawn on BeginPlay
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	// Gameplay variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BYTE")
	float ByteShakeIntensity = 10.0f;

	FVector InitialMeshLocation;
	FRotator InitialMeshRotation;

	// Point to store the connected point when interacting with a Connect type interaction point.
	AByteInteractPoint* ConnectedPoint;

	// Point to store the point we are tuning when interacting with a Tuning type interaction point.
	AByteInteractPoint* TunePoint;

	// Delegate signature for the function which will handle our events.
	FOnTimelineFloat ShakeTimelineUpdateDelegate;
	FOnTimelineFloat DockTimelineUpdateDelegate;
	FOnTimelineEvent DockTimelineFinishedDelegate;

	// Timeline Curve Floats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	TObjectPtr<UCurveFloat> ShakeCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	TObjectPtr<UCurveFloat> DockCurve = nullptr;

	// Timeline Functions
	UFUNCTION()
	void ByteShakeUpdate(float Alpha);

	UFUNCTION()
	void ByteDockUpdate(float Alpha);

	UFUNCTION()
	void ByteDockFinished();

	// Bool to track if we are stopping tuning or not, so we can have two different things happen for ByteDockFinished depending on if we are stopping tuning or not.
	bool bStoppingTuning = false;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Function to set the connected point on the player when interacting with a Connect type interaction point.
	UFUNCTION()
	void SetConnectedPoint(AByteInteractPoint* NewConnectedPoint);

	// Function to start connecting when interacting with a Tuning type interaction point.
	UFUNCTION()
	void StartTuning(AByteInteractPoint* PointToTune);

	// Function to stop tuning when the player is done tuning.
	UFUNCTION()
	void StopTuning();
};
