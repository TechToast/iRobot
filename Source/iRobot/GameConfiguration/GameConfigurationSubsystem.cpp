#include "GameConfigurationSubsystem.h"
#include "GameConfigurationDataParser.h"
#include "GameConfigurationData.h"
#include "iRobot.h"

UGameConfigurationSubsystem::UGameConfigurationSubsystem(const FObjectInitializer& ObjectInitializer)
{
	// Local parser testing
	LoadGameConfigurationData();
}


void UGameConfigurationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadGameConfigurationData();
}


UGameConfigurationSubsystem* UGameConfigurationSubsystem::Get(UWorld* World)
{
	if (!World)
		return nullptr;

	if (!World->GetGameInstance())
		return nullptr;
	
	if (UGameConfigurationSubsystem* GCSystem = World->GetGameInstance()->GetSubsystem<UGameConfigurationSubsystem>())
	{
		/// We should not access the system without valid data
		if (GCSystem->IsDataLoaded())
			return GCSystem;
	}

	return nullptr;
}


void UGameConfigurationSubsystem::LoadGameConfigurationData()
{
	UGameConfigurationDataParser Parser;
	FString ConfigFilePath = GetConfigurationFilePath();
	if (!ConfigFilePath.IsEmpty())
	{
		bDataLoaded = Parser.LoadData(ConfigFilePath, GameConfigurationData);
		if (!bDataLoaded)
		{
			UE_LOG(LogGameConfiguration, Error, TEXT("UGameConfigurationSubsystem::LoadGameConfigurationData() - Failed to load configuration data"));
			return;
		}
	}
}


FString UGameConfigurationSubsystem::GetConfigurationFilePath() const
{
#if !(UE_BUILD_SHIPPING) || CSV_PROFILER_ENABLE_IN_SHIPPING
	FString FileRoot = FPaths::ProjectDir();

	// Setup default config filename

#if WITH_EDITOR
	FString ConfigFileName = FileRoot + GAME_CONFIGURATION_EDITOR_FILE_NAME;
#else
	FString ConfigFileName = FileRoot + GAME_CONFIGURATION_FILE_NAME;
#endif

	// Check for command line override for game configuration filename
	if (FParse::Value(FCommandLine::Get(), *GAME_CONFIGURATION_COMMAND_LINE, ConfigFileName))
	{
		ConfigFileName = FileRoot + ConfigFileName;
	}

	// Parse config file
	if (FPaths::FileExists(ConfigFileName))
	{
		return ConfigFileName;
	}
	
	// MB - Commented out for now, but this will need to be correctly implemented
	/*
	else
	{
#if !UE_BUILD_SHIPPING && !WITH_EDITOR
		// Try and load from the local host
		FString HostConfigFileName = "/host/C:\\LocalBSConfig\\BluestarConfig.json";

		if (FPaths::FileExists(HostConfigFileName))
		{
			UE_LOG(LogGameConfiguration, Log, TEXT("Game Configuration file %s loading from host"), *HostConfigFileName);
			ParseGameConfiguration(HostConfigFileName);
			return;
		}
		else
#endif
		{
			UE_LOG(LogGameConfiguration, Log, TEXT("Game Configuration file %s not loaded..."), *ConfigFileName);
		}
	}

	// dedicated command line
	FString ServerConfigCommandLineName;
	if (FParse::Value(FCommandLine::Get(), TEXT("ServerConfig"), ServerConfigCommandLineName))
	{
		FString		  FileDir = FPaths::Combine(*FPaths::ProjectContentDir(), *FString("GameData/MPConfig/"));
		const FString ServerConfigFileName = FileDir + ServerConfigCommandLineName + ".jsonl";
		if (FPaths::FileExists(ServerConfigFileName))
		{
			return ServerConfigFileName;
		}
	}
	*/

#endif

	return FString();
}