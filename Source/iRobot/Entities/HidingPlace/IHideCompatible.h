#pragma once

#include "UObject/Interface.h"
#include "HidingPlaceData.h"
#include "IHideCompatible.generated.h"


DECLARE_EVENT(UHideCompatible, FHidingPlaceReadyEvent);


///
/// IHideCompatible interface
///

UINTERFACE(MinimalAPI, Blueprintable)
class UHideCompatible : public UInterface
{
	GENERATED_BODY()
};


class IHideCompatible
{
	GENERATED_BODY()

public:

	/// Get the type of this hiding place actor
	virtual EHidingPlaceType GetHidingPlaceType() = 0;

	/// Get the transform for the hiding place that the charcter shouls assume
	virtual bool GetHidingPlaceTransform(FVector InLocation, FTransform& OutTransform) = 0;

	/// Is the given location within the bounds of this hiding place
	virtual bool IsWithinRange(FVector InLocation, float InTolerance) = 0;

	/// Perform any pre-hide operations before using this hiding place
	virtual void PrepareHidingPlace(FVector InLocation) = 0;

	/// Event delegate to listen for when the hiding place is ready
	virtual FHidingPlaceReadyEvent* GetOnHidingPlaceReadyEvent() = 0;

	/// Occupy the hiding place
	virtual void OccupyHidingPlace(FVector InLocation) = 0;

	/// Vacate the hiding place
	virtual void VacateHidingPlace(FVector InLocation) = 0;

	/// Is the hiding place occupied at the given location
	virtual bool IsHidingPlaceOccupied(FVector InLocation) = 0;
};