#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
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
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Data")
	float ImpactImpulse = 1000.f;
	
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


USTRUCT()
struct FInstantWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** base weapon spread (degrees) */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float WeaponSpread;

	/** targeting spread modifier */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float TargetingSpreadMod;

	/** continuous firing: spread increment */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float FiringSpreadIncrement;

	/** continuous firing: max increment */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float FiringSpreadMax;

	/** weapon range */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	float WeaponRange;

	/** damage amount */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	int32 HitDamage;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/** hit verification: scale for bounding box of hit actor */
	UPROPERTY(EditDefaultsOnly, Category=HitVerification)
	float ClientSideHitLeeway;

	/** hit verification: threshold for dot product between view direction and hit direction */
	UPROPERTY(EditDefaultsOnly, Category=HitVerification)
	float AllowedViewDotHitDir;

	/** defaults */
	FInstantWeaponData()
	{
		WeaponSpread = 5.0f;
		TargetingSpreadMod = 0.25f;
		FiringSpreadIncrement = 1.0f;
		FiringSpreadMax = 10.0f;
		WeaponRange = 10000.0f;
		HitDamage = 10;
		DamageType = UDamageType::StaticClass();
		ClientSideHitLeeway = 200.0f;
		AllowedViewDotHitDir = 0.8f;
	}
};