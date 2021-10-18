#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WeaponAnimInstance.generated.h"

UCLASS()
class UWeaponAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	/// Is the weapon currently firing?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "iRobot")
	bool bIsFiring = false;
};