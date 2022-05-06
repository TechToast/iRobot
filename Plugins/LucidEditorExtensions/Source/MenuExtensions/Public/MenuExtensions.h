// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


/*
* Module for general menu extensions
*/
class FMenuExtensionsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	//TUniquePtr<class FP4MenuExtensions> P4MenuExtensions;
	//TUniquePtr<class FAssetEditorMenuExtensions> AssetEditorMenuExtensions;
	TUniquePtr<class FLevelEditorExtensions> LevelEditorExtensions;
};