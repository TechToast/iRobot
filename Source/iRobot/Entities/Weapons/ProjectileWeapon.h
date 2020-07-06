#pragma once

#include "Weapon.h"
#include "ProjectileWeapon.generated.h"


UCLASS()
class AProjectileWeapon : public AWeapon
{
	GENERATED_UCLASS_BODY()

	//FORCEINLINE float GetProjectileLife() const { return ProjectileWeaponData.ProjectileLife; }
	//FORCEINLINE float GetExplosionDamage() const { return ProjectileWeaponData.ExplosionDamage; }
	//FORCEINLINE float GetExplosionRadius() const { return ProjectileWeaponData.ExplosionRadius; }

	/// Apply config on projectile
	//void ApplyWeaponConfig(FProjectileWeaponData& Data);

	/// Projectile class
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AProjectile> ProjectileClass = nullptr;

protected:
	virtual void FireWeapon() override;

	/// Weapon data
	//UPROPERTY(EditDefaultsOnly, Category=Config)
	//FProjectileWeaponData ProjectileWeaponData;

	/// Spawn projectile on server
	UFUNCTION(Reliable, Server, WithValidation)
	void SERVER_FireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir);
};
