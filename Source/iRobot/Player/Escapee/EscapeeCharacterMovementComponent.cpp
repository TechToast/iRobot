#include "EscapeeCharacterMovementComponent.h"
#include "EscapeeCharacter.h"

UEscapeeCharacterMovementComponent::UEscapeeCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}


void UEscapeeCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningEscapeeCharacter = Cast<AEscapeeCharacter>(GetOwner());
	ensureMsgf(OwningEscapeeCharacter.IsValid(), TEXT("UEscapeeCharacterMovementComponent::BeginPlay() - Not attached to AEscapeeCharacter"));
}


void UEscapeeCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const float SpeedSquared = Velocity.SizeSquared();
	if (SpeedSquared != LastSpeedSquared)
	{
		if (OwningEscapeeCharacter.IsValid())
			OwningEscapeeCharacter->OnVelocityUpdated();
	}

	LastSpeedSquared = SpeedSquared;
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