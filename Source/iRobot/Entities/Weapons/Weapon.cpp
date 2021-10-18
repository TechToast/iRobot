#include "Weapon.h"
#include "iRobot.h"
#include "Player/iRobotPlayerController.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
//#include "Bots/ShooterAIController.h"
//#include "Online/ShooterPlayerState.h"
//#include "UI/ShooterHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
//#include "GameFramework/ForceFeedbackEffect.h"

AWeapon::AWeapon(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	//SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh1P"));
	Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetReceivesDecals(false);
	Mesh1P->SetCastShadow(false);
	Mesh1P->SetHiddenInGame(true);
	Mesh1P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = Mesh1P;

	Mesh3P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh3P"));
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->SetReceivesDecals(false);
	Mesh3P->SetCastShadow(true);
	Mesh3P->SetHiddenInGame(true);
	Mesh3P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh3P->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	//Mesh3P->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	Mesh3P->SetupAttachment(Mesh1P);

	bLoopedMuzzleFX = false;
	bLoopedFireAnim = false;
	bPlayingFireAnim = false;
	bIsEquipped = false;
	bWantsToFire = false;
	bPendingEquip = false;
	CurrentState = EWeaponState::Idle;

	BurstCounter = 0;
	LastFireTime = 0.0f;
}


void AWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, OwningPawn);
	DOREPLIFETIME_CONDITION(AWeapon, BurstCounter, COND_SkipOwner);
	//DOREPLIFETIME_CONDITION(AWeapon, bPendingReload, COND_SkipOwner);
}


void AWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//DetachMeshFromPawn();
}


void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	/*if (Mesh1P)
	{
		AnimInstance1P = Cast<UWeaponAnimInstance>(Mesh1P->GetAnimInstance());
		//ensureMsgf(AnimInstance1P.IsValid(), TEXT("AWeapon::BeginPlay() - Anim instance (1P) doesn't inherit from UWeaponAnimInstance."));
	}

	if (Mesh3P)
	{
		AnimInstance3P = Cast<UWeaponAnimInstance>(Mesh3P->GetAnimInstance());
		//ensureMsgf(AnimInstance3P.IsValid(), TEXT("AWeapon::BeginPlay() - Anim instance (3P) doesn't inherit from UWeaponAnimInstance."));
	}*/
}


void AWeapon::Destroyed()
{
	Super::Destroyed();

	StopSimulatingWeaponFire();
}


void AWeapon::StartFire()
{
	// Clients should request firing start on the server
	if (GetLocalRole() < ROLE_Authority)
	{
		SERVER_StartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();

		/*if (AnimInstance1P.IsValid())
			AnimInstance1P->bIsFiring = true;
		if (AnimInstance3P.IsValid())
			AnimInstance3P->bIsFiring = true;*/
	}
}


