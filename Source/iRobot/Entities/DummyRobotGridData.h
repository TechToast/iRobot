#pragma once

#include "CoreMinimal.h"
#include "DummyRobotGridData.generated.h"

class ADummyRobotGroup;

/// FastArray replication (See NetSerialization.h for notes)
///

/// Grid cell data structure
USTRUCT()
struct FGridCell2D : public FFastArraySerializerItem
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	uint8 Row;

	UPROPERTY()
	uint8 Column;

	/// Is this cell currently occupied
	UPROPERTY()
	bool bOccupied;

	/// Impact location that last affected this cell
	UPROPERTY()
	FVector ImpactLocation;

	/// Impact velocity vector that last affected this cell
	UPROPERTY()
	FVector ImpactVelocity;

	FGridCell2D() {}
	FGridCell2D(uint8 InRow, uint8 InColumn)
		: Row(InRow)
		, Column(InColumn)
	{}
	FGridCell2D(const FGridCell2D& OtherCell)
		: Row(OtherCell.Row)
		, Column(OtherCell.Column)
		, bOccupied(OtherCell.bOccupied)
		, ImpactLocation(OtherCell.ImpactLocation)
		, ImpactVelocity(OtherCell.ImpactVelocity)
	{}

	/// NOTE: It is not safe to modify the contents of the array serializer within these functions, nor to rely on the contents of the array
	/// being entirely up-to-date as these functions are called on items individually as they are updated, and so may be called in the middle of a mass update.
	//void PreReplicatedRemove(const struct FGridDataArray& InArraySerializer);
	//void PostReplicatedAdd(const struct FGridDataArray& InArraySerializer);
	void PostReplicatedChange(const struct FGridDataArray& InArraySerializer);

	bool operator==(const FGridCell2D& Other) const
	{
		return Row == Other.Row && Column == Other.Column;
	}

	friend uint32 GetTypeHash(const FGridCell2D& Other)
	{
		return GetTypeHash(Other.Row) + GetTypeHash(Other.Column);
	}
};


/// Fast array replication for the data grid
USTRUCT()
struct FGridDataArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	/// Must be called Items to conform to NetSerialisation requirements
	UPROPERTY()
	TArray<FGridCell2D>	Items;

	/// The owning robot group
	UPROPERTY()
	ADummyRobotGroup* Owner;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGridCell2D, FGridDataArray>(Items, DeltaParms, *this);
	}
};


template<>
struct TStructOpsTypeTraits<FGridDataArray> : public TStructOpsTypeTraitsBase2<FGridDataArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};