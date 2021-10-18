#pragma once

#include "CoreMinimal.h"


class FGameConfigurationEditor
{
public:
	FGameConfigurationEditor()
	{}

	/// Open the configuration editor with the current user's editor configuration file opened
	void EditEditorConfig();

private:

	const FString GAME_CONFIGURATION_FILE_NAME      = TEXT("iRobotEditorConfig.json");
	const FString GAME_CONFIGURATION_ROOT_NODE_NAME = TEXT("GameConfigurationData");

	/// Attempt to find the configuration file editor
	bool FindGameConfigurationBuilderExecutable(FString& OutPath);

	/// Get the path to the editor config file and create the file if it doesn't exist
	FString GetEditorConfigFile();
};