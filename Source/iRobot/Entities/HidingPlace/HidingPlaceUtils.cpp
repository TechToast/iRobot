#include "HidingPlaceUtils.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"


TArray<TScriptInterface<IHideCompatible>> HidingPlaceUtils::StaticArray;


void HidingPlaceUtils::InitialiseHidingPlaces(UWorld* InWorld)
{
	StaticArray.Empty();

	TArray<AActor*> HidingPlaces;
	UGameplayStatics::GetAllActorsWithInterface(InWorld, UHideCompatible::StaticClass(), HidingPlaces);

	for (AActor* HidingPlace : HidingPlaces)
	{
		TScriptInterface<IHideCompatible> Interface;
		if (IHideCompatible* HideCompatible = Cast<IHideCompatible>(HidingPlace))
		{
			Interface.SetInterface(HideCompatible);
			Interface.SetObject(HidingPlace);
			
			StaticArray.Add(Interface);
		}
	}
}