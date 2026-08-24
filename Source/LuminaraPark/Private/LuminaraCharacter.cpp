#include "LuminaraCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"
#include "ByteInteractPoint.h"
#include <InteractInterface.h>

// Sets default values
ALuminaraCharacter::ALuminaraCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Construct the character.

	// Camera
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(GetMesh());
	MainCamera->bUsePawnControlRotation = false;

	// Byte SpringArm
	ByteSpring = CreateDefaultSubobject<USpringArmComponent>(TEXT("ByteSpring"));
	ByteSpring->SetupAttachment(MainCamera);
	ByteSpring->TargetArmLength = 50.0f;
	ByteSpring->bUsePawnControlRotation = false;

	// Byte Anchor Point
	ByteAnchorPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ByteAnchorPoint"));
	ByteAnchorPoint->SetupAttachment(ByteSpring);

	// Byte Mesh
	ByteMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ByteMesh"));
	ByteMesh->SetupAttachment(ByteAnchorPoint);

}

// Called when the game starts or when spawned
void ALuminaraCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Setup Enhanced Input System
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		// Equivalent to "Get Enhanced Input Local Player Subsystem" node (with null check)
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// If we have the mapping context, add it.
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// Create Widget and add it to Viewport (Replaces Create UI Widget in BP)
	if (HUDWidgetClass)
	{

		// Create the Widget variable
		UUserWidget* HUDWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidget)
		{
			// Add the widget to viewport
			HUDWidget->AddToViewport();
		}
	}
	
	// Setup Timeline for Byte Shake
	ShakeTimelineUpdateDelegate.BindUFunction(this, FName("ByteShakeUpdate"));
	ByteShake.AddInterpFloat(ShakeCurve, ShakeTimelineUpdateDelegate);

	// Setup Timeline for Byte Dock
	DockTimelineUpdateDelegate.BindUFunction(this, FName("ByteDockUpdate"));
	ByteDock.AddInterpFloat(DockCurve, DockTimelineUpdateDelegate);

	DockTimelineFinishedDelegate.BindUFunction(this, FName("ByteDockFinished"));
	ByteDock.SetTimelineFinishedFunc(DockTimelineFinishedDelegate);
}

void ALuminaraCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Get the controller rotation and turn that into a rotator
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get the forward vector of the yaw rotation.
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// Get the right vector of the yaw rotation.
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Add movement input based on the forward and right directions.
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ALuminaraCharacter::Look(const FInputActionValue& Value)
{
	// Break Vector 2D and put that into LookAxisVector
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		// Negate Y Axis to flip the look up and down.
		AddControllerPitchInput(-LookAxisVector.Y);
	}
}

void ALuminaraCharacter::Interact(const FInputActionValue& Value)
{
	// Perform a line trace to detect interactable objects
	FVector Start = MainCamera->GetComponentLocation();
	FVector ForwardVector = MainCamera->GetForwardVector();
	FVector End = Start + (ForwardVector * InteractionDistance);
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this); // Ignore self
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);

	// Debug - Red line if missed, Green line if hit. Displays for 2 seconds.
	FColor LineColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugLine(GetWorld(), Start, End, LineColor, false, 2.0f, 0, 2.0f);

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();

		// Draw a point where the impact happened
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Yellow, false, 2.0f);

		// Check if the hit actor implements the interactable interface and send the interact event.
		if (HitActor->Implements<UInteractInterface>())
		{
			IInteractInterface::Execute_Interact(HitActor, this);
		}
	}
}

// Activate the currently connected point.
void ALuminaraCharacter::ActivatePoint()
{
	// Check if the player has a connected point and timeline curve and if so, call the ActivatePoint function on it.
	if (ConnectedPoint)
	{
		// Play the Timeline from Start
		ByteShake.PlayFromStart();

		// Activate the point.
		ConnectedPoint->ActivatePoint();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No connected point."));
	}
}


// Tuning functionality. 
void ALuminaraCharacter::Tune(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("Sent Tuning."));
	// Call the Tune_Implementation function on the TunePoint and pass along the input value from the action.
	TunePoint->Tune_Implementation(Value.Get<float>());
}

// Function to set the connected point on the player when interacting with a Connect type interaction point.
void ALuminaraCharacter::SetConnectedPoint(AByteInteractPoint* NewConnectedPoint)
{
	ConnectedPoint = NewConnectedPoint;
}

