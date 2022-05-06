// Copyright Lucid Games, Inc. All Rights Reserved.


#include "AssetEditor/AssetEditorCommands.h"

#define LOCTEXT_NAMESPACE "AssetEditorCommands"

void FAssetEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenSourceLocation, "Open Source Location", "Open source location in explorer.", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE 