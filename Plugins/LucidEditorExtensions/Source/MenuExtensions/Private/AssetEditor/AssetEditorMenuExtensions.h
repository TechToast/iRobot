// Copyright Lucid Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Toolkits/AssetEditorToolkit.h"

/**
 * Class to handle Asset Editor menu extensions
 */
class FAssetEditorMenuExtensions
{
public:
	FAssetEditorMenuExtensions();
	~FAssetEditorMenuExtensions();

private:
	FString SourceFilePath;

	// Delegates
	FAssetEditorExtender AssetEditorMenuExtenderDelegate;
	FDelegateHandle AssetEditorMenuExtenderDelegateHandle;

	// Commands
	TSharedPtr<class FUICommandList> Commands;

	// Initialisation
	void MapCommands();
	void InstallAssetEditorHooks();

	// Menu Extensions
	TSharedRef<FExtender> OnExtendAssetEditorMenu(const TSharedRef<FUICommandList> Commands, const TArray<UObject*> Assets);

	// Cleanup
	void RemoveHooks();
};
