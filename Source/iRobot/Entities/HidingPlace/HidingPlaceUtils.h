#pragma once

#include "CoreMinimal.h"
#include "HidingPlaceData.h"
#include "IHideCompatible.h"

class AActor;

class HidingPlaceUtils
{
public:
	
	/// Initialise the hiding places array
	static void InitialiseHidingPlaces(UWorld* InWorld);

	/// Get a list of all hiding places
	static TArray<TScriptInterface<IHideCompatible>>& GetHidingPlaces() { return HidingPlaceUtils::StaticArray; }

protected:

	/// Static array of all IHideCompatible actors in the current level
	static TArray<TScriptInterface<IHideCompatible>> StaticArray;
};