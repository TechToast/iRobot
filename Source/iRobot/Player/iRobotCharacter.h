#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "iRobotHitInfo.h"
#include "Interfaces/IInteraction.h"
#include "iRobotCharacterData.h"
#include "iRobotCharacter.generated.h"

class USoundCue;

UCLASS()
class IROBOT_API AiRobotCharacter : public ACharacter, public IInteractor
{
	GENERATED_BODY()

public:
	AiRobotCharacter(const FObjectInitializer& ObjectInitializer);

	float GetHealth() const { return Health; }
	
	virtual FTransform GetCameraTransform() const;

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
	void OnInteractButtonPressed();

	/// Base turn rate, in deg/sec. Other scaling may affect final turn rate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "iRobotCharacter|Camera")
	float BaseTurnRate;

	/// Base look up/down rate, in deg/sec. Other scaling may affect final rate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "iRobotCharacter|Camera")
	float BaseLookUpRate;

	/// How far from the character objects can be interacted with
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "iRobotCharacter|Interaction")
	float MaxInteractionDistance = 300.f;

	/// Animation played on death
	//UPROPERTY(EditDefaultsOnly, Category = Animation)
	//UAnimMontage* DeathAnim;

	/// Sound played on death, local player only
	UPROPERTY(EditDefaultsOnly, Category = "iRobotCharacter|Sound")
	USoundCue* DeathSound;

	/// Default health value
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "iRobotCharacter")
	float DefaultHealth = 100.f;

	/// Spawns default inventory on server
	virtual void SpawnDefaultInventory() {}

	/// This character has just died
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

	/// Start IInteractor interface
	virtual bool HasInteractionCabability(EInteractionCapability InCapability);
	virtual void SetInteractionCapability(EInteractionCapability InCapability);
	virtual void RemoveInteractionCapability(EInteractionCapability InCapability);
	/// End IInteract interface

private:

	/// Interact with an interactable object
	void Interact();

	/// RPC to server to perform an Interaction operation
	UFUNCTION(Reliable, Server, WithValidation)
	void SERVER_StartInteraction();

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

	/// The interaction capabilities of this character
	int32 InteractionCapabilities = 0;
};