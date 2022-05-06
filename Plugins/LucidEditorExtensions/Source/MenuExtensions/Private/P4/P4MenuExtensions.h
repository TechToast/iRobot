#if 0

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ContentBrowserModule.h"	// For FContentBrowserModule

/**
 * Class to handle Source Control menu extensions
 */
class FP4MenuExtensions
{
public:
	FP4MenuExtensions();
	~FP4MenuExtensions();

private:
	TArray<FString> SourceFilePaths;
	FString SelectedAssetPath;

	FContentBrowserMenuExtender_SelectedAssets ContentBrowserAssetContextExtenderDelegate;
	FDelegateHandle ContentBrowserAssetContextExtenderDelegateHandle;

	// Commands
	TSharedPtr<class FUICommandList> Commands;

	// Initialisation
	void MapCommands();
	void InstallContentBrowserHooks();

	// Menu Extensions
	TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);

	bool AreAssetsImportedAndOnDisk(const TArray<FAssetData>& SelectedAssets);
	bool AreAssetsInDepot() const;
	bool AreAssetsCheckedOutOrAdded() const;

	// Cleanup
	void RemoveHooks();
};

#endif