void AWeapon::StopFire()
{
	if (GetLocalRole() < ROLE_Authority && OwningPawn.IsValid() && OwningPawn->IsLocallyControlled())
	{
		SERVER_StopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}


bool AWeapon::SERVER_StartFire_Validate()
{
	return true;
}


void AWeapon::SERVER_StartFire_Implementation()
{
	StartFire();
}


bool AWeapon::SERVER_StopFire_Validate()
{
	return true;
}


void AWeapon::SERVER_StopFire_Implementation()
{
	StopFire();
}


bool AWeapon::CanFire() const
{
	bool bCanFire = OwningPawn.IsValid() && OwningPawn->CanFire();
	bool bStateOKToFire = ( ( CurrentState ==  EWeaponState::Idle ) || ( CurrentState == EWeaponState::Firing) );	
	return (bCanFire  && bStateOKToFire /*&& bPendingReload*/);
}


void AWeapon::HandleReFiring()
{
	// Update TimerIntervalAdjustment
	UWorld* MyWorld = GetWorld();

	float SlackTimeThisFrame = FMath::Max(0.0f, (MyWorld->TimeSeconds - LastFireTime) - GetTimeBetweenShots());

	/*if (bAllowAutomaticWeaponCatchup)
	{
		TimerIntervalAdjustment -= SlackTimeThisFrame;
	}*/

	HandleFiring();
}


void AWeapon::HandleFiring()
{
	if (CanFire())
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}

		if (OwningPawn.IsValid() && OwningPawn->IsLocallyControlled())
		{
			FireWeapon();

			// Update firing FX on remote clients if function was called on server
			BurstCounter++;
		}
	}
	else if (OwningPawn.IsValid() && OwningPawn->IsLocallyControlled())
	{
		// Stop weapon fire FX, but stay in Firing state
		if (BurstCounter > 0)
		{
			OnBurstFinished();
		}
	}
	else
	{
		OnBurstFinished();
	}

	if (OwningPawn.IsValid() && OwningPawn->IsLocallyControlled())
	{
		// Local client will notify server
		if (GetLocalRole() < ROLE_Authority)
		{
			SERVER_HandleFiring();
		}

		// Setup refire timer
		bRefiring = (CurrentState == EWeaponState::Firing && GetTimeBetweenShots() > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AWeapon::HandleReFiring, FMath::Max<float>(GetTimeBetweenShots() + TimerIntervalAdjustment, SMALL_NUMBER), false);
			TimerIntervalAdjustment = 0.f;
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}


bool AWeapon::SERVER_HandleFiring_Validate()
{
	return true;
}


void AWeapon::SERVER_HandleFiring_Implementation()
{
	HandleFiring();

	if (CanFire())
	{
		// Update firing FX on remote clients
		BurstCounter++;
	}
}


void AWeapon::SetWeaponState(EWeaponState NewState)
{
	const EWeaponState PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}


void AWeapon::DetermineWeaponState()
{
	EWeaponState NewState = EWeaponState::Idle;

	// Already equipped
	if (bIsEquipped)
	{
		// Wants to fire
		if (bWantsToFire && CanFire())
			NewState = EWeaponState::Firing;
	}

	// Currently equipping
	else if (bPendingEquip)
	{
		NewState = EWeaponState::Equipping;
	}

	SetWeaponState(NewState);
}


void AWeapon::OnBurstStarted()
{
	// Start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && GetTimeBetweenShots() > 0.0f && LastFireTime + GetTimeBetweenShots() > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AWeapon::HandleFiring, LastFireTime + GetTimeBetweenShots() - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}


void AWeapon::OnBurstFinished()
{
	// stop firing FX on remote clients
	BurstCounter = 0;

	// Stop firing FX locally, unless it's a dedicated server
	if (GetNetMode() != NM_DedicatedServer)
	{
		StopSimulatingWeaponFire();
	}
	
	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;

	// reset firing interval adjustment
	TimerIntervalAdjustment = 0.0f;
}


FVector AWeapon::GetCameraAim() const
{
	AiRobotPlayerController* const PlayerController = GetInstigatorController<AiRobotPlayerController>();
	FVector FinalAim = FVector::ZeroVector;

	// If we have a player controller use it for the aim
	if (PlayerController)
	{
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	else if (GetInstigator())
	{
		FinalAim = GetInstigator()->GetBaseAimRotation().Vector();
	}

	return FinalAim;
}


FVector AWeapon::GetCameraTraceStartLocation(const FVector& AimDir) const
{
	AiRobotPlayerController* PC = OwningPawn.IsValid() ? Cast<AiRobotPlayerController>(OwningPawn->Controller) : NULL;
	FVector OutStartTrace = FVector::ZeroVector;
	if (PC)
	{
		// use player's camera
		FRotator UnusedRot;
		PC->GetPlayerViewPoint(OutStartTrace, UnusedRot);

		// Adjust trace so there is nothing blocking the ray between the camera and the pawn, and calculate distance from adjusted start
		OutStartTrace = OutStartTrace + AimDir * ((GetInstigator()->GetActorLocation() - OutStartTrace) | AimDir);
	}

	return OutStartTrace;
}


FVector AWeapon::GetMuzzleLocation() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketLocation(MuzzleAttachPoint);
}


FVector AWeapon::GetMuzzleDirection() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}


