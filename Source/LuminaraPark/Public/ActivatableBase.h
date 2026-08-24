// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActivateInterface.h"
#include "ActivatableBase.generated.h"

UCLASS()
class LUMINARAPARK_API AActivatableBase : public AActor, public IActivateInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AActivatableBase();
	virtual void Activate_Implementation() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
