#pragma once

#include "GameFramework/GameState.h"
#include "HunterGameState.generated.h"


UCLASS()
class AHunterGameState : public AGameState
{
	GENERATED_UCLASS_BODY()

public:

	/// time left for warmup / match
	UPROPERTY(Transient, Replicated)
	int32 RemainingTime;

	/// is timer paused?
	UPROPERTY(Transient, Replicated)
	bool bTimerPaused;

	void RequestFinishAndExitToMainMenu();
};
