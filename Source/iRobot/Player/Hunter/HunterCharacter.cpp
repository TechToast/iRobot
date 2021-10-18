#include "HunterCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/iRobotPlayerController.h"
#include "Entities/Weapons/Weapon.h"
#include "UI/iRobotHUD.h"

DEFINE_LOG_CATEGORY_STATIC(LogHunterChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AHunterCharacter

AHunterCharacter::AHunterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create a CameraComponent	
	FPCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FPCameraComponent->SetupAttachment(GetCapsuleComponent());
	FPCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FPCameraComponent->bUsePawnControlRotation = true;

	// First person mesh (Arms)
	FPMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonCharacterMesh"));
	FPMesh->SetOnlyOwnerSee(true);
	FPMesh->SetupAttachment(FPCameraComponent);
	FPMesh->bCastDynamicShadow = false;
	FPMesh->SetCastShadow(false);
	FPMesh->SetReceivesDecals(false);
	FPMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	FPMesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	FPMesh->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	FPMesh->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));
	FPMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FPMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	// Regular mesh (character)
	GetMesh()->SetOnlyOwnerSee(false);
	GetMesh()->SetOwnerNoSee(true);

	// Collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Set up the interaction capabilities for this character here
	SetInteractionCapability(INTERACTION_CAPABILITY_OpenDoors);
}


void AHunterCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHunterCharacter, CurrentWeapon);
	DOREPLIFETIME(AHunterCharacter, Inventory);
}


void AHunterCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAction("[HUNTER]Fire", IE_Pressed, this, &AHunterCharacter::OnFireButtonHeld);
		PlayerInputComponent->BindAction("[HUNTER]Fire", IE_Released, this, &AHunterCharacter::OnFireButtonReleased);

		PlayerInputComponent->BindAction("[HUNTER]Interact", IE_Pressed, this, &AHunterCharacter::OnInteractButtonPressed);

		PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &AHunterCharacter::OnNextWeapon);
		PlayerInputComponent->BindAction("PrevWeapon", IE_Pressed, this, &AHunterCharacter::OnPrevWeapon);
	}
}


FTransform AHunterCharacter::GetCameraTransform() const
{
	return FPCameraComponent->GetComponentTransform();
}


void AHunterCharacter::OnFireButtonHeld()
{
	AiRobotPlayerController* PC = Cast<AiRobotPlayerController>(Controller);
	if (PC && PC->IsGameInputAllowed())
	{
		if (!bWantsToFire)
		{
			bWantsToFire = true;
			if (CurrentWeapon)
			{
				CurrentWeapon->StartFire();
			}
		}
	}
}


void AHunterCharacter::OnFireButtonReleased()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFire();
		}
	}
}


bool AHunterCharacter::CanFire() const
{
	//return IsAlive();
	return true;
}


void AHunterCharacter::OnNextWeapon()
{
	AiRobotPlayerController* MyPC = Cast<AiRobotPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (Inventory.Num() >= 2 && (CurrentWeapon == NULL || CurrentWeapon->GetCurrentState() != EWeaponState::Equipping))
		{
			const int32 CurrentWeaponIdx = Inventory.IndexOfByKey(CurrentWeapon);
			AWeapon* NextWeapon = Inventory[(CurrentWeaponIdx + 1) % Inventory.Num()];
			EquipWeapon(NextWeapon);
		}
	}
}


void AHunterCharacter::OnPrevWeapon()
{
	AiRobotPlayerController* MyPC = Cast<AiRobotPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (Inventory.Num() >= 2 && (CurrentWeapon == NULL || CurrentWeapon->GetCurrentState() != EWeaponState::Equipping))
		{
			const int32 CurrentWeaponIdx = Inventory.IndexOfByKey(CurrentWeapon);
			AWeapon* PrevWeapon = Inventory[(CurrentWeaponIdx - 1 + Inventory.Num()) % Inventory.Num()];
			EquipWeapon(PrevWeapon);
		}
	}
}


