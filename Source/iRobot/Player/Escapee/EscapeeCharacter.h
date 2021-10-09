#pragma once

#include "CoreMinimal.h"
#include "Player/iRobotCharacter.h"
#include "Entities/HidingPlace/HidingPlaceData.h"
#include "EscapeeCharacter.generated.h"

class AHidingPlace;
class IHideCompatible;
class UEscapeeCharacterMovementComponent;


UCLASS()
class IROBOT_API AEscapeeCharacter : public AiRobotCharacter
{
	GENERATED_BODY()

public:
	AEscapeeCharacter(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const	{ return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const		{ return FollowCamera; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual FTransform GetCameraTransform() const override;
	virtual void MoveForward(float Val) override;
	virtual void MoveRight(float Val) override;
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, APawn* InstigatingPawn, AActor* DamageCauser) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaTime) override;

	// Input callbacks
	//void OnHideButtonHeld();
	//void OnHideButtonReleased();
	void OnHideButtonPressed();

	/// Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/// Follow camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

private:

	/// Get the most appropriate nearby hiding place
	bool GetHidingPlace(TScriptInterface<IHideCompatible>& HidingPlace) const;

	/// Callback triggered when the hiding place is ready
	void OnHidingPlaceReady();

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
};