FHitResult AWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{
	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, COLLISION_WEAPON, TraceParams);

	return Hit;
}


FHitResult AWeapon::WeaponSweep(const FVector& StartTrace, const FVector& EndTrace, const FCollisionShape& Shape) const
{
	// Perform sweep to retrieve hit info
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->SweepSingleByChannel(Hit, StartTrace, EndTrace, FQuat::Identity, COLLISION_WEAPON, Shape, TraceParams);

	return Hit;
}


void AWeapon::SetOwningPawn(AHunterCharacter* NewOwner)
{
	if (OwningPawn.Get() != NewOwner)
	{
		SetInstigator(NewOwner);
		OwningPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
	}	
}


void AWeapon::OnRep_OwningPawn()
{
	/*if (OwningPawn)
	{
		OnEnterInventory(OwningPawn);
	}
	else
	{
		OnLeaveInventory();
	}*/
}


void AWeapon::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		SimulateWeaponFire();
	}
	else
	{
		StopSimulatingWeaponFire();
	}
}


USkeletalMeshComponent* AWeapon::GetWeaponMesh() const
{
	return (OwningPawn.IsValid() != NULL && OwningPawn->IsFirstPerson()) ? Mesh1P : Mesh3P;
}


void AWeapon::OnEquip()
{
	AttachMeshToPawn();

	bPendingEquip = true;
	DetermineWeaponState();

	// TODO: Add timer and animation for weapon swap
	if (false)
	{
	}
	else
	{
		OnEquipFinished();
	}

	if (OwningPawn.IsValid() && OwningPawn->IsLocallyControlled())
	{
		PlayWeaponSound(EquipSound);
	}
}


void AWeapon::OnEquipFinished()
{
	AttachMeshToPawn();

	bIsEquipped = true;
	bPendingEquip = false;

	DetermineWeaponState();
	
	OnWeaponEquipFinished.Broadcast();
}


void AWeapon::OnUnEquip()
{
	DetachMeshFromPawn();
	bIsEquipped = false;
	StopFire();

	if (bPendingEquip)
	{
		bPendingEquip = false;
	}

	DetermineWeaponState();
}


void AWeapon::OnEnterInventory(AHunterCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}


void AWeapon::OnLeaveInventory()
{
	if (IsAttachedToPawn())
	{
		OnUnEquip();
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		SetOwningPawn(NULL);
	}
}


void AWeapon::AttachMeshToPawn()
{
	if (OwningPawn.IsValid())
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = OwningPawn->GetWeaponAttachPoint();
		if (OwningPawn->IsLocallyControlled() == true)
		{
			USkeletalMeshComponent* PawnMesh1p = OwningPawn->GetFPMesh();
			USkeletalMeshComponent* PawnMesh3p = OwningPawn->GetMesh();
			Mesh1P->SetHiddenInGame(false);
			Mesh3P->SetHiddenInGame(false);
			Mesh1P->AttachToComponent(PawnMesh1p, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
			Mesh3P->AttachToComponent(PawnMesh3p, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
		}
		else
		{
			USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
			USkeletalMeshComponent* UsePawnMesh = OwningPawn->GetPawnMesh();
			UseWeaponMesh->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
			UseWeaponMesh->SetHiddenInGame(false);
		}
	}
}


void AWeapon::DetachMeshFromPawn()
{
	Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh1P->SetHiddenInGame(true);

	Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh3P->SetHiddenInGame(true);
}


UAudioComponent* AWeapon::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = NULL;
	if (Sound && OwningPawn.IsValid())
	{
		AC = UGameplayStatics::SpawnSoundAttached(Sound, OwningPawn->GetRootComponent());
	}

	return AC;
}


