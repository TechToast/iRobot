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