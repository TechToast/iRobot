#pragma once

#include "HidingPlaceData.generated.h"

UENUM()
enum class EHidingPlaceType : uint8
{
	HP_None,
	HP_Freeze
};


USTRUCT()
struct FHidingPlaceTransform
{
	GENERATED_BODY()

	/// The location of the last impact
	UPROPERTY()
	FTransform Transform = FTransform::Identity;

	/// Timestamp to ensure replication occurs
	UPROPERTY()
	float TimeStamp = 0;
};