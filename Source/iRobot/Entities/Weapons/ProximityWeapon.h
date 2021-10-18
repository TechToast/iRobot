#pragma once

#include "Weapon.h"
#include "Interfaces/IScannable.h"
#include "ProximityWeapon.generated.h"


class UMaterialInstanceDynamic;


UCLASS()
class AProximityWeapon : public AWeapon
{
	GENERATED_UCLASS_BODY()

	FORCEINLINE float GetWeaponRange() const		{ return ProximityWeaponData.WeaponRange; }
	FORCEINLINE float GetWeaponSweepRadius() const  { return ProximityWeaponData.WeaponSweepRadius; }
	FORCEINLINE float GetPulseDuration() const		{ return ProximityWeaponData.PulseDuration; }
	FORCEINLINE float GetPulseSweepDuration() const { return FMath::Min(ProximityWeaponData.PulseSweepDuration, ProximityWeaponData.PulseDuration); }
	FORCEINLINE float GetTimeBetweenPulses() const	{ return ProximityWeaponData.TimeBetweenPulses; }
	FORCEINLINE float GetPulseTotalTime() const		{ return GetPulseDuration() + GetTimeBetweenPulses(); }
	FORCEINLINE float GetPulseRadius() const		{ return ProximityWeaponData.PulseRadius; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void FireWeapon() override;
	virtual void OnBurstFinished() override;
	virtual void OnEquipFinished() override;
	virtual void OnUnEquip() override;

	/// Weapon data
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Config")
	FProximityWeaponData ProximityWeaponData;

	/// Target actor types that the weapon should affect
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	TArray<TSoftClassPtr<AActor>> WeaponTargetTypes;

	/// Target actor type that the pulse scan should detect
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	TSoftClassPtr<AActor> PulseTargetType;

	/// Material ID of the scanner material on the 1P mesh
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	int32 ScannerMaterialIDMesh1P = 0;

	/// Material ID of the scanner material on the 3P mesh
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	int32 ScannerMaterialIDMesh3P = 0;

	/// Name of the PulseSweepDuration parameter on the scanner material
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	FName PulseSweepParameterName = TEXT("PulseSweepDuration");

	/// Name of the PulseDuration parameter on the scanner material
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	FName PulseDurationParameterName = TEXT("PulseDuration");

	/// Name of the Rotation parameter on the scanner material
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	FName RotationParameterName = TEXT("Rotation");

	/// Render target to draw proximity items into
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	UTextureRenderTarget2D* ProximityTexture;

	/// The material used to draw dots on the proximity texture
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	UMaterialInterface* ProximityDotBrushMaterial;

	/// Ratio of render texture size to use as the dot size
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	float DotSizeRatio = 0.01f;

	/// Should the scanner only detect targets that are in motion
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	bool bDetectOnlyMotion = false;

	/// The minimum speed that the scanner will detect
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner", meta=(EditCondition="bDetectOnlyMotion"))
	float MinSpeedThreshold = 5.f;

private:

	/// Process the sweep hit and notify the server if necessary
	void ProcessSweepHit(const FHitResult& Impact);

	/// Continue processing the hit, as if it has been confirmed by the server
	void ProcessSweepHit_Confirmed(const FHitResult& Impact);

	void DealDamage(const FHitResult& Impact);
	
	/// Reset the scanner hit data on server
	UFUNCTION(Server, Reliable)
	void SERVER_ResetScannerHit();

	/// RPC to the server to notify of hit from client to verify
	UFUNCTION(Reliable, Server, WithValidation)
	void SERVER_NotifyHit(const FHitResult& Impact);

	void PerformProximityCheck();
	void BeginPulse();
	void UpdatePulse();
	void UpdateProximityTexture();
	void UpdateRotation();
	void GatherPulseTargets();

	float TimeSinceLastPulse = FLT_MAX;
	float PulseRadiusSquared;
	float MinSpeedThresholdSquared;

	TWeakObjectPtr<UMaterialInstanceDynamic> ScannerMaterial;
	TArray<TWeakObjectPtr<AActor>> PulseTargets;
	TArray<FVector2D> PulseTargetLocations;

	/// The last actor that was hit by the scanner weapon
	TScriptInterface<IScannable> LastWeaponHitActor = nullptr;
	int32 LastWeaponHitIndex = -1;
};
