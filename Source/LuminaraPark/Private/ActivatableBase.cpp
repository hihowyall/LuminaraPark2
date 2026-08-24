// Fill out your copyright notice in the Description page of Project Settings.


#include "ActivatableBase.h"

// Sets default values
AActivatableBase::AActivatableBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AActivatableBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AActivatableBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// ADD THIS: The missing interface function implementation!
void AActivatableBase::Activate_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Activate function called on %s"), *GetName());
}

