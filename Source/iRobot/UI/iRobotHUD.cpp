// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "iRobotHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"

AiRobotHUD::AiRobotHUD()
{
}


void AiRobotHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair
	if (CrosshairTexure.IsValid())
	{
		// find center of the Canvas
		const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

		// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
		const FVector2D CrosshairDrawPosition((Center.X - (CrosshairTexure->GetSizeX() * 0.5f)), (Center.Y - (CrosshairTexure->GetSizeY() * 0.5f)));

		// draw the crosshair
		FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTexure->Resource, FLinearColor::White);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem);
	}
}
