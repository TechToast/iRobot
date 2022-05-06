// Copyright Epic Games, Inc. All Rights Reserved.

#include "MenuExtensions.h"

//#include "P4/P4MenuExtensions.h"
//#include "AssetEditor/AssetEditorMenuExtensions.h"
#include "LevelEditor/LevelEditorExtensions.h"

#define LOCTEXT_NAMESPACE "FMenuExtensionsModule"

void FMenuExtensionsModule::StartupModule()
{
	//P4MenuExtensions = MakeUnique<FP4MenuExtensions>();
	//AssetEditorMenuExtensions = MakeUnique<FAssetEditorMenuExtensions>();
	LevelEditorExtensions = MakeUnique<FLevelEditorExtensions>();
}

void FMenuExtensionsModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMenuExtensionsModule, MenuExtensions)