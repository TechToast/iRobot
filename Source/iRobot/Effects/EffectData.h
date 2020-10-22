#pragma once

#include "CoreMinimal.h"
#include "EffectData.generated.h"

USTRUCT()
struct FDecalData
{
	GENERATED_USTRUCT_BODY()

	/// Material
	UPROPERTY(EditDefaultsOnly, Category=Decal)
	UMaterial* DecalMaterial;

	/// Quad size (width & height)
	UPROPERTY(EditDefaultsOnly, Category=Decal)
	float DecalSize = 256.f;

	/// Lifespan
	UPROPERTY(EditDefaultsOnly, Category=Decal)
	float LifeSpan = 10.f;
};