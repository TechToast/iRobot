#pragma once

#include "Weapon.h"
#include "WeaponData.h"
#include "Effects/ImpactEffect.h"
#include "TraceWeapon.generated.h"

//class AShooterImpactEffect;


UCLASS(Blueprintable)
class ATraceWeapon : public AWeapon
{
	GENERATED_UCLASS_BODY()

public:

	FORCEINLINE float GetRange() const					{ return TraceWeaponData.Range; }
	FORCEINLINE float GetBaseSpread() const				{ return TraceWeaponData.BaseSpread; }
	FORCEINLINE float GetFiringSpreadIncrement() const	{ return TraceWeaponData.FiringSpreadIncrement; }
	FORCEINLINE float GetFiringSpreadMax() const		{ return TraceWeaponData.FiringSpreadMax; }
	FORCEINLINE float GetClientSideHitLeeway() const	{ return TraceWeaponData.ClientSideHitLeeway; }
	FORCEINLINE float GetAllowedViewDotHitDir() const	{ return TraceWeaponData.AllowedViewDotHitDir; }

	/// Get current spread
	float GetCurrentSpread() const;

protected:
	virtual void FireWeapon() override;
	virtual void OnBurstFinished() override;

	/// Data about this trace weapon
	UPROPERTY(EditAnywhere, Category = "Weapon|Config")
	FTraceWeaponData TraceWeaponData;

	/// Impact effects
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	TSubclassOf<AImpactEffect> ImpactTemplate;

	/// Smoke trail
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	UParticleSystem* TrailFX;

	/// Param name for beam target in smoke trail
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	FName TrailTargetParam;

	/// instant hit notify for replication
	UPROPERTY(Transient, ReplicatedUsing=OnRep_HitNotify)
	FInstantHitInfo HitNotify;

	/// RPC to the server to notify of hit from client to verify
	UFUNCTION(Reliable, Server, WithValidation)
	void SERVER_NotifyHit(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	/// RPC to the server to notify of miss and show trail FX
	UFUNCTION(Unreliable, Server, WithValidation)
	void SERVER_NotifyMiss(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	/// Continue processing the instant hit, as if it has been confirmed by the server
	void ProcessTraceHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	UFUNCTION()
	void OnRep_HitNotify();

	/// Called in network play to do the cosmetic fx 
	void SimulateTraceHit(const FVector& Origin, int32 RandomSeed, float ReticleSpread);

	/// Spawn effects for impact
	void SpawnImpactEffects(const FHitResult& Impact);

	/// Spawn trail effect
	void SpawnTrailEffect(const FVector& EndPoint);

private:

	/// Process the trace hit and notify the server if necessary
	void ProcessTraceHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	/// Deal damage
	void DealDamage(const FHitResult& Impact, const FVector& ShootDir);

	float CurrentFiringSpread;
};
