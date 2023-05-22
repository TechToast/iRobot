#include "iRobotCharacter.h"
#include "Net/UnrealNetwork.h"
#include "iRobot.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/CollisionProfile.h"
#include "iRobotPlayerController.h"
#include "DrawDebugHelpers.h"


AiRobotCharacter::AiRobotCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	GetMesh()->SetReceivesDecals(false);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
}


void AiRobotCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AiRobotCharacter, Health);
	DOREPLIFETIME(AiRobotCharacter, LastHitInfo);
}


void AiRobotCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		SpawnDefaultInventory();

		// Set initial values
		Health = DefaultHealth;
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


FTransform AiRobotCharacter::GetCameraTransform() const
{
	return GetCapsuleComponent()->GetComponentTransform();
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


float AiRobotCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	/*AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
	if (MyPC && MyPC->HasGodMode())
	{
		return 0.f;
	}*/

	if (Health <= 0.f)
	{
		return 0.f;
	}

	// Modify based on game rules.
	//AShooterGameMode* const Game = GetWorld()->GetAuthGameMode<AShooterGameMode>();
	//Damage = Game ? Game->ModifyDamage(Damage, this, DamageEvent, EventInstigator, DamageCauser) : 0.f;

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		Health -= ActualDamage;
		if (Health <= 0)
		{
			Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
		}
		else
		{
			PlayHit(ActualDamage, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
		}

		//MakeNoise(1.0f, EventInstigator ? EventInstigator->GetPawn() : this);
	}

	return ActualDamage;
}


void AiRobotCharacter::PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	// On server, we should replicate this hit to clients
	if (GetLocalRole() == ROLE_Authority)
		ReplicateHit(DamageTaken, DamageEvent, PawnInstigator, DamageCauser, false);

	if (DamageTaken > 0.f)
		ApplyDamageMomentum(DamageTaken, DamageEvent, PawnInstigator, DamageCauser);


	// FORCE FEEDBACK STUFF

	/*if (GetNetMode() != NM_DedicatedServer)
	{
		// play the force feedback effect on the client player controller
		AShooterPlayerController* PC = Cast<AShooterPlayerController>(Controller);
		if (PC && DamageEvent.DamageTypeClass)
		{
			UShooterDamageType* DamageType = Cast<UShooterDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
			if (DamageType && DamageType->HitForceFeedback && PC->IsVibrationEnabled())
			{
				FForceFeedbackParameters FFParams;
				FFParams.Tag = "Damage";
				PC->ClientPlayForceFeedback(DamageType->HitForceFeedback, FFParams);
			}
		}
	}*/


	// HUD STUFF

	/*AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
	AShooterHUD* MyHUD = MyPC ? Cast<AShooterHUD>(MyPC->GetHUD()) : NULL;
	if (MyHUD)
	{
		MyHUD->NotifyWeaponHit(DamageTaken, DamageEvent, PawnInstigator);
	}

	if (PawnInstigator && PawnInstigator != this && PawnInstigator->IsLocallyControlled())
	{
		AShooterPlayerController* InstigatorPC = Cast<AShooterPlayerController>(PawnInstigator->Controller);
		AShooterHUD* InstigatorHUD = InstigatorPC ? Cast<AShooterHUD>(InstigatorPC->GetHUD()) : NULL;
		if (InstigatorHUD)
		{
			InstigatorHUD->NotifyEnemyHit();
		}
	}*/
}


void AiRobotCharacter::ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser, bool bKilled)
{
	//const float TimeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

	FDamageEvent const& LastDamageEvent = LastHitInfo.GetDamageEvent();
	if ((PawnInstigator == LastHitInfo.PawnInstigator.Get()) && (LastDamageEvent.DamageTypeClass == LastHitInfo.DamageTypeClass) /*&& (LastTakeHitTimeTimeout == TimeoutTime)*/)
	{
		// Same frame damage
		if (bKilled && LastHitInfo.bKilled)
		{
			// Redundant death take hit, just ignore it
			return;
		}

		// otherwise, accumulate damage done this frame
		Damage += LastHitInfo.ActualDamage;
	}

	LastHitInfo.ActualDamage = Damage;
	LastHitInfo.PawnInstigator = Cast<AiRobotCharacter>(PawnInstigator);
	LastHitInfo.DamageCauser = DamageCauser;
	LastHitInfo.SetDamageEvent(DamageEvent);
	LastHitInfo.bKilled = bKilled;
	LastHitInfo.EnsureReplication();

	//LastTakeHitTimeTimeout = TimeoutTime;
}


void AiRobotCharacter::OnRep_LastHitInfo()
{
	if (LastHitInfo.bKilled)
	{
		OnDeath(LastHitInfo.ActualDamage, LastHitInfo.GetDamageEvent(), LastHitInfo.PawnInstigator.Get(), LastHitInfo.DamageCauser.Get());
	}
	else
	{
		PlayHit(LastHitInfo.ActualDamage, LastHitInfo.GetDamageEvent(), LastHitInfo.PawnInstigator.Get(), LastHitInfo.DamageCauser.Get());
	}
}


void AiRobotCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, APawn* InstigatingPawn, AActor* DamageCauser)
{
	// Do nothing if already dying
	if (bIsDying)
		return;

	SetReplicatingMovement(false);
	//TearOff();
	bIsDying = true;

	if (GetLocalRole() == ROLE_Authority)
	{
		ReplicateHit(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser, true);

		// play the force feedback effect on the client player controller
		/*AShooterPlayerController* PC = Cast<AShooterPlayerController>(Controller);
		if (PC && DamageEvent.DamageTypeClass)
		{
			UShooterDamageType* DamageType = Cast<UShooterDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
			if (DamageType && DamageType->KilledForceFeedback && PC->IsVibrationEnabled())
			{
				FForceFeedbackParameters FFParams;
				FFParams.Tag = "Damage";
				PC->ClientPlayForceFeedback(DamageType->KilledForceFeedback, FFParams);
			}
		}*/
	}

	// Cannot use IsLocallyControlled here, because even local client's controller may be NULL here
	if (GetNetMode() != NM_DedicatedServer && DeathSound /*&& Mesh1P && Mesh1P->IsVisible()*/)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	DetachFromControllerPendingDestroy();

	/*if (LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
	{
		LowHealthWarningPlayer->Stop();
	}*/

	/*if (RunLoopAC)
	{
		RunLoopAC->Stop();
	}*/

	if (GetMesh())
	{
		static FName CollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetCollisionProfileName(CollisionProfileName);
	}
	SetActorEnableCollision(true);

	// Death anim
	/*float DeathAnimDuration = PlayAnimMontage(DeathAnim);

	// Ragdoll
	if (DeathAnimDuration > 0.f)
	{
		// Trigger ragdoll a little before the animation early so the character doesn't
		// blend back to its normal position.
		const float TriggerRagdollTime = DeathAnimDuration - 0.7f;

		// Enable blend physics so the bones are properly blending against the montage.
		GetMesh()->bBlendPhysics = true;

		// Use a local timer handle as we don't need to store it for later but we don't need to look for something to clear
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AiRobotCharacter::SetRagdollPhysics, FMath::Max(0.1f, TriggerRagdollTime), false);
	}
	else*/
	{
		SetRagdollPhysics();

		if (GetNetMode() != NM_DedicatedServer)
		{
			// Needed to get the impulse of the damage
			UDamageType const* const DamageTypeCDO = DamageEvent.DamageTypeClass
				? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>()
				: GetDefault<UDamageType>();

			// Point damage event
			if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
			{
				FPointDamageEvent* const PointDamageEvent = (FPointDamageEvent*)&DamageEvent;
				GetMesh()->AddImpulse(PointDamageEvent->ShotDirection * DamageTypeCDO->DamageImpulse, PointDamageEvent->HitInfo.BoneName);
			}
		}
	}

	// disable collisions on capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
}


bool AiRobotCharacter::CanDie() const
{
	if (bIsDying 
		|| IsValid(this)	
		|| GetLocalRole() != ROLE_Authority
		|| GetWorld()->GetAuthGameMode() == nullptr
		//|| GetWorld()->GetAuthGameMode()->GetMatchState() == MatchState::LeavingMap
		)
	{
		return false;
	}

	return true;
}


bool AiRobotCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	if (!CanDie())
		return false;

	Health = 0.f;

	// if this is an environmental death then refer to the previous killer so that they receive credit (knocked into lava pits, etc)
	//UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	//Killer = GetDamageInstigator(Killer, *DamageType);

	//AController* const KilledPlayer = (Controller != NULL) ? Controller : Cast<AController>(GetOwner());
	//GetWorld()->GetAuthGameMode<AShooterGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	//NetUpdateFrequency = GetDefault<AShooterCharacter>()->NetUpdateFrequency;
	//GetCharacterMovement()->ForceReplicationUpdate();

	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	return true;
}


void AiRobotCharacter::SetRagdollPhysics()
{
	bool bInRagdoll = false;

	if (!IsValid(this) && GetMesh() && GetMesh()->GetPhysicsAsset())
	{
		// initialize physics/etc
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;

		bInRagdoll = true;
	}

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	if (!bInRagdoll)
	{
		// hide and set short lifespan
		TurnOff();
		SetActorHiddenInGame(true);
		SetLifeSpan(1.0f);
	}
	else
	{
		SetLifeSpan(10.0f);
	}
}


void AiRobotCharacter::OnInteractButtonPressed()
{
	AiRobotPlayerController* PC = Cast<AiRobotPlayerController>(Controller);
	if (PC && PC->IsGameInputAllowed())
	{
		Interact();
	}
}


void AiRobotCharacter::Interact()
{
	FVector StartTrace = GetCameraTransform().GetLocation();
	FVector EndTrace = (GetCameraTransform().GetUnitAxis(EAxis::X) * MaxInteractionDistance) + StartTrace;

	// Interaction is done on the server
	if (GetLocalRole() < ROLE_Authority)
	{
		SERVER_StartInteraction();

#if WITH_EDITOR
		//DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, 5.f);
#endif

		return;
	}

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(InteractionTrace), true, this);

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByProfile(Hit, StartTrace, EndTrace, UCollisionProfile::BlockAll_ProfileName, TraceParams);

	if (Hit.bBlockingHit && Hit.GetActor() != nullptr)
	{
		if (IInteractable* InteractableActor = Cast<IInteractable>(Hit.GetActor()))
		{
			InteractableActor->OnInteraction(this, Hit);
		}
	}
}


bool AiRobotCharacter::SERVER_StartInteraction_Validate()
{
	return true;
}


void AiRobotCharacter::SERVER_StartInteraction_Implementation()
{
	Interact();
}


bool AiRobotCharacter::HasInteractionCabability(EInteractionCapability InCapability)
{
	return InteractionCapabilities & (int32)InCapability;
}


void AiRobotCharacter::SetInteractionCapability(EInteractionCapability InCapability)
{
	InteractionCapabilities |= InCapability;
}


void AiRobotCharacter::RemoveInteractionCapability(EInteractionCapability InCapability)
{
	InteractionCapabilities &= ~InCapability;
}