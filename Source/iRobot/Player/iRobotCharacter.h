#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/IShootable.h"
#include "iRobotCharacter.generated.h"

UCLASS()
class IROBOT_API AiRobotCharacter : public ACharacter, public IShootable
{
	GENERATED_BODY()

public:
	AiRobotCharacter();

	float GetHealth() const { return Health; }

	virtual void CauseDamage(float DamageAmount);
	virtual void HealDamage(float HealAmount);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	virtual void OnShot(FVector ImpactLocation, FVector ImpactVelocity, FName BoneName);

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

private:

	/// Remaining health of this character (0 - 1)
	UPROPERTY(ReplicatedUsing=OnRep_Health)
	float Health = 1;

	/// Called when Health is replicated
	UFUNCTION()
	void OnRep_Health();
};