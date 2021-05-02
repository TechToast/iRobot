#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DummyRobot.generated.h"

class USkeletalMeshComponent;
class USceneComponent;
class AController;

UCLASS()
class IROBOT_API ADummyRobot : public AActor
{
	GENERATED_BODY()
	
public:	
	ADummyRobot();

	/// How long to wait after being shot before removing from the world
	UPROPERTY(EditDefaultsOnly, Category = "Dissolve")
	FVector2D TimeBeforeRemovalRange = FVector2D(5.f, 8.f);

	/// How long the dissolve effect should take
	UPROPERTY(EditDefaultsOnly, Category = "Dissolve")
	float DissolveTime = 1.f;

	/// Name of the dissolve parameter on the dummy robot character
	UPROPERTY(EditDefaultsOnly, Category = "Dissolve")
	FName DissolveParamName = TEXT("Dissolve");

	/// Apply an impulse to this robot
	void ApplyImpulse(FVector ImpactLocation, FVector ImpactVelocity, FName BoneName = NAME_None);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	/// Called when point damage has been received by this actor
	UFUNCTION()
	void OnPointDamageReceived(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, 
		UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, 
		AActor* DamageCauser);

	/// Called when radial damage has been received by this actor
	UFUNCTION()
	void OnRadialDamageReceived(AActor* DamagedActor, float Damage, const UDamageType* DamageType, 
		FVector Origin, FHitResult HitInfo,  AController* InstigatedBy, 
		AActor* DamageCauser);

protected:
	/// Root component
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USceneComponent* Scene;

	/// Dynamic mesh of the robot
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* DynamicMesh;

	
private:
	
	/// Remove this actor after a delay
	UFUNCTION()
	void RemoveAfterDelay();

	/// Cause the dummy robot to dissolve
	void Dissolve();

	float ElapsedDissolveTime = 0;

	TArray<TWeakObjectPtr<UMaterialInstanceDynamic>> Materials;

	FScriptDelegate PointDelegate;
	FScriptDelegate RadialDelegate;

	FTimerHandle RemovalHandle;
};
