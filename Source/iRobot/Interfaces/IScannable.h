#pragma once

#include "UObject/Interface.h"
#include "IScannable.generated.h"


///
/// IScannable interface
///


UINTERFACE(MinimalAPI, Blueprintable)
class UScannable : public UInterface
{
	GENERATED_BODY()
};


class IScannable
{
	GENERATED_BODY()
public:

	/// Called when this object is scanned
	virtual void OnScanned(int32 ScannedIndex) = 0;
	
	/// Get an index to identify the scan location of this scannable object
	virtual int32 GetScanIndex(const FVector& ScanHitLocation) = 0;
};


