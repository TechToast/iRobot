#include "EscapeeCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EscapeeCharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/iRobotPlayerController.h"
#include "EscapeeAnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "Entities/HidingPlace/HidingPlaceUtils.h"
#include "Entities/HidingPlace/IHideCompatible.h"
#include "EscapeeCharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


//////////////////////////////////////////////////////////////////////////
// AEscapeeCharacter

AEscapeeCharacter::AEscapeeCharacter(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UEscapeeCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Set up the interaction capabilities for this character here
	SetInteractionCapability(INTERACTION_CAPABILITY_OpenDoors);
	SetInteractionCapability(INTERACTION_CAPABILITY_DissolveRobot);
}


/*void AEscapeeCharacter::Tick(float Delta)
{
	Super::Tick(Delta);
}*/


void AEscapeeCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AEscapeeCharacter, HideState, COND_None);
	DOREPLIFETIME_CONDITION(AEscapeeCharacter, bScanned, COND_None);
	DOREPLIFETIME_CONDITION(AEscapeeCharacter, CurrentHidingPlaceTransform, COND_OwnerOnly);
}


void AEscapeeCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetMesh())
	{
		AnimInstance = Cast<UEscapeeAnimInstance>(GetMesh()->GetAnimInstance());
		ensureMsgf(AnimInstance.IsValid(), TEXT("AEscapeeCharacter::BeginPlay() - Anim instance doesn't inherit from UEscapeeAnimInstance."));

		if (GetNetMode() != NM_DedicatedServer)
		{
			if (ScannedStateMaterialIndex < GetMesh()->GetNumMaterials() && ScannedStateParamName != NAME_None)
			{
				ScannedStateMaterial = GetMesh()->CreateDynamicMaterialInstance(ScannedStateMaterialIndex);
			}
		}
	}

	EscapeeCharMove = Cast<UEscapeeCharacterMovementComponent>(GetMovementComponent());
}


void AEscapeeCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up gameplay key bindings
	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAction("[ESCAPEE]Interact", IE_Pressed, this, &AEscapeeCharacter::OnInteractButtonPressed);

		//PlayerInputComponent->BindAction("Hide", IE_Pressed, this, &AEscapeeCharacter::OnHideButtonHeld);
		//PlayerInputComponent->BindAction("Hide", IE_Released, this, &AEscapeeCharacter::OnHideButtonReleased);
		PlayerInputComponent->BindAction("Hide", IE_Released, this, &AEscapeeCharacter::OnHideButtonPressed);
	}
}


FTransform AEscapeeCharacter::GetCameraTransform() const
{
	return FollowCamera->GetComponentTransform();
}


void AEscapeeCharacter::MoveForward(float Value)
{
	// No movement if hidden
	if (HideState != EHidingPlaceType::HP_None)
		return;

	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}


void AEscapeeCharacter::MoveRight(float Value)
{
	// No movement if hidden
	if (HideState != EHidingPlaceType::HP_None)
		return;

	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void AEscapeeCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, APawn* InstigatingPawn, AActor* DamageCauser)
{
	Super::OnDeath(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser);
}


void AEscapeeCharacter::Jump()
{
	if (bWantsToHide)
		return;

	Super::Jump();
}


/*void AEscapeeCharacter::OnHideButtonHeld()
{
	AiRobotPlayerController* PC = Cast<AiRobotPlayerController>(Controller);
	if (PC && PC->IsGameInputAllowed())
	{
		Hide();
	}
}


void AEscapeeCharacter::OnHideButtonReleased()
{
	AiRobotPlayerController* PC = Cast<AiRobotPlayerController>(Controller);
	if (PC && PC->IsGameInputAllowed())
	{
		UnHide();
	}
}*/


void AEscapeeCharacter::OnHideButtonPressed()
{
	AiRobotPlayerController* PC = Cast<AiRobotPlayerController>(Controller);
	if (PC && PC->IsGameInputAllowed())
	{
		if (bWantsToHide)
		{
			bWantsToHide = false;
			UnHide();
		}
		else
		{
			bWantsToHide = true;
			Hide();
		}
	}
}


void AEscapeeCharacter::Hide()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		SERVER_Hide();
		return;
	}

	if (HideState == EHidingPlaceType::HP_None)
	{
		TScriptInterface<IHideCompatible> HidingPlace;
		if (GetHidingPlace(HidingPlace))
		{
			HideState = HidingPlace->GetHidingPlaceType();
			OnRep_HideState();

			CurrentHidingPlace = HidingPlace;

			// Prepare the hiding place for us to hide in
			HidingPlaceReadyHandle = CurrentHidingPlace->GetOnHidingPlaceReadyEvent()->AddUObject(this, &AEscapeeCharacter::OnHidingPlaceReady);
			CurrentHidingPlace->PrepareHidingPlace(GetActorLocation());
		}
	}
}


