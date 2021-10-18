#include "ProximityWeapon.h"
#include "iRobot.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "CollisionDebugDrawingPublic.h"


AProximityWeapon::AProximityWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = false;
}


void AProximityWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (Mesh1P)
		ScannerMaterial = Mesh1P->CreateDynamicMaterialInstance(ScannerMaterialIDMesh1P);
	
	if (Mesh3P)
	{
		if (ScannerMaterial.IsValid())
			Mesh3P->SetMaterial(ScannerMaterialIDMesh3P, ScannerMaterial.Get());
		else
			ScannerMaterial = Mesh3P->CreateDynamicMaterialInstance(ScannerMaterialIDMesh3P);
	}

	PulseRadiusSquared = GetPulseRadius() * GetPulseRadius();
	MinSpeedThresholdSquared = MinSpeedThreshold * MinSpeedThreshold;
}


void AProximityWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsEquipped() || GetNetMode() == NM_DedicatedServer)
	{
		SetActorTickEnabled(false);
		return;
	}

	TimeSinceLastPulse += DeltaSeconds;

	if (TimeSinceLastPulse >= GetPulseTotalTime())
	{
		TimeSinceLastPulse = 0;
		BeginPulse();
	}

	UpdatePulse();
	UpdateRotation();
}


void AProximityWeapon::FireWeapon()
{
	const FVector AimDir = GetCameraAim();
	const FVector StartTrace = GetCameraTraceStartLocation(AimDir);
	const FVector EndTrace = StartTrace + (AimDir * GetWeaponRange());

	// Perform the weapon sweep
	const FCollisionShape ScannerShape = FCollisionShape::MakeSphere(GetWeaponSweepRadius());
	const FHitResult Impact = WeaponSweep(StartTrace, EndTrace, ScannerShape);

	// Process any hit
	ProcessSweepHit(Impact);

	//DrawSphereSweeps(GetWorld(), StartTrace, EndTrace, GetWeaponSweepRadius(), TArray< FHitResult> {Impact}, 30.f);
}


void AProximityWeapon::OnBurstFinished()
{
	Super::OnBurstFinished();

	SERVER_ResetScannerHit();
}


void AProximityWeapon::ProcessSweepHit(const FHitResult& Impact)
{
	// Client (Dedicated server)
	if (GetOwningPawn().IsValid() && GetOwningPawn()->IsLocallyControlled() && GetNetMode() == NM_Client)
	{
		// Only interested in scannable targets
		if (IScannable* ScannableTarget = Cast<IScannable>(Impact.GetActor()))
		{
			// if we're a client and we've hit something that is being controlled by the server
			if (Impact.GetActor() && Impact.GetActor()->GetRemoteRole() == ROLE_Authority)
			{
				// notify the server of the hit
				SERVER_NotifyHit(Impact);
			}
		}
		else
		{
			SERVER_ResetScannerHit();
		}
	}

	// process a confirmed hit
	ProcessSweepHit_Confirmed(Impact);
}


bool AProximityWeapon::SERVER_NotifyHit_Validate(const FHitResult& Impact)
{
	return true;
}


void AProximityWeapon::SERVER_NotifyHit_Implementation(const FHitResult& Impact)
{
	ProcessSweepHit_Confirmed(Impact);
}

void AProximityWeapon::ProcessSweepHit_Confirmed(const FHitResult& Impact)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		// Did we hit a valid target?
		if (IScannable* ScannableTarget = Cast<IScannable>(Impact.GetActor()))
		{
			int32 ScanIndex = ScannableTarget->GetScanIndex(Impact.ImpactPoint);

			// Already pre-scanned this target
			if (LastWeaponHitActor == ScannableTarget && ScanIndex == LastWeaponHitIndex)
			{
				ScannableTarget->OnScanned(ScanIndex);
				SERVER_ResetScannerHit();
			}
			else
			{
				LastWeaponHitActor.SetInterface(ScannableTarget);
				LastWeaponHitActor.SetObject(Impact.GetActor());
				LastWeaponHitIndex = ScanIndex;
			}
		}
		else
		{
			SERVER_ResetScannerHit();
		}
	}
}


void AProximityWeapon::DealDamage(const FHitResult& Impact)
{

}


void AProximityWeapon::OnEquipFinished()
{
	Super::OnEquipFinished();

	// Only start pulsing on the owning client's machine
	if (GetOwningPawn().IsValid() && GetOwningPawn()->IsLocallyControlled() && !GetOwningPawn()->IsBotControlled())
	{
		// Start pulsing
		if (GetNetMode() != NM_DedicatedServer)
			SetActorTickEnabled(true);
	}
}


void AProximityWeapon::OnUnEquip()
{
	Super::OnUnEquip();

	// Stop pulsing
	SetActorTickEnabled(false);
	
	// Clear data
	PulseTargets.Empty();
	PulseTargetLocations.Empty();
	UpdateProximityTexture();

}


