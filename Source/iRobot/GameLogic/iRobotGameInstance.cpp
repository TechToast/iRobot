#include "iRobotGameInstance.h"
#include "iRobot.h"
//#include "UObject/ConstructorHelpers.h"
//#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
//#include "OnlineSubsystemTypes.h"
#include "MainMenu/MainMenu.h"


const static FName SESSION_NAME = TEXT("MyBigFatGameSession");
const static FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");


UiRobotGameInstance::UiRobotGameInstance(const FObjectInitializer& ObjectInitializer)
{
}


void UiRobotGameInstance::Init()
{
	IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get();

	if (SubSystem != nullptr)
	{
		SessionInterface = SubSystem->GetSessionInterface();

		UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::Init() - Found session interface"));

		if (SessionInterface.IsValid())
		{
			UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::Init() - SessionInterface.IsValid"));
			
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UiRobotGameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UiRobotGameInstance::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UiRobotGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UiRobotGameInstance::OnJoinSessionsComplete);
		}
	}
	else
	{
		UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::Init() - Found NO SUBSYSTEM"));
	}
}


void UiRobotGameInstance::LoadMainMenu()
{
	if (MenuClass.IsNull()) 
		return;

	MainMenu = CreateWidget<UMainMenu>(this, MenuClass.LoadSynchronous());
	if (!MainMenu.IsValid()) 
		return;

	MainMenu->Setup(this);
}


void UiRobotGameInstance::CreateSession()
{
	UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::CreateSession() - Creating %s"), *SESSION_NAME.ToString());

	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;

		// Switch between bIsLANMatch when using NULL subsystem
		if (IOnlineSubsystem::Get()->GetSubsystemName().ToString() == "NULL")
		{
			SessionSettings.bIsLANMatch = true;
		}
		else
		{
			SessionSettings.bIsLANMatch = false;
		}

		// Number of sessions
		SessionSettings.NumPublicConnections = 2;
		SessionSettings.bIsDedicated = false;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true;
		//SessionSettings.bUseLobbiesIfAvailable = true;
		//SessionSettings.bAllowJoinInProgress = true;
		//SessionSettings.bAllowJoinViaPresence = true;
		SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
	}
}


void UiRobotGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
	// It will not be success if there are more than one session with the same name already created
	if (!bSuccess)
	{
		UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnCreateSessionComplete() - Failure"));
		return;
	}

	UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnCreateSessionComplete() - Success SessionName: %s"), *SessionName.ToString());

	// Teardown Menu and change levels
	if (MainMenu.IsValid())
		MainMenu->Teardown();

	UEngine* Engine = GetEngine();
	if (!Engine)
		return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("[OnCreateSessionComplete::Host]"));

	UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnCreateSessionComplete() - HOST TRAVEL TO LOBBY"));

	UWorld* World = GetWorld();
	if (!World)
		return;

	if (!HunterGameMap.IsNull())
	{
		//bUseSeamlessTravel = true;
		const FString TravelURL = TEXT("/Game/Maps/iRobot_TestMap2?listen");//FString::Printf(TEXT("%s?listen"), *HunterGameMap.ToSoftObjectPath().ToString());
		World->ServerTravel(TravelURL);
	}
	else
	{
		UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnCreateSessionComplete() - No map provided. Unable to travel to new map"));
	}
}


void UiRobotGameInstance::BeginSessionSearch()
{
	if (!MainMenu.IsValid()) 
		return;

	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	if (SessionSearch.IsValid())
	{
		UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OpenSessionListMenu() - Session is valid"));
		//SessionSearch->bIsLanQuery = true;
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}


void UiRobotGameInstance::OnFindSessionsComplete(bool bSuccess)
{
	if (!MainMenu.IsValid()) 
		return;

	TArray<FServerData> ServerData;
	
	if (bSuccess && SessionSearch.IsValid())
	{
		if (SessionSearch->SearchResults.Num() <= 0)
		{
			UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnFindSessionsComplete() - No Sessions Found"));
		}
		else
		{
			for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
			{
				UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnFindSessionsComplete() - Session Name %s"), *SearchResult.GetSessionIdStr());

				FServerData Data;
				FString ServerName;
				if (SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName))
				{
					UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnFindSessionsComplete() - Data found in settings %s"), *ServerName);
					Data.Name = ServerName;
				}
				else
				{
					UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnFindSessionsComplete() - Data NOT found in settings"));
					Data.Name = "Could not find name";
				}

				Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
				Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
				Data.HostUsername = SearchResult.Session.OwningUserName;

				ServerData.Add(Data);
			}
		}
	}
	else
	{
		UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnFindSessionsComplete() - Error session not found"));
	}

	MainMenu->PopulateSessionList(ServerData);
}


void UiRobotGameInstance::OnDestroySessionComplete(FName SessionName, bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnDestroySessionComplete() - Success"));
		CreateSession();
	}
	else
	{
		UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnDestroySessionComplete() - Failure"));
	}
}


void UiRobotGameInstance::Host(FString ServerName)
{
	DesiredServerName = ServerName;

	if (SessionInterface.IsValid())
	{
		// Checks for an existing session
		auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);

		if (ExistingSession != nullptr)
		{
			UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::Host() - There is an existing session. Removing the current one..."));

			SessionInterface->DestroySession(SESSION_NAME);
		}
		else
		{
			UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::Host() - Creating session"));

			// Create a new session
			CreateSession();
		}
	}
	else
	{
		UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::Host() - SessionInterface invalid"));
	}
}


void UiRobotGameInstance::JoinSession(uint32 Index)
{
	if (!SessionInterface.IsValid() || (!SessionSearch.IsValid())) 
		return;

	if (Index < (uint32)(SessionSearch->SearchResults.Num()))
	{
		SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
	}
}


void UiRobotGameInstance::OnJoinSessionsComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnJoinSessionsComplete()"));

	if (MainMenu.IsValid())
		MainMenu->Teardown();

	if (!SessionInterface.IsValid()) 
		return;

	FString Url;
	if (!SessionInterface->GetResolvedConnectString(SESSION_NAME, Url))
	{
		UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnJoinSessionsComplete() - Couldn't get Connect String"));
		return;
	}

	UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnJoinSessionsComplete() - Url: %s"), *Url);

	APlayerController* PlayerController = GetFirstLocalPlayerController();

	if (PlayerController == nullptr)
	{
		UE_LOG(LogiRobot, Warning, TEXT("UiRobotGameInstance::OnJoinSessionsComplete() - Player Controller DOESN'T EXIST"));
		return;
	}

	PlayerController->ClientTravel(Url, ETravelType::TRAVEL_Absolute);
}