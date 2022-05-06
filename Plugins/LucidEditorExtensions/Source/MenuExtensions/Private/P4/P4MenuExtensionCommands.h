// Copyright Lucid Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorStyleSet.h"	 // For FEditorStyle

/**
 * 
 */
class FP4MenuExtensionCommands : public TCommands<FP4MenuExtensionCommands>
{
public:
	FP4MenuExtensionCommands()
		: TCommands<FP4MenuExtensionCommands>
		(
			TEXT("MenuExtensionCommands"),
			NSLOCTEXT("Contexts", "MenuExtensionCommands", "Menu Extension Commands"),
			NAME_None,
			FEditorStyle::GetStyleSetName()
			) {}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> CheckoutSourceAsset;
	TSharedPtr<FUICommandInfo> RevertSourceAsset;
	TSharedPtr<FUICommandInfo> AddSourceAsset;
	TSharedPtr<FUICommandInfo> FocusSourceWorkspaceTree;
	TSharedPtr<FUICommandInfo> FocusWorkspaceTree;
};
