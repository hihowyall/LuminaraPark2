// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h" // Needed for Enhanced Input callbacks - check build file to make sure it's included if not compiling.
#include "LuminaraCharacter.generated.h"

// Forward declarations, saves compile time.
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UStaticMesh;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BYTE")
	TObjectPtr<USpringArmComponent> ByteSpring;

	// The anchor point component on the character where BYTE attached/sits
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BYTE")
	TObjectPtr<USceneComponent> ByteAnchorPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BYTE")
	TObjectPtr<UStaticMeshComponent> ByteMesh;

	// Main Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BYTE")
	TObjectPtr<UCameraComponent> MainCamera;


	// Enhanced Input

	// The Default Mapping Context (IMC) set in BeginPlay
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// Movement Input Action (IA_Move)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	// Look Input Action (IA_Look)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;


	//Input Functions

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);


	// UI & Variables

	// UI Widget Class to spawn on BeginPlay (e.g. WBP_MainHUD)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	// Gameplay variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BYTE")
	float ByteShakeIntensity = 10.0f;

	



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