float AWeapon::PlayWeaponAnimation(const FWeaponAnim& Animation)
{
	float Duration = 0.0f;
	if (OwningPawn.IsValid())
	{
		UAnimMontage* UseAnim = OwningPawn->IsFirstPerson() ? Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			Duration = OwningPawn->PlayAnimMontage(UseAnim);
		}

		/*if (AnimInstance1P.IsValid())
			AnimInstance1P->bIsFiring = false;
		if (AnimInstance3P.IsValid())
			AnimInstance3P->bIsFiring = false;*/
	}

	return Duration;
}


void AWeapon::StopWeaponAnimation(const FWeaponAnim& Animation)
{
	if (OwningPawn.IsValid())
	{
		UAnimMontage* UseAnim = OwningPawn->IsFirstPerson() ? Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			OwningPawn->StopAnimMontage(UseAnim);
		}
	}
}


void AWeapon::SimulateWeaponFire()
{
	if (GetLocalRole() == ROLE_Authority && CurrentState != EWeaponState::Firing)
	{
		return;
	}

	if (MuzzleFX)
	{
		USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
		if (!bLoopedMuzzleFX || MuzzlePSC == NULL)
		{
			// Split screen requires we create 2 effects. One that we see and one that the other player sees.
			/*if ((MyPawn != NULL) && (MyPawn->IsLocallyControlled() == true))
			{
				AController* PlayerCon = MyPawn->GetController();
				if (PlayerCon != NULL)
				{
					Mesh1P->GetSocketLocation(MuzzleAttachPoint);
					MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh1P, MuzzleAttachPoint);
					MuzzlePSC->bOwnerNoSee = false;
					MuzzlePSC->bOnlyOwnerSee = true;

					Mesh3P->GetSocketLocation(MuzzleAttachPoint);
					MuzzlePSCSecondary = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh3P, MuzzleAttachPoint);
					MuzzlePSCSecondary->bOwnerNoSee = true;
					MuzzlePSCSecondary->bOnlyOwnerSee = false;
				}
			}
			else*/
			{
				MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, UseWeaponMesh, MuzzleAttachPoint);
			}
		}
	}

	if (!bLoopedFireAnim || !bPlayingFireAnim)
	{
		PlayWeaponAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	if (bLoopedFireSound)
	{
		if (FireAC == NULL)
		{
			FireAC = PlayWeaponSound(FireLoopSound);
		}
	}
	else
	{
		PlayWeaponSound(FireSound);
	}

	AiRobotPlayerController* PC = OwningPawn.IsValid() ? Cast<AiRobotPlayerController>(OwningPawn->Controller) : NULL;
	if (PC != NULL && PC->IsLocalController())
	{
		if (FireCameraShake != NULL)
		{
#if ENGINE_MINOR_VERSION > 26
			PC->ClientStartCameraShake(FireCameraShake, 1);
#else
			PC->ClientPlayCameraShake(FireCameraShake, 1);
#endif
		}
		/*if (FireForceFeedback != NULL && PC->IsVibrationEnabled())
		{
			FForceFeedbackParameters FFParams;
			FFParams.Tag = "Weapon";
			PC->ClientPlayForceFeedback(FireForceFeedback, FFParams);
		}*/
	}
}


void AWeapon::StopSimulatingWeaponFire()
{
	if (bLoopedMuzzleFX)
	{
		if (MuzzlePSC != NULL)
		{
			MuzzlePSC->DeactivateSystem();
			MuzzlePSC = NULL;
		}
		/*if (MuzzlePSCSecondary != NULL)
		{
			MuzzlePSCSecondary->DeactivateSystem();
			MuzzlePSCSecondary = NULL;
		}*/
	}

	if (bLoopedFireAnim && bPlayingFireAnim)
	{
		StopWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}

	if (FireAC)
	{
		FireAC->FadeOut(0.1f, 0.0f);
		FireAC = NULL;

		PlayWeaponSound(FireFinishSound);
	}
}