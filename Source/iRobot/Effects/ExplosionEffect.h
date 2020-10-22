#pragma once

#include "EffectData.h"
#include "GameFramework/Actor.h"
#include "ExplosionEffect.generated.h"

class UParticleSystem;
class UPointLightComponent;
class USoundCue;

// Spawnable effect for explosion - NOT replicated to clients
// Each explosion type should be defined as separate blueprint
//
UCLASS(Abstract, Blueprintable)
class AExplosionEffect : public AActor
{
	GENERATED_UCLASS_BODY()

	/// Explosion FX
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	UParticleSystem* ExplosionFX;

private:
	/// Explosion light
	UPROPERTY(VisibleDefaultsOnly, Category=Effect)
	UPointLightComponent* ExplosionLight;
public:

	/// How long keep explosion light on?
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	float ExplosionLightFadeOut;

	/// Explosion sound
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	USoundCue* ExplosionSound;
	
	/// Explosion decals
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	struct FDecalData Decal;

	/// Surface data for spawning
	UPROPERTY(BlueprintReadOnly, Category=Surface)
	FHitResult SurfaceHit;

	/// Update fading light
	virtual void Tick(float DeltaSeconds) override;

protected:
	/// Spawn explosion
	virtual void BeginPlay() override;

private:

	/// Point light component name
	FName ExplosionLightComponentName;

public:
	/// Returns ExplosionLight subobject
	FORCEINLINE UPointLightComponent* GetExplosionLight() const { return ExplosionLight; }
};
