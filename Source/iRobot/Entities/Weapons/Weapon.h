#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//#include "Engine/Canvas.h" // for FCanvasIcon
#include "WeaponData.h"
#include "Camera/CameraShakeBase.h"
#include "Player/Hunter/HunterCharacter.h"
#include "WeaponAnimInstance.h"
#include "Weapon.generated.h"

class UAnimMontage;
class UAudioComponent;
class UParticleSystemComponent;
class UCameraShakeBase;
//class UForceFeedbackEffect;
class USoundCue;

DECLARE_EVENT(AWeapon, FOnWeaponEquipFinished)

UCLASS(Abstract)
class AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon(const FObjectInitializer& ObjectInitializer);

	/// Weapon data getters
	FORCEINLINE float GetRateOfFire() const								{ return WeaponData.RateOfFire; }
	//FORCEINLINE float GetImpactImpulse() const						{ return WeaponData.ImpactImpulse; }
	FORCEINLINE float GetTimeBetweenShots() 							{ return WeaponData.GetTimeBetweenShots(); }
	FORCEINLINE TSubclassOf<UDamageType> GetDamageType() const			{ return WeaponData.DamageType; }
	FORCEINLINE int32 GetHitDamage() const								{ return WeaponData.HitDamage; }
	FORCEINLINE UTexture2D* GetCrosshairTexture() const					{ return CrosshairTexture; }
	
	FOnWeaponEquipFinished* GetOnWeaponEquipFinished()					{ return &OnWeaponEquipFinished; }

	/// General getters
	EWeaponState						GetCurrentState() const			{ return CurrentState; }
	TWeakObjectPtr<AHunterCharacter>	GetOwningPawn() const			{ return OwningPawn.Get(); }
	USkeletalMeshComponent*				GetWeaponMesh() const;
	FVector								GetMuzzleLocation() const;
	FVector								GetMuzzleDirection() const;

	bool IsEquipped() const												{ return bIsEquipped; }
	bool IsAttachedToPawn() const										{ return bIsEquipped/* || bPendingEquip*/; }

	/// General setters
	void SetOwningPawn(AHunterCharacter* InCharacter);

	/// Start/Stop weapon fire
	virtual void StartFire();
	virtual void StopFire();

	/// Inventory
	void OnEnterInventory(AHunterCharacter* NewOwner);
	void OnLeaveInventory();

	/// Equip/un-equip this weapon
	virtual void OnEquip();
	virtual void OnEquipFinished();
	virtual void OnUnEquip();

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	/// Data about the weapon
	UPROPERTY(EditAnywhere, Category = "Weapon|Config")
	FBaseWeaponData WeaponData;

	/// Name of bone/socket for muzzle in weapon mesh
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
	FName MuzzleAttachPoint;

	/// Is muzzle FX looped?
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
	uint32 bLoopedMuzzleFX : 1;

	/// FX for muzzle flash
	UPROPERTY(EditDefaultsOnly, Category= "Weapon|Effects")
	UParticleSystem* MuzzleFX;

	/// Camera shake on firing
	UPROPERTY(EditDefaultsOnly, Category= "Weapon|Effects")
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	/// Force feedback effect to play when the weapon is fired
	//UPROPERTY(EditDefaultsOnly, Category=Effects)
	//UForceFeedbackEffect *FireForceFeedback;

	/// Is fire sound looped?
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sound")
	bool bLoopedFireSound = true;

	/// Single fire sound
	UPROPERTY(EditDefaultsOnly, Category= "Weapon|Sound", meta=(EditCondition="!bLoopedFireSound"))
	USoundCue* FireSound;

	/// Looped fire sound
	UPROPERTY(EditDefaultsOnly, Category= "Weapon|Sound", meta = (EditCondition = "bLoopedFireSound"))
	USoundCue* FireLoopSound;

	/// Finished burst sound
	UPROPERTY(EditDefaultsOnly, Category= "Weapon|Sound", meta = (EditCondition = "bLoopedFireSound"))
	USoundCue* FireFinishSound;

	/// Equip sound
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sound")
	USoundCue* EquipSound;

	/// Fire animations
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	FWeaponAnim FireAnim;

	/// Is the fire animation looped?
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	bool bLoopedFireAnim;

	/// The crosshair texture to use for this weapon. Leave blank if none required
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Crosshair")
	UTexture2D* CrosshairTexture;

	/// Adjustment to handle frame rate affecting actual timer interval.
	UPROPERTY(Transient)
	float TimerIntervalAdjustment;

	/// Whether to allow automatic weapons to catch up with shorter refire cycles
	//UPROPERTY()
	//bool bAllowAutomaticWeaponCatchup = true;

	/// Simulate weapon fire start/stop
	virtual void SimulateWeaponFire();
	virtual void StopSimulatingWeaponFire();

	/// Weapon specific fire implementation (Local)
	virtual void FireWeapon() PURE_VIRTUAL(AWeapon::FireWeapon, );
	
	/// Firing finished
	virtual void OnBurstFinished();

	/// Play the given sound
	UAudioComponent* PlayWeaponSound(USoundCue* Sound, float VolumeMultiplier = 1.f);

	/// Trace forward from the weapon to see if we hit anything
	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;
	FHitResult WeaponSweep(const FVector& StartTrace, const FVector& EndTrace, const FCollisionShape& Shape) const;

	/// Get the aim of the camera
	FVector GetCameraAim() const;

	/// Get the originating location for camera traces
	FVector GetCameraTraceStartLocation(const FVector& AimDir) const;

	/// Delegate events
	FOnWeaponEquipFinished OnWeaponEquipFinished;

