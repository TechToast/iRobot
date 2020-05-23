#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "iRobotPlayerController.generated.h"


UCLASS()
class AiRobotPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AiRobotPlayerController(const FObjectInitializer& ObjectInitializer);

	virtual void SetupInputComponent() override;

	bool IsGameInputAllowed() const { return true; }

	/// Notify player about started match
	UFUNCTION(reliable, client)
	void ClientGameStarted();

	/// The pawn class allocated to this player controller
	UPROPERTY(Transient)
	UClass* AllocatedPawnClass;
};