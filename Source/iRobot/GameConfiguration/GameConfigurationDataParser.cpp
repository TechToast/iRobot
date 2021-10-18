#include "GameConfigurationDataParser.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"
#include "iRobot.h"


bool UGameConfigurationDataParser::LoadData(FStringView InData, FGameConfigurationData& OutData)
{
	bool bParsed = false;

	// First lets check to see if the data is a file path
	if (FPaths::FileExists(InData.GetData()))
	{
		FString JsonString;
		if (FFileHelper::LoadFileToString(JsonString, InData.GetData()))
		{
			// Files coming from the config builder tool will have additional data which the in-built json parser won't like,
			// so we need to strip that out first
			TSharedPtr<FJsonObject>   JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
			if (FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				if (JsonObject->HasField(GAME_CONFIGURATION_DATA_ROOT))
				{
					const TSharedPtr<FJsonObject> GCSystemData = JsonObject->GetObjectField(GAME_CONFIGURATION_DATA_ROOT);
					TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonString);
					FJsonSerializer::Serialize(GCSystemData.ToSharedRef(), Writer);
				}
			}

			bParsed = ParseJSON(JsonString, OutData);
		}

		if (!bParsed)
		{
			UE_LOG(LogGameConfiguration, Error, TEXT("UGameConfigurationDataParser::LoadData() - Failed to parse JSON file: %s"), *InData.GetData());
			return false;
		}
		
	}

	// Otherwise lets check to see if the data is valid JSON
	else
	{
		if (!ParseJSON(InData.GetData(), OutData))
		{
			UE_LOG(LogGameConfiguration, Error, TEXT("UGameConfigurationDataParser::LoadData() - Game configuration data was not valid JSON: %s"), *InData.GetData());
			return false;
		}
	}

	return true;
}


bool UGameConfigurationDataParser::ParseJSON(FStringView InJsonData, FGameConfigurationData& OutData)
{
	// Now parse the actual data
	if (FJsonObjectConverter::JsonObjectStringToUStruct(InJsonData.GetData(), &OutData, 0, 0))
	{
		bool bHasInvalidData = OutData.Validate();
		if (bHasInvalidData)
			UE_LOG(LogGameConfiguration, Warning, TEXT("UGameConfigurationDataParser::ParseJSON() - Data contained some invalid data. Some values will not be used by the system due to invalidation."));
	}

	return true;
}