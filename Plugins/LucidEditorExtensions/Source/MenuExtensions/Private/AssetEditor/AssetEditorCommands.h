// Copyright Lucid Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorStyleSet.h"	 // For FEditorStyle

/**
 * 
 */
class FAssetEditorCommands : public TCommands<FAssetEditorCommands>
{
public:
	FAssetEditorCommands()
		: TCommands<FAssetEditorCommands>
		(
			TEXT("AssetEditorMenuExtensionCommands"),	// Context name for fast lookup
			NSLOCTEXT("Contexts", "AssetEditorMenuExtensionCommands", "Asset Editor Menu Extension Commands"), // Localised context name for displaying
			NAME_None,	// Icon Style Set
			FEditorStyle::GetStyleSetName() // Parent context name. 
			) {}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenSourceLocation;
};
