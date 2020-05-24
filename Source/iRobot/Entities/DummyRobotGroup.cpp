#include "DummyRobotGroup.h"
#include "DummyRobot.h"
#include "Engine/SkeletalMesh.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Net/UnrealNetwork.h"

ADummyRobotGroup::ADummyRobotGroup()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	InstancedMesh = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
	InstancedMesh->SetMobility(EComponentMobility::Movable);
	InstancedMesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	InstancedMesh->SetRelativeLocation(FVector::ZeroVector);
	RootComponent = InstancedMesh;
}


void ADummyRobotGroup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADummyRobotGroup, GridCells);
}


void ADummyRobotGroup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (InstancedMesh)
	{
		InstancedMesh->ClearInstances();

		if (InstancedMesh->GetStaticMesh() != nullptr)
		{
			for (int32 Column = 0; Column < NumColumns; ++Column)
			{
				for (int32 Row = 0; Row < NumRows; ++Row)
				{
					FTransform Trans = GetActorTransform();
					Trans.SetLocation(FVector(Spacing.X * Row, Spacing.Y * Column, 0.f));
					InstancedMesh->AddInstance(Trans);
				}
			}
		}
	}
}


void ADummyRobotGroup::BeginPlay()
{
	Super::BeginPlay();
	
	CreateCellToInstanceMapping();
	GenerateInitialGrid();
}


void ADummyRobotGroup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}


void ADummyRobotGroup::OnShot(FVector ImpactLocation, FVector ImpactVelocity, FName BoneName)
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	FTransform InstanceTransform;
	int32 InstanceId;
	FGridCell2D Cell;
	if (FindClosestInstance(ImpactLocation, InstanceId, Cell, InstanceTransform))
	{
		// Rely on the fact that the array order is correct on server
		int32 CellIndex = (Cell.Column * NumRows) + Cell.Row;
		if (CellIndex < GridCells.Items.Num())
		{
			GridCells.Items[CellIndex].bOccupied = false;
			GridCells.Items[CellIndex].ImpactLocation = ImpactLocation;
			GridCells.Items[CellIndex].ImpactVelocity = ImpactVelocity;
			GridCells.MarkItemDirty(GridCells.Items[CellIndex]);
		}

		// Manually trigger the callback on server
		OnGridCellChange(GridCells.Items[CellIndex]);
	}
}


void ADummyRobotGroup::RemoveInstance(int32 InstanceId)
{
	InstancedMesh->RemoveInstance(InstanceId);
	CreateCellToInstanceMapping();
}


void ADummyRobotGroup::RemoveInstances(TArray<int32> InstanceIds)
{
	InstancedMesh->RemoveInstances(InstanceIds);
	CreateCellToInstanceMapping();
}


void ADummyRobotGroup::GenerateInitialGrid()
{
	if (GetLocalRole() == ROLE_Authority && !bGridInitialised)
	{
		for (int32 Column = 0; Column < NumColumns; ++Column)
		{
			for (int32 Row = 0; Row < NumRows; ++Row)
			{
				FGridCell2D Cell(Row, Column);
				Cell.bOccupied = true;
				GridCells.MarkItemDirty(GridCells.Items.Add_GetRef(Cell));
			}
		}
	}

	// Ensure the grid cell array has an owner specified
	GridCells.Owner = this;
	
	// On clients, we need to listen out for the callback
	if (GetNetMode() == NM_Client)
		GetGridCellChangedDelegate()->BindUObject(this, &ADummyRobotGroup::OnGridCellChange);
}


void ADummyRobotGroup::CreateCellToInstanceMapping()
{
	CellToInstanceMapping.Empty();

	for (int32 Index = 0; Index < InstancedMesh->PerInstanceSMData.Num(); ++Index)
	{
		FVector Location = InstancedMesh->PerInstanceSMData[Index].Transform.GetOrigin();
		FGridCell2D Cell(FMath::RoundToInt(Location.X / Spacing.X), FMath::RoundToInt(Location.Y / Spacing.Y));
		
		CellToInstanceMapping.Add(Cell, Index);
	}
}


bool ADummyRobotGroup::FindClosestInstance(FVector InLocation, int32& OutInstanceId, FGridCell2D& OutGridCell, FTransform& OutInstanceTransform)
{
	FVector InverseLocation = GetActorTransform().InverseTransformPosition(InLocation);
	FGridCell2D Cell(FMath::RoundToInt(InverseLocation.X / Spacing.X), FMath::RoundToInt(InverseLocation.Y / Spacing.Y));

	if (CellToInstanceMapping.Contains(Cell))
	{
		OutGridCell = Cell;
		OutInstanceId = CellToInstanceMapping[Cell];
		OutInstanceTransform = FTransform(InstancedMesh->PerInstanceSMData[OutInstanceId].Transform);
		return true;
	}
	else
		return false;
}


void ADummyRobotGroup::OnGridCellChange(const struct FGridCell2D& ChangedGridCell)
{
	if (CellToInstanceMapping.Contains(ChangedGridCell))
	{
		FTransform SpawnTransform(InstancedMesh->PerInstanceSMData[CellToInstanceMapping[ChangedGridCell]].Transform);

		// Remove the static mesh instance on all machines including the server
		RemoveInstance(CellToInstanceMapping[ChangedGridCell]);

		// Spawn the ragdoll on all but dedicated server
		if (GetNetMode() != NM_DedicatedServer)
		{
			FVector Location = GetActorTransform().TransformPosition(SpawnTransform.GetLocation());
			ADummyRobot* NewDummyRobot = GetWorld()->SpawnActor<ADummyRobot>(DummyRobotClass.LoadSynchronous(), Location, FRotator(GetActorTransform().GetRotation()));
			if (NewDummyRobot)
				NewDummyRobot->OnShot(ChangedGridCell.ImpactLocation, ChangedGridCell.ImpactVelocity, NAME_None);
		}
	}
}


void ADummyRobotGroup::OnRep_GridCells()
{
	TArray<int32> InstancesToRemove;
	TArray<FTransform> TransformsToSpawnAt;

	//// Loop over the list of cells that should be removed
	//for (FGridCell2D& Cell : GridCells)
	//{
	//	if (CellToInstanceMapping.Contains(Cell) && CellToInstanceMapping[Cell] > -1)
	//	{
	//		InstancesToRemove.Add(CellToInstanceMapping[Cell]);
	//		TransformsToSpawnAt.Add(FTransform(InstancedMesh->PerInstanceSMData[CellToInstanceMapping[Cell]].Transform));
	//	}
	//}

	//// Now we can remove the instances all together and then spawn the new actors
	//RemoveInstances(InstancesToRemove);
	//
	//// Only spawn the "broken" robots on clients
	//if (GetNetMode() != NM_DedicatedServer)
	//{
	//	for (FTransform& InstanceTransform : TransformsToSpawnAt)
	//	{
	//		FVector Location = GetActorTransform().TransformPosition(InstanceTransform.GetLocation());
	//		ADummyRobot* NewDummyRobot = GetWorld()->SpawnActor<ADummyRobot>(DummyRobotClass.LoadSynchronous(), Location, FRotator(GetActorTransform().GetRotation()));
	//		if (NewDummyRobot)
	//			NewDummyRobot->OnShot(ImpactLocation, ImpactVelocity, BoneName);
	//	}
	//}
}