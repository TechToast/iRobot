#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EscapeeAnimInstance.generated.h"

UCLASS()
class UEscapeeAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	/// Is this character frozen or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "iRobot")
	bool bFrozen = false;
};