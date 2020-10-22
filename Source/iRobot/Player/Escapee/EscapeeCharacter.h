#pragma once

#include "CoreMinimal.h"
#include "Player/iRobotCharacter.h"
#include "EscapeeCharacter.generated.h"

UCLASS(config=Game)
class AEscapeeCharacter : public AiRobotCharacter
{
	GENERATED_BODY()

public:
	AEscapeeCharacter();

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const	{ return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const		{ return FollowCamera; }

protected:
	virtual void MoveForward(float Val) override;
	virtual void MoveRight(float Val) override;
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, APawn* InstigatingPawn, AActor* DamageCauser) override;

	/// Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/// Follow camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
};

