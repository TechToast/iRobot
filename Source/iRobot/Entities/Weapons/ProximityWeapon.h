#pragma once

#include "Weapon.h"
#include "ProximityWeapon.generated.h"


UCLASS()
class AProximityWeapon : public AWeapon
{
	GENERATED_UCLASS_BODY()

	FORCEINLINE float GetPulseFrequency() const { return ProximityWeaponData.PulseFrequency; }
	FORCEINLINE float GetPulseRadius() const { return ProximityWeaponData.PulseRadius; }

protected:
	virtual void FireWeapon() override;
	//virtual void OnEquip();
	virtual void OnEquipFinished() override;
	virtual void OnUnEquip() override;

	/// Weapon data
	UPROPERTY(EditDefaultsOnly, Category=Config)
	FProximityWeaponData ProximityWeaponData;

	/// Spawn projectile on server
	//UFUNCTION(Reliable, Server, WithValidation)
	//void SERVER_FireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir);


};
