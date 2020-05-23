#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "iRobotCharacter.generated.h"

UCLASS()
class IROBOT_API AiRobotCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AiRobotCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	
	/// Input callbacks
	virtual void MoveForward(float Val);
	virtual void MoveRight(float Val);
	virtual void TurnAtRate(float Rate);
	virtual void LookUpAtRate(float Rate);

	/// Base turn rate, in deg/sec. Other scaling may affect final turn rate.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/// Base look up/down rate, in deg/sec. Other scaling may affect final rate.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/// Spawns default inventory on server
	virtual void SpawnDefaultInventory() {}
};