void AEscapeeCharacter::UnHide()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		SERVER_UnHide();
		
		// Disable the rotation override on the movement component
		if (GetLocalRole() == ROLE_AutonomousProxy && EscapeeCharMove.IsValid())
			EscapeeCharMove->SetUseHidingPlaceRotation(false);

		return;
	}

	if (HideState != EHidingPlaceType::HP_None)
	{
		HideState = EHidingPlaceType::HP_None;
		OnRep_HideState();

		CurrentHidingPlace->VacateHidingPlace(GetActorLocation());
	}
}


void AEscapeeCharacter::OnRep_HideState()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (AnimInstance.IsValid())
			AnimInstance->bFrozen = HideState == EHidingPlaceType::HP_Freeze;
	}
}


bool AEscapeeCharacter::SERVER_Hide_Validate()
{
	return true;
}


void AEscapeeCharacter::SERVER_Hide_Implementation()
{
	Hide();
}


bool AEscapeeCharacter::SERVER_UnHide_Validate()
{
	return true;
}


void AEscapeeCharacter::SERVER_UnHide_Implementation()
{
	UnHide();
}


bool AEscapeeCharacter::GetHidingPlace(TScriptInterface<IHideCompatible>& OutHidingPlace) const
{
	float BestDistSquared = FLT_MAX;
	bool bHidingPlaceFound = false;

	const FVector& CharacterLocation = GetActorLocation();

	for (TScriptInterface<IHideCompatible>& HidingPlace : HidingPlaceUtils::GetHidingPlaces())
	{
		if (HidingPlace != nullptr && HidingPlace->IsWithinRange(CharacterLocation, MaxInteractionDistance))
		{
			FTransform HidingPlaceTransform;
			if (HidingPlace->GetHidingPlaceTransform(CharacterLocation, HidingPlaceTransform))
			{
				float DistSq = FVector::DistSquared(CharacterLocation, HidingPlaceTransform.GetLocation());
				if (DistSq < BestDistSquared)
				{
					BestDistSquared = DistSq;
					OutHidingPlace = HidingPlace;
					bHidingPlaceFound = true;
				}
			}
		}
	}

	return bHidingPlaceFound;
}


void AEscapeeCharacter::OnHidingPlaceReady()
{
	if (CurrentHidingPlace != nullptr)
	{
		// Clean up the delegate
		if (HidingPlaceReadyHandle.IsValid())
		{
			CurrentHidingPlace->GetOnHidingPlaceReadyEvent()->Remove(HidingPlaceReadyHandle);
			HidingPlaceReadyHandle.Reset();
		}

		const FVector& CharacterLocation = GetActorLocation();

		// Now hide
		FTransform HidingPlaceTransform;
		if (CurrentHidingPlace->GetHidingPlaceTransform(CharacterLocation, HidingPlaceTransform))
		{
			CurrentHidingPlace->OccupyHidingPlace(CharacterLocation);

			FVector Location = HidingPlaceTransform.GetLocation();
			Location.Z = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			HidingPlaceTransform.SetLocation(Location);
			
			CurrentHidingPlaceTransform.Transform = HidingPlaceTransform;
			CurrentHidingPlaceTransform.TimeStamp = GetWorld()->GetTimeSeconds();
			
			OnRep_CurrentHidingPlaceTransform();
		}
	}
}


void AEscapeeCharacter::OnRep_CurrentHidingPlaceTransform()
{
	TeleportTo(CurrentHidingPlaceTransform.Transform.GetLocation(), CurrentHidingPlaceTransform.Transform.Rotator(), false, true);

	// This is our hacky-fix to get around the CharacterMovementComponent's client prediction which usually 
	// overrides the rotation of the character on the autonomous client
	if (GetLocalRole() == ROLE_AutonomousProxy && EscapeeCharMove.IsValid())
	{
		EscapeeCharMove->SetHidingPlaceRotation(CurrentHidingPlaceTransform.Transform.GetRotation());
		EscapeeCharMove->SetUseHidingPlaceRotation(true);
	}
}


void AEscapeeCharacter::OnScanned(int32 ScannedIndex)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetScannedState(true);

		// Start a timer to disable the state
		GetWorld()->GetTimerManager().SetTimer(ScannedTimerHandle, this, &AEscapeeCharacter::OnScanStateRestored, TimerBeforeScanStateChange, false);
	}
}


void AEscapeeCharacter::OnRep_bScanned()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (ScannedStateMaterial.IsValid())
		{
			ScannedStateMaterial->SetScalarParameterValue(ScannedStateParamName, bScanned ? 1.f : 0.f);
		}

		// Play sound
		if (bScanned)
			UGameplayStatics::PlaySoundAtLocation(this, ScannedSound, GetActorLocation());
	}
}


void AEscapeeCharacter::OnScanStateRestored()
{
	SetScannedState(false);
}


void AEscapeeCharacter::SetScannedState(bool bInState)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bScanned = bInState;
		OnRep_bScanned();
	}
}