void AHunterCharacter::SetCurrentWeapon(AWeapon* NewWeapon, AWeapon* LastWeapon)
{
	AWeapon* LocalLastWeapon = nullptr;

	if (LastWeapon != nullptr)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	// Unequip previous
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
	}

	CurrentWeapon = NewWeapon;

	// Equip the new one
	if (CurrentWeapon)
	{
		// Ensure owning pawn is set in case replication hasn't happened yet
		CurrentWeapon->SetOwningPawn(this);

		// Listen for equipping finished event
		OnWeaponEquipFinishedHandle = CurrentWeapon->GetOnWeaponEquipFinished()->AddUObject(this, &AHunterCharacter::OnWeaponEquipFinished);

		// Equip it
		CurrentWeapon->OnEquip();
	}
}


void AHunterCharacter::OnWeaponEquipFinished()
{
	if (CurrentWeapon)
	{
		if (!HUD.IsValid())
		{
			if (Controller)
			{
				AiRobotPlayerController* PlayerController = Cast<AiRobotPlayerController>(Controller);
				if (PlayerController)
					HUD = Cast<AiRobotHUD>(PlayerController->GetHUD());
			}
		}

		if (HUD.IsValid())
		{
			HUD->SetCrosshairTexture(CurrentWeapon->GetCrosshairTexture());
		}

		if (OnWeaponEquipFinishedHandle.IsValid())
		{
			CurrentWeapon->GetOnWeaponEquipFinished()->Remove(OnWeaponEquipFinishedHandle);
			OnWeaponEquipFinishedHandle.Reset();
		}
	}
}


void AHunterCharacter::OnRep_CurrentWeapon(AWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}


void AHunterCharacter::SpawnDefaultInventory()
{
	Super::SpawnDefaultInventory();

	// Only on server
	if (GetLocalRole() < ROLE_Authority)
		return;

	int32 NumWeaponClasses = DefaultInventoryClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (!DefaultInventoryClasses[i].IsNull())
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AWeapon* NewWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultInventoryClasses[i].LoadSynchronous(), SpawnInfo);
			AddWeapon(NewWeapon);
		}
	}

	// Equip first weapon in inventory
	if (Inventory.Num() > 0)
	{
		EquipWeapon(Inventory[0]);
	}
}


void AHunterCharacter::AddWeapon(AWeapon* Weapon)
{
	if (Weapon && GetLocalRole() == ROLE_Authority)
	{
		Weapon->OnEnterInventory(this);
		Inventory.AddUnique(Weapon);
	}
}


void AHunterCharacter::RemoveWeapon(AWeapon* Weapon)
{
	if (Weapon && GetLocalRole() == ROLE_Authority)
	{
		Weapon->OnLeaveInventory();
		Inventory.RemoveSingle(Weapon);
	}
}


void AHunterCharacter::EquipWeapon(AWeapon* Weapon)
{
	if (Weapon)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			SetCurrentWeapon(Weapon, CurrentWeapon);
		}
		else
		{
			SERVER_EquipWeapon(Weapon);
		}
	}
}


bool AHunterCharacter::SERVER_EquipWeapon_Validate(AWeapon* Weapon)
{
	return true;
}


void AHunterCharacter::SERVER_EquipWeapon_Implementation(AWeapon* Weapon)
{
	EquipWeapon(Weapon);
}


USkeletalMeshComponent* AHunterCharacter::GetPawnMesh() const
{
	return IsFirstPerson() ? FPMesh : GetMesh();
}


bool AHunterCharacter::IsFirstPerson() const
{
	return /*IsAlive() &&*/ Controller && Controller->IsLocalPlayerController();
}


FRotator AHunterCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}


float AHunterCharacter::PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance)
	{
		return UseMesh->AnimScriptInstance->Montage_Play(AnimMontage, InPlayRate);
	}

	return 0.0f;
}


void AHunterCharacter::StopAnimMontage(class UAnimMontage* AnimMontage)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance &&
		UseMesh->AnimScriptInstance->Montage_IsPlaying(AnimMontage))
	{
		UseMesh->AnimScriptInstance->Montage_Stop(AnimMontage->BlendOut.GetBlendTime(), AnimMontage);
	}
}


void AHunterCharacter::StopAllAnimMontages()
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (UseMesh && UseMesh->AnimScriptInstance)
	{
		UseMesh->AnimScriptInstance->Montage_Stop(0.0f);
	}
}


void AHunterCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, APawn* InstigatingPawn, AActor* DamageCauser)
{
	Super::OnDeath(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser);

	// Remove all weapons
	//DestroyInventory();

	// Switch back to 3rd person view
	//UpdatePawnMeshes();

	//StopAllAnimMontages();
}