#include "HunterGameState.h"
#include "HunterGameMode.h"
#include "Net/UnrealNetwork.h"


AHunterGameState::AHunterGameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	RemainingTime = 0;
	bTimerPaused = false;
}


void AHunterGameState::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME( AHunterGameState, RemainingTime );
	DOREPLIFETIME( AHunterGameState, bTimerPaused );
}


void AHunterGameState::RequestFinishAndExitToMainMenu()
{
	if (AuthorityGameMode)
	{
		// we are server, tell the gamemode
		AHunterGameMode* const GameMode = Cast<AHunterGameMode>(AuthorityGameMode);
		if (GameMode)
		{
			GameMode->RequestFinishAndExitToMainMenu();
		}
	}
	else
	{
		// we are client, handle our own business
		/*UShooterGameInstance* GameInstance = Cast<UShooterGameInstance>(GetGameInstance());
		if (GameInstance)
		{
			GameInstance->RemoveSplitScreenPlayers();
		}*/

		/*APlayerController* const PrimaryPC = GetGameInstance()->GetFirstLocalPlayerController();
		if (PrimaryPC)
		{
			PrimaryPC->HandleReturnToMainMenu();
		}*/
	}

}
