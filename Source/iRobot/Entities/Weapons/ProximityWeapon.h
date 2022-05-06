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
	FORCEINLINE float GetPulseDuration() const		{ return PulseDuration; }
	FORCEINLINE float GetPulseSweepDuration() const { return FMath::Min(PulseSweepDuration, PulseDuration); }
	FORCEINLINE float GetTimeBetweenPulses() const	{ return TimeBetweenPulses; }
	FORCEINLINE float GetPulseTotalTime() const		{ return GetPulseDuration() + GetTimeBetweenPulses(); }
	FORCEINLINE float GetPulseRadius() const		{ return PulseRadius; }

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

	/// The name of the intensity parameter on the dot brush material
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	FName IntensityParamName = TEXT("Intensity");

	/// Duration of each proximity pulse
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	float PulseDuration = 0.5f;

	/// Duration of the visible sweep during a proximity pulse ( Must always be less than or equal to PulseDuration)
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	float PulseSweepDuration = 0.2f;

	/// How long between proximity pulses
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	float TimeBetweenPulses = 2.f;

	/// Pulse Radius
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	float PulseRadius = 300.f;

	/// The speed at which the scanner detects the target at minimum intensity
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	float MinSpeedThreshold = 5.f;

	/// Should the image be faded based on the speed of the target
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	bool bFadeBasedOnSpeed = true;

	/// The speed at which the scanner detects the target at full intensity
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner", meta = (EditCondition = "bFadeBasedOnSpeed"))
	float MaxSpeedThreshold = 300.f;

	/// The minimum intensity when the target speed is at or below the MinSpeedThreshold 
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner", meta = (EditCondition = "bFadeBasedOnSpeed"))
	float MinSpeedIntensity = 0.25f;

	/// Should the image be faded based on the distance of the target
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	bool bFadeBasedOnDistance = true;

	/// The distance at which the scanner starts to fade the intensity (fully faded at PulseRadius)
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner", meta = (EditCondition = "bFadeBasedOnDistance"))
	float MinDistanceThreshold = 5.f;

	/// The minimum intensity when the target distance is at or below the MinDistanceThreshold 
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner", meta = (EditCondition = "bFadeBasedOnDistance"))
	float MinDistanceIntensity = 0.25f;

	/// Ratio of render texture size to use as the dot size
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	float DotSizeRatio = 0.01f;

	/// Offset the dot from the centre of the texture by a certain ratio
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Scanner")
	float CentreOffsetRatio = 0.05f;

	/// Pulse sound
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sound")
	USoundCue* PulseSound;

	/// Blip sound
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sound")
	USoundCue* BlipSound;

	/// Volume of the blip at minimum intensity
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sound")
	float MinBlipVolume = 0.25f;

private:

	/// Process the sweep hit and notify the server if necessary
	void ProcessSweepHit(const FHitResult& Impact);

	/// Continue processing the hit, as if it has been confirmed by the server
	void ProcessSweepHit_Confirmed(const FHitResult& Impact);

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
	float MinSpeedThresholdSquared;
	float MaxSpeedThresholdSquared;

	TWeakObjectPtr<UMaterialInstanceDynamic> ScannerMaterial;
	TArray<TWeakObjectPtr<AActor>> PulseTargets;
	TArray<FProximityTargetLocation> PulseTargetLocations;

	/// The last actor that was hit by the scanner weapon
	TScriptInterface<IScannable> LastWeaponHitActor = nullptr;
	int32 LastWeaponHitIndex = -1;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* DynamicProximityDotBrushMaterial;
};
