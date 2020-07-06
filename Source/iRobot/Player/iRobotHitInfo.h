#pragma once

//#include "CoreMinimal.h"
#include "EngineMinimal.h"
#include "iRobotHitInfo.generated.h"

class AiRobotCharacter;

/// Replicated information on a hit we've taken
USTRUCT()
struct FHitInfo
{
	GENERATED_USTRUCT_BODY()

public:
	FHitInfo() {}

	FDamageEvent& GetDamageEvent();
	void SetDamageEvent(const FDamageEvent& DamageEvent);
	void EnsureReplication();

	/// The amount of damage actually applied
	UPROPERTY()
	float ActualDamage = 0;

	/// The player that insitigated the hit
	UPROPERTY()
	TWeakObjectPtr<AiRobotCharacter> PawnInstigator;

	/// The actor that actually caused the damage
	UPROPERTY()
	TWeakObjectPtr<class AActor> DamageCauser;

	/// The damage type we were hit with.
	UPROPERTY()
	UClass* DamageTypeClass;

	/// Was this a killshot?
	UPROPERTY()
	bool bKilled = false;

private:

	/// A rolling counter used to ensure the struct is dirty and will replicate.
	UPROPERTY()
	uint8 EnsureReplicationByte = 0;

	/// Specifies which DamageEvent below describes the damage received.
	UPROPERTY()
	int32 DamageEventClassID;

	/// Describes general damage.
	UPROPERTY()
	FDamageEvent GeneralDamageEvent;

	/// Describes point damage, if that is what was received.
	UPROPERTY()
	FPointDamageEvent PointDamageEvent;

	/// Describes radial damage, if that is what was received.
	UPROPERTY()
	FRadialDamageEvent RadialDamageEvent;
};