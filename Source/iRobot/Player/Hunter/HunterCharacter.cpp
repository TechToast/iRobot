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

DEFINE_LOG_CATEGORY_STATIC(LogHunterChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AHunterCharacter

AHunterCharacter::AHunterCharacter()
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
	GetMesh()->SetReceivesDecals(false);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//GetMesh()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	//GetMesh()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	//GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	//GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	//GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
}


void AHunterCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHunterCharacter, CurrentWeapon);
}


void AHunterCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	if (PlayerInputComponent)
	{
		// Bind fire event
		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AHunterCharacter::OnFireButtonHeld);
		PlayerInputComponent->BindAction("Fire", IE_Released, this, &AHunterCharacter::OnFireButtonReleased);
	}
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


void AHunterCharacter::SetCurrentWeapon(AWeapon* NewWeapon)
{
	// Unequip the current weapon
	if (CurrentWeapon)
		CurrentWeapon->OnUnEquip();

	CurrentWeapon = NewWeapon;

	// Equip the new one
	if (CurrentWeapon)
	{
		// Ensure owning pawn is set in case replication hasn't happened yet
		CurrentWeapon->SetOwningPawn(this);
		CurrentWeapon->OnEquip();
	}
}


void AHunterCharacter::OnRep_CurrentWeapon()
{
	SetCurrentWeapon(CurrentWeapon);
}


/*void AHunterCharacter::FireShot()
{
	FHitResult OutHit;
	FVector Start = FPCameraComponent->GetComponentLocation();
	FVector ForwardVector = FPCameraComponent->GetForwardVector();
	FVector End = ((ForwardVector * 5000.f) + Start);
	FCollisionQueryParams CollisionParams;

	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			DrawDebugLine(GetWorld(), FPMuzzleLocation->GetComponentLocation(), OutHit.ImpactPoint, FColor::Red, false, 0.1f, 0, 1);
			DrawDebugPoint(GetWorld(), OutHit.ImpactPoint, 5.f, FColor::Red, false, 0.25f, 0);

			// Did we hit a shootable object?
			if (OutHit.Actor->GetClass()->ImplementsInterface(UShootable::StaticClass()))
			{
				IShootable* ShootableObject = Cast<IShootable>(OutHit.Actor);
				ShootableObject->OnShot(OutHit.ImpactPoint, ForwardVector * GetCurrentWeaponImpulse(), OutHit.BoneName);
			}
		}
	}
	else
		DrawDebugLine(GetWorld(), FPCameraComponent->GetComponentLocation(), End, FColor::Green, false, 0.1f, 0, 1);

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Start);
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = FPMesh->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}*/


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
			SetCurrentWeapon(Weapon);
		}
		else
		{
			//SERVER_EquipWeapon(Weapon);
		}
	}
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