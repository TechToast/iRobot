// Copyright Lucid Games, Inc. All Rights Reserved.


#include "MenuExtensionsMiscUtils.h"

#include "AssetToolsModule.h"

// Checks for source assets on disk. If they exist returns true and outputs their paths
bool FMenuExtensionsMiscUtils::TryGetAssetSourcePaths(const TArray<FAssetData>& AssetData, TArray<FString>& OutSourcePaths)
{
	OutSourcePaths.Empty();

	FAssetToolsModule& assetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");

	// Check that all of the selected assets are imported
	for (auto& assetDatum : AssetData)
	{
		UClass* assetClass = assetDatum.GetClass();
		if (assetClass)
		{
			TSharedPtr<IAssetTypeActions> AssetTypeActions = assetToolsModule.Get().GetAssetTypeActionsForClass(assetClass).Pin();
			if (!AssetTypeActions.IsValid() || !AssetTypeActions->IsImportedAsset())
			{
				return false;
			}
			else
			{
				TArray<FString> assetSourcePaths;
				TArray<UObject*> typeAssets = { assetDatum.GetAsset() };
				AssetTypeActions->GetResolvedSourceFilePaths(typeAssets, assetSourcePaths);

				// Check if path exists on disk
				for (const FString& sourcePath : assetSourcePaths)
				{
					if (!FPaths::FileExists(sourcePath))
					{
						OutSourcePaths.Empty();
						return false;
					}
					OutSourcePaths.Emplace(sourcePath);
				}

				if (OutSourcePaths.Num() == 0)
				{
					return false;
				}
			}
		}
	}

	return true;
}
