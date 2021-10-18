#pragma once

#include "CoreMinimal.h"
#include "GameConfigurationData.h"

class UGameConfigurationDataParser
{
public:

	/// Public method to load data into the game configuration data struct
	/// InData can either be an OS path to a json file or a blob of json data
	bool LoadData(FStringView InData, FGameConfigurationData& OutData);

private:
	// The name of the root data entry in the game configuration data file
	const FString GAME_CONFIGURATION_DATA_ROOT = TEXT("GameConfigurationData");

	/// Parse the game configuration data from the given JSON file
	bool ParseJSON(FStringView InFileName, FGameConfigurationData& OutData);
};