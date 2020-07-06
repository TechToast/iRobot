#include "DummyRobotGridData.h"
#include "DummyRobotGroup.h"
#include "iRobot.h"


//void FGridCell2D::PreReplicatedRemove(const struct FGridDataArray& InArraySerializer) 
//{
//	UE_LOG(LogiRobot, Log, TEXT("PreReplicatedRemove"));
//}


//void FGridCell2D::PostReplicatedAdd(const struct FGridDataArray& InArraySerializer)
//{
//	UE_LOG(LogiRobot, Log, TEXT("PreReplicatedAdd"));
//}


void FGridCell2D::PostReplicatedChange(const struct FGridDataArray& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		const FGridCell2D ThisCell(*this);
		InArraySerializer.Owner->GetGridCellChangedDelegate()->ExecuteIfBound(ThisCell);
	}
	else
	{
		UE_LOG(LogiRobot, Error, TEXT("FGridCell2D::PostReplicatedChange() : FGridDataArray didn't have an owner assigned. Replication callbacks wont work"));
	}
}