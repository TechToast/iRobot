#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EscapeeAnimInstance.generated.h"

UCLASS()
class UEscapeeAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	void SetIsFrozen(bool bInIsFrozen)				{ bIsFrozen		= bInIsFrozen; }
	void SetIsCrouching(bool bInIsCrouching)		{ bIsCrouching	= bInIsCrouching; }
	void SetIsInAir(bool bInIsInAir)				{ bIsInAir		= bInIsInAir; }
	void SetCurrentSpeed(float InCurrentSpeed)		{ CurrentSpeed	= InCurrentSpeed; }

protected:

	/// Is this character frozen or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "iRobot")
	bool bIsFrozen = false;

	/// Is this character crouching or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "iRobot")
	bool bIsCrouching = false;

	/// Is this character in the air or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "iRobot")
	bool bIsInAir = false;

	/// The current speed of the escapee character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "iRobot")
	float CurrentSpeed = 0.f;
};