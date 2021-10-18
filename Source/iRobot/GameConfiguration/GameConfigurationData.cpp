#include "GameConfigurationData.h"
#include "iRobot.h"


template<typename T, typename T2>
bool ValidateMap(const TMap<T, T2>& InMap, FStringView InMapName)
{
	for (const auto& Pair : InMap)
	{
		if (!Pair.Value.Validate())
		{
			UE_LOG(LogGameConfiguration, Error, TEXT("FGameConfigurationData::Validate() - Validation failed on %s"), InMapName.GetData());
			return false;
		}
	}

	return true;
}


template<typename T>
bool ValidateArray(const TArray<T>& InArray, FStringView InArrayName)
{
	for (const auto& Data : InArray)
	{
		if (!Data.Validate())
		{
			UE_LOG(LogGameConfiguration, Error, TEXT("FGameConfigurationData::Validate() - Validation failed on %s"), InArrayName.GetData());
			return false;
		}
	}

	return true;
}


template<typename T>
bool ValidateStructure(const T& InStruct, FStringView InStructName)
{
	if (!InStruct.Validate())
	{
		UE_LOG(LogGameConfiguration, Error, TEXT("FGameConfigurationData::Validate() - Validation failed on %s"), InStructName.GetData());
		return false;
	}

	return true;
}


bool FGameConfigurationData::Validate()
{
	// Make sure you add any new data structs to this list to ensure they are correctly validated
	//
	// You can use the convenience methods ValidateMap(), ValidateArray() or ValidateStructure() to 
	// call the relevant validate function on your data units
	//
	// Note, you only need to include structures which need to be validated. If your data doesn't require validation
	// you can ommit them from this function
	
	// MB - Left in as an example of how to use the function
	//
	/*if (!(

		ValidateMap(SlamBalancingData, TEXT("SlamBalancingData"))									&&
		ValidateMap(HeroVehicleAbilityBalancingData, TEXT("HeroVehicleAbilityBalancingData"))		&&
		ValidateMap(CharacterBalancingData, TEXT("CharacterBalancingData"))							&&
		ValidateStructure(HeroVehicleSpawnerBalancingData, TEXT("HeroVehicleSpawnerBalancingData"))

		))
		return false;*/

	return true;
}