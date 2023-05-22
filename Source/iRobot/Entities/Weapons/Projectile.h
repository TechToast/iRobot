#pragma once

#include "GameFramework/Actor.h"
#include "WeaponData.h"
#include "ProjectileWeapon.h"
#include "Effects/ExplosionEffect.h"
#include "Projectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

// 
UCLASS(Blueprintable)
class AProjectile : public AActor
{
	GENERATED_UCLASS_BODY()

public:

	FORCEINLINE float GetProjectileLife() const							{ return ProjectileData.ProjectileLife; }
	FORCEINLINE float GetExplosionDamage() const						{ return ProjectileData.ExplosionDamage; }
	FORCEINLINE float GetExplosionRadius() const						{ return ProjectileData.ExplosionRadius; }
	FORCEINLINE TSubclassOf<UDamageType> GetDamageType() const			{ return ProjectileData.DamageType; }

	FORCEINLINE UProjectileMovementComponent* GetMovementComp() const	{ return MovementComp; }
	FORCEINLINE USphereComponent* GetCollisionComp() const				{ return CollisionComp; }
	FORCEINLINE UParticleSystemComponent* GetParticleComp() const		{ return ParticleComp; }

	/// Effects for explosion
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<class AExplosionEffect> ExplosionTemplate;

	/// Weapon data
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FProjectileData ProjectileData;

	/// Setup velocity
	void InitVelocity(FVector& ShootDirection);

protected:
	virtual void PostInitializeComponents() override;
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

protected:

	/// Movement component
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	UProjectileMovementComponent* MovementComp;

	/// Collisions
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* CollisionComp;

	/// Particle effects
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	UParticleSystemComponent* ParticleComp;

	UFUNCTION(BlueprintImplementableEvent)
	void OnExplodedBP();

private:

	/// Handle hit
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);

	/// Trigger explosion
	void Explode(const FHitResult& Impact);

	/// Shutdown projectile and prepare for destruction
	void DisableAndDestroy();

	/// Did it explode?
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Exploded)
	bool bExploded;

	/// Called when bExploded is replicated
	UFUNCTION()
	void OnRep_Exploded();

	/// Controller that fired me (cache for damage calculations)
	TWeakObjectPtr<AController> MyController;

	/// Projectile data
	//struct FProjectileWeaponData WeaponConfig;
};
