#pragma once

#include "CoreMinimal.h"
#include "GameFramework\CharacterMovementComponent.h"
#include "EscapeeCharacterMovementComponent.generated.h"

class AEscapeeCharacter;

UCLASS()
class IROBOT_API UEscapeeCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	UEscapeeCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	void SetHidingPlaceRotation(FQuat InRotation)					{ HidingPlaceRotation = InRotation; }
	void SetUseHidingPlaceRotation(bool bInSetHidingPlaceRotation)	{ bUseHidingPlaceRotation = bInSetHidingPlaceRotation; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void ReplicateMoveToServer(float DeltaTime, const FVector& NewAcceleration) override;

private:
	bool bUseHidingPlaceRotation = false;
	FQuat HidingPlaceRotation;

	TWeakObjectPtr<AEscapeeCharacter> OwningEscapeeCharacter;
	float LastSpeedSquared = 0;
};