#include "HunterGameMode.h"
#include "UI/iRobotHUD.h"
#include "HunterGameState.h"
#include "Player/iRobotPlayerController.h"
#include "Player/iRobotCharacter.h"
//#include "UObject/ConstructorHelpers.h"

AHunterGameMode::AHunterGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	//DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	//HUDClass = AiRobotHUD::StaticClass();
}


void AHunterGameMode::StartPlay()
{
	Super::StartPlay();
}


void AHunterGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	AHunterGameState* const HunterGameState = Cast<AHunterGameState>(GameState);
	const bool bMatchIsOver = HunterGameState && HunterGameState->HasMatchEnded();
	if (bMatchIsOver)
	{
		ErrorMessage = TEXT("Match is over!");
	}
	else
	{
		// GameSession can be NULL if the match is over
		Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	}
}


void AHunterGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// update spectator location for client
	/*AShooterPlayerController* NewPC = Cast<AShooterPlayerController>(NewPlayer);
	if (NewPC && NewPC->GetPawn() == NULL)
	{
		NewPC->ClientSetSpectatorCamera(NewPC->GetSpawnLocation(), NewPC->GetControlRotation());
	}*/

	// notify new player if match is already in progress
	/*if (NewPC && IsMatchInProgress())
	{
		NewPC->ClientGameStarted();
		NewPC->ClientStartOnlineGame();
	}*/
}


UClass* AHunterGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	AiRobotPlayerController* iRobotPC = Cast<AiRobotPlayerController>(InController);
	if (iRobotPC && !EscapeePawnClass.IsNull() && !HunterPawnClass.IsNull())
	{
		// Has this player already been allocated a pawn class?
		if (!iRobotPC->AllocatedPawnClass)
		{
			// If the hunter has been allocated, just spawn further escapees
			if (!bHunterAllocated)
			{
				iRobotPC->AllocatedPawnClass = HunterPawnClass.LoadSynchronous();
				bHunterAllocated = true;
			}
			else
			{
				iRobotPC->AllocatedPawnClass = EscapeePawnClass.LoadSynchronous();
			}
		}
			
		return iRobotPC->AllocatedPawnClass;
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}


void AHunterGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
}


void AHunterGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AHunterGameState* const HunterGameState = Cast<AHunterGameState>(GameState);
	if (HunterGameState)
	{
		HunterGameState->RemainingTime = MatchDuration;
	}

	// notify players
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AiRobotPlayerController* PC = Cast<AiRobotPlayerController>(*It);
		if (PC)
		{
			PC->ClientGameStarted();
		}
	}
}


void AHunterGameMode::RequestFinishAndExitToMainMenu()
{
	//FinishMatch();

	/*UShooterGameInstance* const GameInstance = Cast<UShooterGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		GameInstance->RemoveSplitScreenPlayers();
	}

	AShooterPlayerController* LocalPrimaryController = nullptr;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AShooterPlayerController* Controller = Cast<AShooterPlayerController>(*Iterator);

		if (Controller == NULL)
		{
			continue;
		}

		if (!Controller->IsLocalController())
		{
			const FText RemoteReturnReason = NSLOCTEXT("NetworkErrors", "HostHasLeft", "Host has left the game.");
			Controller->ClientReturnToMainMenuWithTextReason(RemoteReturnReason);
		}
		else
		{
			LocalPrimaryController = Controller;
		}
	}

	// GameInstance should be calling this from an EndState.  So call the PC function that performs cleanup, not the one that sets GI state.
	if (LocalPrimaryController != NULL)
	{
		LocalPrimaryController->HandleReturnToMainMenu();
	}*/
}