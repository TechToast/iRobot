#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IShootable.h"
#include "DummyRobot.generated.h"

class USkeletalMeshComponent;
class USceneComponent;

UCLASS()
class IROBOT_API ADummyRobot : public AActor, public IShootable
{
	GENERATED_BODY()
	
public:	
	ADummyRobot();

	/// How long to wait after being shot before removing from the world
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	float TimeBeforeRemoval = 10.f;

	// Begin IShootable interface
	virtual void OnShot(FVector ImpactLocation, FVector ImpactVelocity, FName BoneName) override;
	// End IShootable interface

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	FTimerHandle RemovalHandle;

};
