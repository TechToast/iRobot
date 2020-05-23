#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "HunterGameMode.generated.h"

class AiRobotCharacter;

UCLASS(minimalapi)
class AHunterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AHunterGameMode();

	virtual void StartPlay() override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleMatchIsWaitingToStart() override;
	virtual void HandleMatchHasStarted() override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/// The Hunter pawn character to use
	UPROPERTY(EditDefaultsOnly, Category = "Hunter Game Mode")
	TSoftClassPtr<AiRobotCharacter> HunterPawnClass;

	/// The Escapee pawn character to use
	UPROPERTY(EditDefaultsOnly, Category = "Hunter Game Mode")
	TSoftClassPtr<AiRobotCharacter> EscapeePawnClass;

	/// Tries to spawn the player's pawn
	//virtual void RestartPlayer(AController* NewPlayer) override;

	/// select best spawn point for player
	//virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/// always pick new random spawn
	//virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

	/// Finishes the match and bumps everyone to main menu.
	/// Only GameInstance should call this function 
	void RequestFinishAndExitToMainMenu();

protected:
	
	/// Match duration
	UPROPERTY()
	int32 MatchDuration;

private:
	bool bHunterAllocated = false;
};



