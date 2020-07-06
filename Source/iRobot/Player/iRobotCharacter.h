#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "iRobotHitInfo.h"
#include "iRobotCharacter.generated.h"

class USoundCue;

UCLASS()
class IROBOT_API AiRobotCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AiRobotCharacter();

	float GetHealth() const { return Health; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

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

	/// Animation played on death
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* DeathAnim;

	/// Sound played on death, local player only
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* DeathSound;

	/// Default health value
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float DefaultHealth = 100.f;

	/// Spawns default inventory on server
	virtual void SpawnDefaultInventory() {}

	/// This character has just died
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

private:

	/// Play hit effects
	void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser);

	/// Can this character currently be killed
	bool CanDie() const;

	/// Kill this character (Server/authority only)
	bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser);

	/// Switch to ragdoll
	void SetRagdollPhysics();

	/// Replicate where this pawn was last hit and damaged
	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastHitInfo)
	FHitInfo LastHitInfo;

	/// Set hit data to be replicated
	void ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, APawn* InstigatingPawn, AActor* DamageCauser, bool bKilled);

	/// play hit or death on client
	UFUNCTION()
	void OnRep_LastHitInfo();

	/// Remaining health of this character (0 - 100)
	UPROPERTY(Replicated)
	float Health;

	bool bIsDying = false;
};