protected:

	/// Weapon mesh: 1st person view
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/// Weapon mesh: 3rd person view
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh3P;

private:

	/// Attach/Detach this weapon mesh to/from pawn's mesh
	void AttachMeshToPawn();
	void DetachMeshFromPawn();

	/// Weapon state
	void DetermineWeaponState();
	void SetWeaponState(EWeaponState NewState);

	/// Firing
	bool CanFire() const;
	void HandleReFiring();
	void HandleFiring();
	void OnBurstStarted();

	/// RPC to server to start firing the weapon
	UFUNCTION(Reliable, Server, WithValidation)
	void SERVER_StartFire();

	/// RPC to server to stop firing the weapon
	UFUNCTION(Reliable, Server, WithValidation)
	void SERVER_StopFire();

	/// RPC to server to fire the weapon
	UFUNCTION(Reliable, Server, WithValidation)
	void SERVER_HandleFiring();

	/// Play weapon animations
	float PlayWeaponAnimation(const FWeaponAnim& Animation);

	/// Stop playing weapon animations
	void StopWeaponAnimation(const FWeaponAnim& Animation);

	/// Pawn owner
	UPROPERTY(Transient, ReplicatedUsing = OnRep_OwningPawn)
	TWeakObjectPtr<AHunterCharacter> OwningPawn;

	/// Called when OwningPawn is replicated
	UFUNCTION()
	void OnRep_OwningPawn();

	/// Burst counter, used for replicating fire events to remote clients
	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;

	/// Called when BurstCounter is replicated
	UFUNCTION()
	void OnRep_BurstCounter();

	/// Spawned component for muzzle FX
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	/// Audio component reference to play local looped sounds
	UPROPERTY(Transient)
	UAudioComponent* FireAC;

	/// Hold a reference to the anim instances of this weapon
	//TWeakObjectPtr<UWeaponAnimInstance> AnimInstance1P;
	//TWeakObjectPtr<UWeaponAnimInstance> AnimInstance3P;

	FTimerHandle TimerHandle_HandleFiring;

	bool bIsEquipped;
	bool bPendingEquip;
	bool bWantsToFire;
	bool bRefiring;
	bool bPlayingFireAnim;

	EWeaponState CurrentState;

	float LastFireTime;
};

