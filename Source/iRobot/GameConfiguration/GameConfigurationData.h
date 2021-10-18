#pragma once

#include "GameConfigurationTypes.h"
#include "GameConfigurationData.generated.h"


/// Base game configuration data unit
USTRUCT()
struct FGameConfigurationDataUnit
{
	GENERATED_BODY()

	virtual ~FGameConfigurationDataUnit() {}

	/// Override this method if you want to ensure your data is valid for use
	virtual bool Validate() { return true; }
};


/// Network overrides for spoofing network lag
USTRUCT()
struct FNetOverrides : public FGameConfigurationDataUnit
{
	GENERATED_BODY()

	UPROPERTY() FGCBool UseNetPktSettings;
	UPROPERTY() FGCInt	PktLoss;	
	UPROPERTY() FGCInt	PktOrder;
	UPROPERTY() FGCInt	PktDup;
	UPROPERTY() FGCInt	PktLag;
	UPROPERTY() FGCInt	PktLagVariance;
};


///
/// FGameConfigurationData
///
/// * This is the parent Game Configuration data structure. 
///
/// * All game configuration data should be stored within this one structure.
///
/// * Any new data added to this (or child) structures should be UPROPERTYs and should use only the following types
///		- FGCInt
///		- FGCFloat
///		- FGCBool
///		- FGCString
///		- FGCName
///		- TArray<FGameConfigurationDataUnit>
///		- TMap<T, FGameConfigurationDataUnit>
///		- FGameConfigurationDataUnit
///
/// * NOTE: Nested containers are not supported by the UE4 reflection system. If you need any nested containers you will
///   need to do this by using structures to contain the nested containers.
///
/// * Don't provide defaults within these structures
///
/// * If you want your data to be validated before allowing it to be used then override the Validate() function of your 
///   new FGameConfigurationDataUnit structs. If you determine that data is invalid then you must call Invalidate() on any 
///   FGCOptional properties that you want to invalidate, otherwise the data will still be accessible.
///
/// * Make sure to include your new data inside the FGameConfigurationData::Validate() function to ensure new data 
///   gets validated
///
USTRUCT()
struct FGameConfigurationData
{
	GENERATED_BODY()

	//
	// NOTE: Please add comments to any new properties to clarify their purpose
	//

	/// Network overrides for spoofing network lag
	UPROPERTY() 
	FNetOverrides NetOverrides;

	/// Validate all of the data to ensure the values are appropriate
	bool Validate();
};