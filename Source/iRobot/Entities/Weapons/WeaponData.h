#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
#include "WeaponData.generated.h"

class UAnimMontage;


UENUM()
enum class EWeaponState
{
	Idle,
	Firing,
	Reloading,
	Equipping,

	MAX
};


UENUM()
enum class EAmmoType
{
	Bullet,
	Rocket,
	
	MAX
};


USTRUCT()
struct FBaseWeaponData
{
	GENERATED_USTRUCT_BODY()

public:

	/// Rate of fire (Shots per second)
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Data")
	float RateOfFire = 2;

	/// How powerful is the impulse of the "bullets" that this weapon fires?
	//UPROPERTY(EditDefaultsOnly, Category = "Weapon Data")
	//float ImpactImpulse = 1000.f;
	
	/// Type of damage
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/// Damage amount
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	int32 HitDamage;

	/// Convert the RateOfFire into an actual time value
	float GetTimeBetweenShots()
	{
		// Prevent divide by zero
		if (RateOfFire == 0)
			return 1;

		if (TimeBetweenShots == -1)
			TimeBetweenShots = 1.f / RateOfFire;

		return TimeBetweenShots;
	}

private:
	float TimeBetweenShots = -1;
};


USTRUCT()
struct FTraceWeaponData
{
	GENERATED_USTRUCT_BODY()

public:
	
	/// How far this weapon can fire traces
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Data")
	float Range = 10000.f;

	/// Base weapon spread (degrees)
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float BaseSpread = 5.f;

	/// Targeting spread modifier
	//UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	//float TargetingSpreadMod;

	/// Continuous firing: spread increment
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float FiringSpreadIncrement = 1.f;

	/// Continuous firing: max increment
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float FiringSpreadMax = 10.f;

	/// Hit verification: scale for bounding box of hit actor
	UPROPERTY(EditDefaultsOnly, Category = HitVerification)
	float ClientSideHitLeeway = 200.f;

	/// Hit verification: threshold for dot product between view direction and hit direction
	/// 1 = perfect alignment
	/// 0 = 90 degrees off (I think)
	UPROPERTY(EditDefaultsOnly, Category = HitVerification)
	float AllowedViewDotHitDir = 0.8f;
};


USTRUCT()
struct FProjectileData
{
	GENERATED_USTRUCT_BODY()

	/// Life time
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	float ProjectileLife = 10.f;

	/// Damage at impact point
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	int32 ExplosionDamage = 100.f;

	/// Radius of damage
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	float ExplosionRadius = 300.f;

	/// Type of damage
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	TSubclassOf<UDamageType> DamageType = UDamageType::StaticClass();
};


USTRUCT()
struct FProximityWeaponData
{
	GENERATED_USTRUCT_BODY()

	/// Range of the actual scan weapon
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float WeaponRange = 100.f;

	/// Radius of the scan weapon sweep
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float WeaponSweepRadius = 30.f;

	/// Duration of each proximity pulse
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float PulseDuration = 0.5f;

	/// Duration of the visible sweep during a proximity pulse ( Must always be less than or equal to PulseDuration)
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float PulseSweepDuration = 0.2f;

	/// How long between proximity pulses
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float TimeBetweenPulses = 2.f;

	/// Pulse Radius
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	float PulseRadius = 300.f;

	/// Type of damage
	//UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	//TSubclassOf<UDamageType> DamageType = UDamageType::StaticClass();
};


USTRUCT()
struct FWeaponAnim
{
	GENERATED_USTRUCT_BODY()

	/// animation played on pawn (1st person view)
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* Pawn1P;

	/// animation played on pawn (3rd person view)
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* Pawn3P;
};


USTRUCT()
struct FInstantHitInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Origin;

	UPROPERTY()
	float ReticleSpread;

	UPROPERTY()
	int32 RandomSeed;
};


/*USTRUCT(BlueprintType)
struct FScannerDamageEvent : public FDamageEvent
{
	GENERATED_BODY()

	/// Describes the trace/location that caused this damage
	UPROPERTY()
	FHitResult HitInfo;

	FScannerDamageEvent() {}
	FScannerDamageEvent(FHitResult const& InHitInfo, TSubclassOf<UDamageType> InDamageTypeClass)
		: FDamageEvent(InDamageTypeClass)
		, HitInfo(InHitInfo)
	{}

	/// ID for this class. NOTE this must be unique for all damage events.
	static const int32 ClassID = 16;

	virtual int32 GetTypeID() const override 
	{ 
		return FScannerDamageEvent::ClassID; 
	}

	virtual bool IsOfType(int32 InID) const override 
	{ 
		return (FScannerDamageEvent::ClassID == InID) || FDamageEvent::IsOfType(InID); 
	}

	/// Not sure we need this
	//virtual void GetBestHitInfo(AActor const* HitActor, AActor const* HitInstigator, struct FHitResult& OutHitInfo, FVector& OutImpulseDir) const override {}
};*/