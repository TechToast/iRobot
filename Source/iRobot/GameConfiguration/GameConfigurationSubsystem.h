#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameConfigurationData.h"
#include "GameConfigurationSubsystem.generated.h"

UCLASS()
class UGameConfigurationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UGameConfigurationSubsystem(const FObjectInitializer& ObjectInitializer);

	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// End USubsystem

	/// Is the game configuration data loaded into the system
	bool IsDataLoaded() const { return bDataLoaded; }

	/// Convenience getter to access the game configuration system
	/// NOTE: this method will return nullptr if the data has not been loaded successfully
	static UGameConfigurationSubsystem* Get(UWorld* World);

	/// Access the loaded game configuration data structure
	const FGameConfigurationData& GetData() const { return GameConfigurationData; }
	
	/// The name of the configuration file name to use for standard builds
	const FString GAME_CONFIGURATION_FILE_NAME = TEXT("iRobotConfig.json");
	
	/// The name of the configuration file name to use for editor builds
	const FString GAME_CONFIGURATION_EDITOR_FILE_NAME = TEXT("iRobotEditorConfig.json");

	/// The command line flag to override the config file name
	const FString GAME_CONFIGURATION_COMMAND_LINE = TEXT("irobotconfig=");

private:
	/// Load data into the system from the data source
	void LoadGameConfigurationData();

	/// The main imported game configuration data
	FGameConfigurationData GameConfigurationData;

	/// Get the configuration file path for the current executable instance
	FString GetConfigurationFilePath() const;

	bool bDataLoaded = false;
};