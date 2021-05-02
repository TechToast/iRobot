#pragma once

#include "UObject/Interface.h"
#include "IInteraction.generated.h"


enum EInteractionCapability
{
	INTERACTION_CAPABILITY_OpenDoors		= 0x01,
	INTERACTION_CAPABILITY_DissolveRobot	= 0x02,
	//INTERACTION_CAPABILITY_Blah1			= 0x04,
	//INTERACTION_CAPABILITY_Blah2			= 0x08,
};


///
/// IInteractor interface
///


UINTERFACE(MinimalAPI, Blueprintable)
class UInteractor : public UInterface
{
	GENERATED_BODY()
};


class IInteractor
{
	GENERATED_BODY()
public:

	/// Does this interacting actor have the given interaction capability
	virtual bool HasInteractionCabability(EInteractionCapability InCapability) = 0;

	/// Set/Remove the given capability on the interactor
	virtual void SetInteractionCapability(EInteractionCapability InCapability) = 0;
	virtual void RemoveInteractionCapability(EInteractionCapability InCapability) = 0;
};


///
/// IInteractable interface
///


UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};


class IInteractable
{
	GENERATED_BODY()
public:

	/// When this interactable actor is interacted with
	virtual void OnInteraction(IInteractor* InInstigator, FHitResult& InteractionHit) = 0;
};


