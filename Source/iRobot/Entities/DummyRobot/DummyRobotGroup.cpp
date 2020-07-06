#include "DummyRobotGroup.h"
#include "DummyRobot.h"
#include "Engine/SkeletalMesh.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Net/UnrealNetwork.h"
#include "iRobot.h"
#include "DrawDebugHelpers.h"


ADummyRobotGroup::ADummyRobotGroup()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	InstancedMesh = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
	InstancedMesh->SetMobility(EComponentMobility::Movable);
	InstancedMesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	InstancedMesh->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	InstancedMesh->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
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


float ADummyRobotGroup::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (GetLocalRole() != ROLE_Authority)
		return ActualDamage;

	// Needed to get the impulse of the damage
	UDamageType const* const DamageTypeCDO = DamageEvent.DamageTypeClass 
		? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() 
		: GetDefault<UDamageType>();

	// Point damage event
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		FPointDamageEvent* const PointDamageEvent = (FPointDamageEvent*)&DamageEvent;

		FTransform InstanceTransform;
		int32 InstanceId;
		FGridCell2D Cell;
		if (FindClosestInstance(PointDamageEvent->HitInfo.ImpactPoint, InstanceId, Cell, InstanceTransform))
		{
			// Rely on the fact that the array order is correct on server
			int32 CellIndex = (Cell.Column * NumRows) + Cell.Row;
			if (CellIndex < GridCells.Items.Num())
			{
				FDamageEvent EvtRef = DamageEvent;
				GridCells.Items[CellIndex].bOccupied = false;
				GridCells.Items[CellIndex].ImpactLocation = PointDamageEvent->HitInfo.ImpactPoint;
				GridCells.Items[CellIndex].ImpactVelocity = PointDamageEvent->ShotDirection * DamageTypeCDO->DamageImpulse;
				GridCells.MarkItemDirty(GridCells.Items[CellIndex]);

				// Manually trigger the callback on server
				OnGridCellChange(GridCells.Items[CellIndex]);
			}
		}
	}

	// Radial damage event
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		FRadialDamageEvent* const RadialDamageEvent = (FRadialDamageEvent*)&DamageEvent;

		TArray<FGridCell2D> AffectedCells;
		if (FindInstancesInRange(RadialDamageEvent->Origin, RadialDamageEvent->Params.OuterRadius, AffectedCells))
		{
			for (FGridCell2D& Cell : AffectedCells)
			{
				int32 CellIndex = (Cell.Column * NumRows) + Cell.Row;
				if (CellIndex < GridCells.Items.Num())
				{
					FDamageEvent EvtRef = DamageEvent;
					GridCells.Items[CellIndex].bOccupied = false;
					GridCells.Items[CellIndex].ImpactLocation = RadialDamageImpactPoint;

					FTransform InstanceTransform = GetActorTransform() * FTransform(InstancedMesh->PerInstanceSMData[CellIndex].Transform);
					FVector Direction = (InstanceTransform.GetLocation() + RadialDamageImpactPoint - RadialDamageEvent->Origin).GetSafeNormal();
					GridCells.Items[CellIndex].ImpactVelocity = Direction * DamageTypeCDO->DamageImpulse;

					GridCells.MarkItemDirty(GridCells.Items[CellIndex]);

					// Manually trigger the callback on server
					OnGridCellChange(GridCells.Items[CellIndex]);

					DrawDebugPoint(GetWorld(), InstanceTransform.GetLocation(), 20, FColor::Red, false, 5);
				}
			}
		}
	}

	return ActualDamage;
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


bool ADummyRobotGroup::FindInstancesInRange(FVector InLocation, float InRadius, TArray<FGridCell2D>& AffectedCells)
{
	AffectedCells.Empty();

	// First find the cell that the location is in
	FVector InverseLocation = GetActorTransform().InverseTransformPosition(InLocation);
	int32 CellRow    = FMath::Clamp(FMath::RoundToInt(InverseLocation.X / Spacing.X), 0, NumRows - 1);
	int32 CellColumn = FMath::Clamp(FMath::RoundToInt(InverseLocation.Y / Spacing.Y), 0, NumColumns - 1);

	int32 ExtendX = FMath::CeilToInt(InRadius / Spacing.X);
	int32 ExtendY = FMath::CeilToInt(InRadius / Spacing.Y);

	int32 MinRow    = FMath::Clamp(CellRow - ExtendX, 0, NumRows - 1);
	int32 MaxRow    = FMath::Clamp(CellRow + ExtendX, 0, NumRows - 1);
	int32 MinColumn = FMath::Clamp(CellColumn - ExtendY, 0, NumColumns - 1);
	int32 MaxColumn = FMath::Clamp(CellColumn + ExtendY, 0, NumColumns - 1);

	float DistSquareThreshold = InRadius * InRadius;
	InLocation = GetActorTransform().Inverse().TransformPosition(InLocation);

	for (int32 RowIndex = MinRow; RowIndex <= MaxRow; RowIndex++)
	{
		for (int32 ColumnIndex = MinColumn; ColumnIndex <= MaxColumn; ColumnIndex++)
		{
			FGridCell2D AffectedCell(RowIndex, ColumnIndex);
			
			if (CellToInstanceMapping.Contains(AffectedCell))
			{
				FTransform InstanceTransform(InstancedMesh->PerInstanceSMData[CellToInstanceMapping[AffectedCell]].Transform);
				FVector Diff = InstanceTransform.GetLocation() - InLocation;
				if (Diff.SizeSquared() < DistSquareThreshold)
					AffectedCells.Add(AffectedCell);
			}
		}
	}

	return AffectedCells.Num() > 0;
}


void ADummyRobotGroup::OnGridCellChange(const struct FGridCell2D& ChangedGridCell)
{
	if (CellToInstanceMapping.Contains(ChangedGridCell))
	{
		// Grab the transform before removing the instance
		FTransform SpawnTransform(InstancedMesh->PerInstanceSMData[CellToInstanceMapping[ChangedGridCell]].Transform);

		// Remove the static mesh instance on all machines including the server
		RemoveInstance(CellToInstanceMapping[ChangedGridCell]);

		// Spawn the ragdoll on all but dedicated server
		if (GetNetMode() != NM_DedicatedServer)
		{
			FVector Location = GetActorTransform().TransformPosition(SpawnTransform.GetLocation());
			ADummyRobot* NewDummyRobot = GetWorld()->SpawnActor<ADummyRobot>(DummyRobotClass.LoadSynchronous(), Location, FRotator(GetActorTransform().GetRotation()));
			if (NewDummyRobot)
				NewDummyRobot->ApplyImpulse(ChangedGridCell.ImpactLocation, ChangedGridCell.ImpactVelocity);
		}
	}
}


/*void ADummyRobotGroup::OnRep_GridCells()
{
}*/