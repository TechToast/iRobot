#include "iRobotCharacter.h"


AiRobotCharacter::AiRobotCharacter()
{
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
}


void AiRobotCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		SpawnDefaultInventory();
	}
}


void AiRobotCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
		PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

		PlayerInputComponent->BindAxis("MoveForward", this, &AiRobotCharacter::MoveForward);
		PlayerInputComponent->BindAxis("MoveRight", this, &AiRobotCharacter::MoveRight);

		// We have 2 versions of the rotation bindings to handle different kinds of devices differently
		// "turn" handles devices that provide an absolute delta, such as a mouse.
		// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
		PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
		PlayerInputComponent->BindAxis("TurnRate", this, &AiRobotCharacter::TurnAtRate);
		PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
		PlayerInputComponent->BindAxis("LookUpRate", this, &AiRobotCharacter::LookUpAtRate);
	}
}


void AiRobotCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}


void AiRobotCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}


void AiRobotCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}


void AiRobotCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}