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
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	float TimeBeforeRemoval = 10.f;

	/// Apply an impulse to this robot
	void ApplyImpulse(FVector ImpactLocation, FVector ImpactVelocity, FName BoneName = NAME_None);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	FScriptDelegate PointDelegate;
	FScriptDelegate RadialDelegate;

	FTimerHandle RemovalHandle;
};
