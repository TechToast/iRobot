#pragma once

#include "UObject/Interface.h"
#include "IShootable.generated.h"


UINTERFACE(MinimalAPI, Blueprintable)
class UShootable : public UInterface
{
	GENERATED_BODY()
};


class IShootable
{
	GENERATED_BODY()
public:

	/// When this shootable actor is shot
	virtual void OnShot(FVector ImpactLocation, FVector ImpactVelocity, FName BoneName) = 0;
};