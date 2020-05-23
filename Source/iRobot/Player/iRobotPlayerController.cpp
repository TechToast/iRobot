#include "iRobotPlayerController.h"

AiRobotPlayerController::AiRobotPlayerController(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	//PlayerCameraManagerClass = AiRobotPlayerCameraManager::StaticClass();
	//CheatClass = UiRobotCheatManager::StaticClass();
}


void AiRobotPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// UI input
	//InputComponent->BindAction("InGameMenu", IE_Pressed, this, &AiRobotPlayerController::OnToggleInGameMenu);
	//InputComponent->BindAction("Scoreboard", IE_Pressed, this, &AiRobotPlayerController::OnShowScoreboard);
	//InputComponent->BindAction("Scoreboard", IE_Released, this, &AiRobotPlayerController::OnHideScoreboard);
	//InputComponent->BindAction("ConditionalCloseScoreboard", IE_Pressed, this, &AiRobotPlayerController::OnConditionalCloseScoreboard);
	//InputComponent->BindAction("ToggleScoreboard", IE_Pressed, this, &AiRobotPlayerController::OnToggleScoreboard);

	// voice chat
	//InputComponent->BindAction("PushToTalk", IE_Pressed, this, &APlayerController::StartTalking);
	//InputComponent->BindAction("PushToTalk", IE_Released, this, &APlayerController::StopTalking);
}


void AiRobotPlayerController::ClientGameStarted_Implementation()
{
	// Enable controls mode now the game has started
	SetIgnoreMoveInput(false);

	/*AShooterHUD* ShooterHUD = GetShooterHUD();
	if (ShooterHUD)
	{
		ShooterHUD->SetMatchState(EShooterMatchState::Playing);
		ShooterHUD->ShowScoreboard(false);
	}

	const UWorld* World = GetWorld();

	// Send round start event
	const IOnlineEventsPtr Events = Online::GetEventsInterface(World);
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	if (LocalPlayer != nullptr && World != nullptr && Events.IsValid())
	{
		FUniqueNetIdRepl UniqueId = LocalPlayer->GetPreferredUniqueNetId();

		if (UniqueId.IsValid())
		{
			// Generate a new session id
			Events->SetPlayerSessionId(*UniqueId, FGuid::NewGuid());

			FString MapName = *FPackageName::GetShortName(World->PersistentLevel->GetOutermost()->GetName());

			// Fire session start event for all cases
			FOnlineEventParms Params;
			Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
			Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
			Params.Add(TEXT("MapName"), FVariantData(MapName));

			Events->TriggerEvent(*UniqueId, TEXT("PlayerSessionStart"), Params);

			// Online matches require the MultiplayerRoundStart event as well
			UShooterGameInstance* SGI = Cast<UShooterGameInstance>(World->GetGameInstance());

			if (SGI && (SGI->GetOnlineMode() == EOnlineMode::Online))
			{
				FOnlineEventParms MultiplayerParams;

				// @todo: fill in with real values
				MultiplayerParams.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
				MultiplayerParams.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
				MultiplayerParams.Add(TEXT("MatchTypeId"), FVariantData((int32)1)); // @todo abstract the specific meaning of this value across platforms
				MultiplayerParams.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

				Events->TriggerEvent(*UniqueId, TEXT("MultiplayerRoundStart"), MultiplayerParams);
			}

			bHasSentStartEvents = true;
		}
	}*/
}