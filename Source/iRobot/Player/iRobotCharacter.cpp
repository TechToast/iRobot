#include "iRobotCharacter.h"
#include "Net/UnrealNetwork.h"
#include "iRobot.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PawnMovementComponent.h"


AiRobotCharacter::AiRobotCharacter()
{
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	GetMesh()->SetReceivesDecals(false);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
}


void AiRobotCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AiRobotCharacter, Health);
}


void AiRobotCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		SpawnDefaultInventory();

		// Set initial values
		Health = 1;
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


void AiRobotCharacter::OnShot(FVector ImpactLocation, FVector ImpactVelocity, FName BoneName)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		// TODO: Need to pass damage through
		CauseDamage(0.25f);
	}

	// If no bone name was found (non skeletal mesh hit) find the closest now
	//if (BoneName == NAME_None)
	//{
	//	FVector TempVec;
	//	BoneName = GetMesh()->FindClosestBone(ImpactLocation, &TempVec, 0.f, true);
	//}

	//GetMovementComponent()->impul
	//GetMesh()->AddImpulseAtLocation(ImpactVelocity, ImpactLocation, BoneName);
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


void AiRobotCharacter::CauseDamage(float DamageAmount)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (Health > DamageAmount)
			Health -= DamageAmount;
		else
			Health = 0;

		// Server should do OnRep too
		OnRep_Health();
	}
}


void AiRobotCharacter::HealDamage(float HealAmount)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (Health + HealAmount < 1)
			Health += HealAmount;
		else
		{
			Health = 1;
		}
	}
}


void AiRobotCharacter::OnRep_Health()
{
	if (Health <= 0)
		GetMesh()->SetSimulatePhysics(true);
}