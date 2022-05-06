#pragma once

#include "CoreMinimal.h"
#include "Player/iRobotCharacter.h"
#include "Entities/HidingPlace/HidingPlaceData.h"
#include "Interfaces/IScannable.h"
#include "EscapeeCharacter.generated.h"

class AHidingPlace;
class IHideCompatible;
class UEscapeeCharacterMovementComponent;


UCLASS()
class IROBOT_API AEscapeeCharacter : public AiRobotCharacter, public IScannable
{
	GENERATED_BODY()

public:
	AEscapeeCharacter(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const	{ return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const		{ return FollowCamera; }

	/// Start IScannable interface
	virtual void OnScanned(int32 ScannedIndex);
	virtual int32 GetScanIndex(const FVector& ScanHitLocation) { return 0; }
	/// End IScannable interface

	void OnVelocityUpdated();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	//virtual void Tick(float DeltaTime) override;
	virtual FTransform GetCameraTransform() const override;
	virtual void MoveForward(float Val) override;
	virtual void MoveRight(float Val) override;
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, APawn* InstigatingPawn, AActor* DamageCauser) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Jump() override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	// Input callbacks
	//void OnHideButtonHeld();
	//void OnHideButtonReleased();
	void OnHideButtonPressed();
	void OnCrouchButtonPressed();

	/// Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USpringArmComponent* CameraBoom;

	/// Follow camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCameraComponent* FollowCamera;

	/// How long before the scanned character is restored to its default state
	UPROPERTY(EditDefaultsOnly, Category = "Escapee")
	float TimerBeforeScanStateChange = 5.f;

	/// The index of the material used by the scan state
	UPROPERTY(EditDefaultsOnly, Category = "Escapee")
	int32 ScannedStateMaterialIndex = 0;

	/// The name of the material param used to enable/disable the scanned state
	UPROPERTY(EditDefaultsOnly, Category = "Escapee")
	FName ScannedStateParamName = TEXT("ScannedState");

	/// Played when the character is scanned?
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* ScannedSound;

private:

	/// Get the most appropriate nearby hiding place
	bool GetHidingPlace(TScriptInterface<IHideCompatible>& HidingPlace) const;

	/// Callback triggered when the hiding place is ready
	void OnHidingPlaceReady();

	/// Callback when the scane state is restored to default
	UFUNCTION()
	void OnScanStateRestored();

	/// Cause this escapee character to hide/unhide in place
	void Hide();
	void UnHide();

	/// RPC to server to hide the character
	UFUNCTION(Reliable, Server, WithValidation)
	void SERVER_Hide();

	/// RPC to server to stop hiding the character
	UFUNCTION(Reliable, Server, WithValidation)
	void SERVER_UnHide();

	/// Hold a reference to the anim instance
	TWeakObjectPtr<class UEscapeeAnimInstance> AnimInstance;

	/// The current hide state of this character
	UPROPERTY(ReplicatedUsing = OnRep_HideState)
	EHidingPlaceType HideState = EHidingPlaceType::HP_None;

	/// Called when HideState is replicated
	UFUNCTION()
	void OnRep_HideState();

	bool bWantsToHide = false;

	TWeakObjectPtr<UEscapeeCharacterMovementComponent> EscapeeCharMove;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHidingPlaceTransform)
	FHidingPlaceTransform CurrentHidingPlaceTransform;

	UFUNCTION()
	void OnRep_CurrentHidingPlaceTransform();

	TScriptInterface<IHideCompatible> CurrentHidingPlace;

	FDelegateHandle HidingPlaceReadyHandle;

	void SetScannedState(bool bInState);

	UPROPERTY(ReplicatedUsing=OnRep_bScanned)
	bool bScanned = false;

	UFUNCTION()
	void OnRep_bScanned();

	TWeakObjectPtr<UMaterialInstanceDynamic> ScannedStateMaterial;

	FTimerHandle ScannedTimerHandle;
};

