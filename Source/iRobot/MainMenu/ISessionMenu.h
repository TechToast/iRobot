#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ISessionMenu.generated.h"


// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USessionMenu : public UInterface
{
	GENERATED_BODY()
};


class ISessionMenu
{
	GENERATED_BODY()

public:

	virtual void Host(FString ServerName) = 0;
	virtual void JoinSession(uint32 Index) = 0;
	//virtual void EndSession() = 0;
	virtual void BeginSessionSearch() = 0;

};
