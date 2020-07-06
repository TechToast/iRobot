// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/iRobotCharacter.h"
#include "Entities/Weapons/WeaponData.h"
#include "HunterCharacter.generated.h"

class UInputComponent;
class USceneComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class AWeapon;
class UAnimMontage;
class USoundBase;


UCLASS(config=Game)
class IROBOT_API AHunterCharacter : public AiRobotCharacter
{
	GENERATED_BODY()

public:
	AHunterCharacter();

	/// Public Getters
	FORCEINLINE USkeletalMeshComponent* GetFPMesh() const		{ return FPMesh; }
	FORCEINLINE UCameraComponent* GetFPCameraComponent() const	{ return FPCameraComponent; }
	
	FORCEINLINE FName GetWeaponAttachPoint() const				{ return WeaponAttachPoint; }
	FORCEINLINE int32 GetInventoryCount() const					{ return Inventory.Num(); }
	USkeletalMeshComponent* GetPawnMesh() const;

	bool IsFirstPerson() const;
	bool CanFire() const;

	/// Get the aim offsets for animation blueprint
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	FRotator GetAimOffsets() const;

	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	virtual void SpawnDefaultInventory() override;
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, APawn* InstigatingPawn, AActor* DamageCauser) override;

	/// Input
	void OnFireButtonHeld();
	void OnFireButtonReleased();

	/// Inventory
	void AddWeapon(AWeapon* Weapon);
	void RemoveWeapon(AWeapon* Weapon);
	void EquipWeapon(AWeapon* Weapon);
	void OnNextWeapon();
	void OnPrevWeapon();

	/// RPC to server to equip the weapon
	UFUNCTION(Reliable, Server, WithValidation)
	void SERVER_EquipWeapon(AWeapon* NewWeapon);

	/// socket or bone name for attaching weapon mesh
	UPROPERTY(EditDefaultsOnly, Category = "Hunter")
	FName WeaponAttachPoint;

	/// Default inventory list
	UPROPERTY(EditDefaultsOnly, Category = "Hunter")
	TArray<TSoftClassPtr<AWeapon>> DefaultInventoryClasses;

	/// weapons in inventory
	UPROPERTY(Transient, Replicated)
	TArray<AWeapon*> Inventory;

protected:

	/// Pawn mesh: 1st person view (arms; seen only by self)
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FPMesh;

	/// First person camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FPCameraComponent;

private:

	/// Stop playing all montages
	void StopAllAnimMontages();

	/// Set the given weapon as the current weapon
	void SetCurrentWeapon(AWeapon* NewWeapon);

	/// Currently equipped weapon
	UPROPERTY(Transient, ReplicatedUsing=OnRep_CurrentWeapon)
	AWeapon* CurrentWeapon;

	/// Called when CurrentWeapon is replicated
	UFUNCTION()
	void OnRep_CurrentWeapon();

	bool bWantsToFire = false;
};

