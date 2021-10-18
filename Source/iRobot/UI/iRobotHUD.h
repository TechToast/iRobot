// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "iRobotHUD.generated.h"

UCLASS()
class AiRobotHUD : public AHUD
{
	GENERATED_BODY()

public:
	AiRobotHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	void SetCrosshairTexture(UTexture2D* InCrosshairTexture) { CrosshairTexure = InCrosshairTexture; }

private:
	TWeakObjectPtr<UTexture2D> CrosshairTexure;
};