void ALuminaraCharacter::StartTuning(AByteInteractPoint* PointToTune)
{
	// To tune, we want to disable the default mapping context and add the tuning mapping context. This will allow us to use the tuning input actions.
	
	// Swap Enhanced Input Mapping Contexts
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// Remove the Default Context so normal movement/actions stop firing.
			if (DefaultMappingContext)
			{
				// Remove movement controls immediately so player can't walk away during docking
				Subsystem->RemoveMappingContext(DefaultMappingContext);

				// Set the TunePoint to the point we are tuning.
				TunePoint = PointToTune;
				
				// Save initial Mesh Location
				InitialMeshLocation = ByteMesh->GetComponentLocation();

				// Set stopping tuning to false and play the Dock Timeline from Start
				bStoppingTuning = false;
				ByteDock.PlayFromStart();
			}

			else
			{
				UE_LOG(LogTemp, Warning, TEXT("DefaultMappingContextis not set."));
			}
		}
	}	
}

void ALuminaraCharacter::StopTuning()
{
	// Swap Enhanced Input Mapping Contexts
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// Remove the Inspection Context so tune actions stop firing.
			if (DefaultMappingContext && InspectionMappingContext)
			{
				Subsystem->RemoveMappingContext(InspectionMappingContext);
				bStoppingTuning = true;
				ByteDock.ReverseFromEnd();
			}

			else
			{
				UE_LOG(LogTemp, Warning, TEXT("DefaultMappingContext is not set."));
			}
		}
	}
}

void ALuminaraCharacter::ByteShakeUpdate(float Alpha)
{
	UE_LOG(LogTemp, Warning, TEXT("Shake Alpha Value: %f"), Alpha);

	// take Alpha in, multiply that by shake intensity and then set that to the relative rotation of Byte Mesh on the y axis.
	float newRotation = Alpha * ByteShakeIntensity;
	ByteMesh->SetRelativeRotation(FRotator(newRotation, 0.0f, 0.0f));
}

void ALuminaraCharacter::ByteDockUpdate(float Alpha)
{
	UE_LOG(LogTemp, Warning, TEXT("Dock Alpha Value: %f"), Alpha);

	// Lerp the ByteMesh's relative location from its current transform to the ByteAnchorPoint's transform based on Alpha.
	if (IsValid(TunePoint) && TunePoint->ByteAttachPoint)
	{
		FVector TargetLocation = bStoppingTuning ? ByteAnchorPoint->GetComponentLocation() : TunePoint->ByteAttachPoint->GetComponentLocation();
		FRotator TargetRotation = bStoppingTuning ? ByteAnchorPoint->GetComponentRotation() : TunePoint->ByteAttachPoint->GetComponentRotation();

		FVector NewLocation = FMath::Lerp(InitialMeshLocation, TargetLocation, Alpha);
		FRotator NewRotation = FQuat::Slerp(InitialMeshRotation.Quaternion(), TargetRotation.Quaternion(), Alpha).Rotator();

		ByteMesh->SetWorldLocationAndRotation(NewLocation, NewRotation);
	}
}

void ALuminaraCharacter::ByteDockFinished()
{
	UE_LOG(LogTemp, Log, TEXT("Timeline Finished."));

	// If we aren't stopping tuning, then we are docking in.
	if (!bStoppingTuning)
	{
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				if (InspectionMappingContext)
				{
					UE_LOG(LogTemp, Log, TEXT("Added Inspection Mapping Context."));
					Subsystem->AddMappingContext(InspectionMappingContext, 1);
					ByteMesh->AttachToComponent(TunePoint->ByteAttachPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("InspectionMappingContext is not set."));
				}
			}
		}
	}
	// Otherwise, we are stopping tuning and docking out.
	else
	{
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				if (DefaultMappingContext)
				{
					UE_LOG(LogTemp, Log, TEXT("Added Default Mapping Context."));
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("DefaultMappingContext is not set."));
				}

				ByteMesh->AttachToComponent(ByteAnchorPoint, FAttachmentTransformRules::SnapToTargetIncludingScale);

				if (IsValid(TunePoint))
				{
					TunePoint->ResetInteractionPoint();
					TunePoint = nullptr;
				}
			}
		}
	}
}


// Called every frame
void ALuminaraCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ByteShake.TickTimeline(DeltaTime);
	ByteDock.TickTimeline(DeltaTime);
}

// Called to bind functionality to input
void ALuminaraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Cast to UEnhancedInputComponent and bind actions
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind Movement
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALuminaraCharacter::Move);
		}

		// Bind Looking
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALuminaraCharacter::Look);
		}

		// Bind Interact
		if(InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ALuminaraCharacter::Interact);
		}

		// Bind Activate Point
		if (ActivatePointAction)
		{
			EnhancedInputComponent->BindAction(ActivatePointAction, ETriggerEvent::Triggered, this, &ALuminaraCharacter::ActivatePoint);
		}

		// Bind Tuning
		if(TuneAction)
		{
			EnhancedInputComponent->BindAction(TuneAction, ETriggerEvent::Triggered, this, &ALuminaraCharacter::Tune);
		}
	}
}

