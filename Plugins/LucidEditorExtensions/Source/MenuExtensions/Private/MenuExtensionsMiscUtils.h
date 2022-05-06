// Copyright Lucid Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Miscellaneous utility functions used by menu extension classes
 */
class FMenuExtensionsMiscUtils
{
public:
	static bool TryGetAssetSourcePaths(const TArray<FAssetData>& AssetData, TArray<FString>& OutSourcePaths);

private:
	FMenuExtensionsMiscUtils() {}
};