void AProximityWeapon::UpdateRotation()
{
	if (GetOwningPawn().IsValid() && RotationParameterName != NAME_None && ScannerMaterial.IsValid())
	{
		FRotator Rotation = GetOwningPawn()->GetActorRotation();
		ScannerMaterial->SetScalarParameterValue(RotationParameterName, (Rotation.Yaw + 180.f) / 360.f);
	}
}


void AProximityWeapon::BeginPulse()
{
	// Don't do this on dedicated server
	if (GetNetMode() == NM_DedicatedServer)
		return;

	PerformProximityCheck();
}


void AProximityWeapon::UpdatePulse()
{
	if (ScannerMaterial.IsValid())
	{
		if (PulseSweepParameterName != NAME_None)
			ScannerMaterial->SetScalarParameterValue(PulseSweepParameterName, FMath::Clamp(TimeSinceLastPulse / GetPulseSweepDuration(), 0.f, 1.f));
		if (PulseDurationParameterName != NAME_None)
			ScannerMaterial->SetScalarParameterValue(PulseDurationParameterName, FMath::Clamp(TimeSinceLastPulse / GetPulseDuration(), 0.f, 1.f));
	}
}


void AProximityWeapon::PerformProximityCheck()
{
	if (PulseTargets.Num() == 0)
		GatherPulseTargets();

	PulseTargetLocations.Empty();

	if (GetOwningPawn().IsValid())
	{
		for (TWeakObjectPtr<AActor>& Target : PulseTargets)
		{
			if (Target.IsValid())
			{
				const FVector& PlayerPosition = GetOwningPawn()->GetActorLocation();
				const FVector& TargetPosition = Target->GetActorLocation();
				const FVector Diff = PlayerPosition - TargetPosition;

				// Within Range, draw a dot
				if (Diff.SizeSquared() < PulseRadiusSquared)
				{
					const FVector Location = (PlayerPosition - TargetPosition) / GetPulseRadius();

					if (bDetectOnlyMotion && Target->GetVelocity().SizeSquared() < MinSpeedThresholdSquared)
						continue;

					PulseTargetLocations.Add(FVector2D(Location.Y + 0.5f, (-Location.X + 0.5f)));
				}
			}
		}

		UpdateProximityTexture();
	}
}


void AProximityWeapon::UpdateProximityTexture()
{
	if (IsPendingKillPending())
		return;

	if (!ProximityTexture || !ProximityTexture->IsValidLowLevel())
	{
		UE_LOG(LogiRobotWeapon, Error, TEXT("AProximityWeapon::UpdateProximityTexture - ProximityTexture is null or invalid on %s"), *GetName());
		return;
	}

	if (!ProximityDotBrushMaterial || !ProximityDotBrushMaterial->IsValidLowLevel())
	{
		UE_LOG(LogiRobotWeapon, Error, TEXT("AProximityWeapon::UpdateProximityTexture - ProximityDotBrushMaterial is null or invalid on %s"), *GetName());
		return;
	}

	UKismetRenderingLibrary::ClearRenderTarget2D(this, ProximityTexture, FLinearColor::Black);

	UCanvas* Canvas = nullptr;
	FDrawToRenderTargetContext Context;
	FVector2D Size(ProximityTexture->SizeX, ProximityTexture->SizeY);
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, ProximityTexture, Canvas, Size, Context);

	for (int32 Index = 0; Index < PulseTargetLocations.Num(); Index++)
	{
		const FVector2D& ThePosition = PulseTargetLocations[Index];//FVector2D(FMath::RandRange(0.f, 1.f), FMath::RandRange(0.f, 1.f));

		FVector2D ScreenSize = Size * DotSizeRatio;
		FVector2D ScreenPosition = (ThePosition * Size) - (ScreenSize * 0.5f);
		FVector2D CoordinatePosition = FVector2D::ZeroVector;
		FVector2D CoordinateSize = FVector2D::UnitVector;

		Canvas->K2_DrawMaterial(ProximityDotBrushMaterial, ScreenPosition, ScreenSize, CoordinatePosition);
	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
}


void AProximityWeapon::GatherPulseTargets()
{
	PulseTargets.Empty();
	if (!PulseTargetType.IsNull())
	{
		for (TActorIterator<AActor> TargetIter(GetWorld(), PulseTargetType.LoadSynchronous()); TargetIter; ++TargetIter)
		{
			if (*TargetIter)
			{
				PulseTargets.Add(*TargetIter);
			}
		}
	}
}


void AProximityWeapon::SERVER_ResetScannerHit_Implementation()
{
	LastWeaponHitActor = nullptr;
	LastWeaponHitIndex = -1;
}