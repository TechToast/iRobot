#include "EscapeeCharacterMovementComponent.h"

UEscapeeCharacterMovementComponent::UEscapeeCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UEscapeeCharacterMovementComponent::ReplicateMoveToServer(float DeltaTime, const FVector& NewAcceleration)
{
	Super::ReplicateMoveToServer(DeltaTime, NewAcceleration);

	if (bUseHidingPlaceRotation)
	{
		if (UpdatedComponent)
		{
			UpdatedComponent->SetRelativeRotation(HidingPlaceRotation);
		}
	}
}