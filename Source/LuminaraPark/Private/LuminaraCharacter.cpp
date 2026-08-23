#include "LuminaraCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ALuminaraCharacter::ALuminaraCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

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

		// Annd movement input based on the forward and right directions.
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
		// Draw a point where the impact happened
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Yellow, false, 2.0f);

		//// Check if the hit actor implements the interactable interface
		//if (HitResult.GetActor()->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		//{
		//	IInteractableInterface::Execute_Interact(HitResult.GetActor(), this);
		//}
	}
}

// Called every frame
void ALuminaraCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	}
}

