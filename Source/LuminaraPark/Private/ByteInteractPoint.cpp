#include "ByteInteractPoint.h"
#include "ActivatableBase.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AByteInteractPoint::AByteInteractPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Construct the components
	Pivot = CreateDefaultSubobject<UArrowComponent>(TEXT("Pivot"));
	Pivot->SetupAttachment(RootComponent);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Pivot);

	ByteAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ByteAttachPoint"));
	ByteAttachPoint->SetupAttachment(Pivot);

}

// Called when the game starts or when spawned
void AByteInteractPoint::BeginPlay()
{
	Super::BeginPlay();
	

	// Setup Timeline for Reset Timeline
	ResetTimelineUpdateDelegate.BindUFunction(this, FName("ResetTimelineUpdate"));
	ResetTimeline.AddInterpFloat(ResetCurve, ResetTimelineUpdateDelegate);
}

void AByteInteractPoint::ResetTimelineUpdate(float Alpha)
{
	FRotator NewRotation = FMath::Lerp(InitialResetRotation, FRotator (InitialResetRotation.Pitch, InitialResetRotation.Yaw, 0.0f), Alpha);
	Pivot->SetRelativeRotation(NewRotation);
}

// Called every frame
void AByteInteractPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ResetTimeline.TickTimeline(DeltaTime);

}

void AByteInteractPoint::Interact_Implementation(ALuminaraCharacter* Interactor)
{
	if(!IsValid(TargetedObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetedObject is not valid."));
		return;
	}

	if(TargetedObject->GetClass()->ImplementsInterface(UActivateInterface::StaticClass()))
	{
		switch (InteractionType)
		{
		case::EInteractionType::Basic:
			// Notify the targeted object of the interaction.
			IActivateInterface::Execute_Activate(TargetedObject);
			break;

		case::EInteractionType::Connect:
			// Notify the player character of the connection, so it can handle the connection logic.
			// Only the player character can interact with the connection point, so we can avoid casting it.
			Interactor->SetConnectedPoint(this);
			break;

		case::EInteractionType::Tune:
			// Notify the targeted object of the tuning interaction, so it can start the tuning process
			Interactor->StartTuning(this);
			break;

		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetedObject does not implement IActivateInterface."));
	}

}

// Logic for activating the point. This is used for the Connect interaction type.
void AByteInteractPoint::ActivatePoint()
{
	if(!IsValid(TargetedObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetedObject is not valid."));
		return;
	}
	if(TargetedObject->GetClass()->ImplementsInterface(UActivateInterface::StaticClass()))
	{
		IActivateInterface::Execute_Activate(TargetedObject);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetedObject does not implement IActivateInterface."));
	}
}

void AByteInteractPoint::ResetInteractionPoint()
{
	InitialResetRotation = Pivot->GetRelativeRotation(); // Cache starting rotation
	ResetTimeline.PlayFromStart();
}


// Logic for actually tuning the point. On the player tune input, we will trigger this function and rotate the dial based on the input value.
void AByteInteractPoint::Tune_Implementation(float Value)
{
	UE_LOG(LogTemp, Log, TEXT("Recieved Tuning with value: %f"), Value);

	//  Get the current rotation of the pivot
	FRotator CurrentRotation = Pivot->GetRelativeRotation();
	float CurrenRoll = FRotator::NormalizeAxis(CurrentRotation.Roll); // Normalize the current roll to be between -180 and 180 degrees, so we don't have issues with clamping.

	// If RotationThreshold is set to a value greater than 0, that means we have to rotate to the right to reach the threshold, meaning needs to be stopped to the left.
	// If RotationThreshold is set to a value less than 0, that means we have to rotate to the left to reach the threshold, meaning needs to be stopped to the right.
	// TLDR: Different Clamps depending on the sign of the RotationThreshold.
	if (RotationThreshold > 0)
	{
		float NewRoll = FMath::Clamp(CurrenRoll + Value, RotationMin, RotationThreshold);
		// Now that it's clamped, we can set the new rotation of the pivot.
		Pivot->SetRelativeRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, NewRoll));

		// If the new roll is greater than or equal to the RotationThreshold, that means we have reached the threshold and can activate the point.
		if (NewRoll >= RotationThreshold)
		{
			IActivateInterface::Execute_Activate(TargetedObject);

			// Later replace this cast, but for now, we can just get the player character and tell it to stop tuning, since we have reached the threshold and activated the point.
			if (ALuminaraCharacter* Character = Cast<ALuminaraCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn()))
			{
				Character->StopTuning();
			}
		}
	}
	else
	{
		float NewRoll = FMath::Clamp(CurrenRoll + Value, RotationThreshold, RotationMin);
		// Now that it's clamped, we can set the new rotation of the pivot.
		Pivot->SetRelativeRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, NewRoll));

		// If the new roll is less than or equal to the RotationThreshold, that means we have reached the threshold and can activate the point.
		if (NewRoll <= RotationThreshold)
		{
			IActivateInterface::Execute_Activate(TargetedObject);

			// Later replace this cast, but for now, we can just get the player character and tell it to stop tuning, since we have reached the threshold and activated the point.
			if (ALuminaraCharacter* Character = Cast<ALuminaraCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn()))
			{
				Character->StopTuning();
			}
		}
	};
}

