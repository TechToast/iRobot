#include "DummyRobotGroup.h"
#include "DummyRobot.h"
#include "Engine/SkeletalMesh.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Net/UnrealNetwork.h"
#include "iRobot.h"
#include "DrawDebugHelpers.h"
#include "Interfaces/IInteraction.h"
#include "Entities/HidingPlace/HidingPlaceData.h"


ADummyRobotGroup::ADummyRobotGroup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

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
					FTransform Trans = FTransform::Identity;// = GetActorTransform();
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
				//FDamageEvent EvtRef = DamageEvent;
				GridCells.Items[CellIndex].bOccupied = false;
				GridCells.Items[CellIndex].ChangeReason = EGridCellChangedReason::Damaged;
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
					//FDamageEvent EvtRef = DamageEvent;
					GridCells.Items[CellIndex].bOccupied = false;
					GridCells.Items[CellIndex].ChangeReason = EGridCellChangedReason::Damaged;
					GridCells.Items[CellIndex].ImpactLocation = RadialDamageImpactPoint;

					FTransform InstanceTransform = GetActorTransform() * CellTransforms[Cell];
					FVector Diff = (InstanceTransform.GetLocation() + RadialDamageImpactPoint - RadialDamageEvent->Origin);
					float DamageScale = RadialDamageEvent->Params.GetDamageScale(Diff.Size());
					GridCells.Items[CellIndex].ImpactVelocity = Diff.GetSafeNormal() * FMath::Lerp(DamageTypeCDO->DamageImpulse * 0.5f, DamageTypeCDO->DamageImpulse, DamageScale);

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
	if (InstancedMesh)
	{
		if (bRestoreDummyRobots)
		{
			FTransform InstanceTransform;
			InstancedMesh->GetInstanceTransform(InstanceId, InstanceTransform);
			QueueRestoredRobot(InstanceTransform);
		}

		InstancedMesh->RemoveInstance(InstanceId);
		CreateCellToInstanceMapping();
	}
}


/*void ADummyRobotGroup::RemoveInstances(TArray<int32> InstanceIds)
{
	InstancedMesh->RemoveInstances(InstanceIds);
	CreateCellToInstanceMapping();
}*/


void ADummyRobotGroup::QueueRestoredRobot(FTransform InTransform)
{
	// TODO: This need to be replaced with a proper manager to handle batches of 
	// robots to be restored

	// For now its a bit of a hack
	FTimerDelegate TimerDel;
	FTimerHandle Handle;
	TimerDel.BindUFunction(this, FName("RestoreInstance"), InTransform);
	GetWorldTimerManager().SetTimer(Handle, TimerDel, TimeBeforeNewRobot, false);
}


void ADummyRobotGroup::RestoreInstance(FTransform Trans)
{
	InstancedMesh->AddInstance(Trans);
	CreateCellToInstanceMapping();
}


void ADummyRobotGroup::GenerateInitialGrid()
{
	if (!bGridInitialised)
	{
		for (int32 Column = 0; Column < NumColumns; ++Column)
		{
			for (int32 Row = 0; Row < NumRows; ++Row)
			{
				FGridCell2D Cell(Row, Column);

				if (GetLocalRole() == ROLE_Authority)
				{
					Cell.bOccupied = true;
					GridCells.MarkItemDirty(GridCells.Items.Add_GetRef(Cell));
				}

				// Cache the transforms separately on server and client as they are fixed and don't need to be replicated
				FTransform Trans = GetActorTransform();
				Trans.SetLocation(FVector(Spacing.X * Row, Spacing.Y * Column, 0.f));
				CellTransforms.Add(Cell, Trans);
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


FGridCell2D ADummyRobotGroup::FindClosestCell(const FVector InLocation) const
{
	FVector InverseLocation = GetActorTransform().InverseTransformPosition(InLocation);
	InverseLocation.X = FMath::Max(0.f, InverseLocation.X);
	InverseLocation.Y = FMath::Max(0.f, InverseLocation.Y);
	return FGridCell2D(FMath::RoundToInt(InverseLocation.X / Spacing.X), FMath::RoundToInt(InverseLocation.Y / Spacing.Y));
}


/*FGridCell2D ADummyRobotGroup::FindClosestUnoccupiedCell(const FVector InLocation) const
{

}


bool ADummyRobotGroup::IsCellOccupied(const FGridCell2D& InCell) const
{
	if (GetLocalRole() != ROLE_Authority)
	{
		checkNoEntry();
		return false;
	}

	int32 CellIndex = (InCell.Column * NumRows) + InCell.Row;
	if (CellIndex < GridCells.Items.Num())
		return GridCells.Items[CellIndex].bOccupied;

	return false;
}*/


bool ADummyRobotGroup::FindClosestInstance(const FVector InLocation, int32& OutInstanceId, FGridCell2D& OutGridCell, FTransform& OutInstanceTransform) const
{
	FGridCell2D Cell = FindClosestCell(InLocation);
	if (CellToInstanceMapping.Contains(Cell))
	{
		OutGridCell = Cell;
		OutInstanceId = CellToInstanceMapping[Cell];
		OutInstanceTransform = CellTransforms[Cell];
		return true;
	}
	else
		return false;
}


bool ADummyRobotGroup::FindInstancesInRange(const FVector InLocation, float InRadius, TArray<FGridCell2D>& AffectedCells) const
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
	FVector ObjectSpaceLocation = GetActorTransform().Inverse().TransformPosition(InLocation);

	for (int32 RowIndex = MinRow; RowIndex <= MaxRow; RowIndex++)
	{
		for (int32 ColumnIndex = MinColumn; ColumnIndex <= MaxColumn; ColumnIndex++)
		{
			FGridCell2D AffectedCell(RowIndex, ColumnIndex);
			
			if (CellToInstanceMapping.Contains(AffectedCell))
			{
				FTransform InstanceTransform(CellTransforms[AffectedCell]);
				FVector Diff = InstanceTransform.GetLocation() - ObjectSpaceLocation;
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
		// Damaged or dissolved
		if (ChangedGridCell.ChangeReason == EGridCellChangedReason::Damaged ||
			ChangedGridCell.ChangeReason == EGridCellChangedReason::Dissolved)
		{
			// Grab the transform before removing the instance
			FTransform SpawnTransform(CellTransforms[ChangedGridCell]);

			// Remove the static mesh instance on all machines including the server
			RemoveInstance(CellToInstanceMapping[ChangedGridCell]);

			// Spawn the ragdoll on all but dedicated server
			if (GetNetMode() != NM_DedicatedServer)
			{
				bool bDamaged = ChangedGridCell.ChangeReason == EGridCellChangedReason::Damaged;
				UClass* ClassToUse = bDamaged ? DummyRobotClass_Damaged.LoadSynchronous() : DummyRobotClass_Dissolved.LoadSynchronous();

				FVector Location = GetActorTransform().TransformPosition(SpawnTransform.GetLocation());
				ADummyRobot* NewDummyRobot = GetWorld()->SpawnActor<ADummyRobot>(ClassToUse, Location, FRotator(GetActorTransform().GetRotation()));
				if (NewDummyRobot && bDamaged)
					NewDummyRobot->ApplyImpulse(ChangedGridCell.ImpactLocation, ChangedGridCell.ImpactVelocity);
			}

		}

		// Respawned
		else if (ChangedGridCell.ChangeReason == EGridCellChangedReason::Respawned)
		{

		}
	}
}


/*void ADummyRobotGroup::OnRep_GridCells()
{
}*/


/*void ADummyRobotGroup::OnInteraction(IInteractor* InInstigator, FHitResult& InteractionHit)
{
	// Only allow interaction on server
	if (GetLocalRole() != ROLE_Authority)
		return;

	if (InInstigator && InInstigator->HasInteractionCabability(INTERACTION_CAPABILITY_DissolveRobot))
	{
		FTransform InstanceTransform;
		int32 InstanceId;
		FGridCell2D Cell;
		if (FindClosestInstance(InteractionHit.ImpactPoint, InstanceId, Cell, InstanceTransform))
		{
			// Rely on the fact that the array order is correct on server
			int32 CellIndex = (Cell.Column * NumRows) + Cell.Row;
			if (CellIndex < GridCells.Items.Num())
			{
				GridCells.Items[CellIndex].bOccupied = false;
				GridCells.Items[CellIndex].ChangeReason = EGridCellChangedReason::Dissolved;
				GridCells.MarkItemDirty(GridCells.Items[CellIndex]);

				// Manually trigger the callback on server
				OnGridCellChange(GridCells.Items[CellIndex]);
			}
		}
	}
}*/


bool ADummyRobotGroup::IsWithinRange(FVector InLocation, float InTolerance)
{
	FVector InverseLocation = GetActorTransform().InverseTransformPosition(InLocation);
	if (InverseLocation.X > -InTolerance
		&& InverseLocation.Y > -InTolerance
		&& InverseLocation.X < ((NumRows * Spacing.X) + InTolerance)
		&& InverseLocation.Y < ((NumColumns * Spacing.Y) + InTolerance))
	{
		return true;
	}

	return false;
}



bool ADummyRobotGroup::GetHidingPlaceTransform(FVector InLocation, FTransform& OutTransform)
{
	FGridCell2D Cell = FindClosestCell(InLocation);

	OutTransform.SetLocation(FVector(Cell.Row * Spacing.X, Cell.Column * Spacing.Y, 0.f));
	OutTransform = OutTransform * GetActorTransform();

	FQuat Rot = OutTransform.GetRotation();
	Rot = DummyRobotLocalRotation.Quaternion() * Rot;
	OutTransform.SetRotation(Rot);

	return true;
}


void ADummyRobotGroup::PrepareHidingPlace(FVector InLocation)
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	FTransform InstanceTransform;
	int32 InstanceId;
	FGridCell2D Cell;
	if (FindClosestInstance(InLocation, InstanceId, Cell, InstanceTransform))
	{
		// Rely on the fact that the array order is correct on server
		int32 CellIndex = (Cell.Column * NumRows) + Cell.Row;
		if (CellIndex < GridCells.Items.Num())
		{
			GridCells.Items[CellIndex].bOccupied = false;
			GridCells.Items[CellIndex].ChangeReason = EGridCellChangedReason::Dissolved;
			GridCells.MarkItemDirty(GridCells.Items[CellIndex]);

			// Manually trigger the callback on server
			OnGridCellChange(GridCells.Items[CellIndex]);

			// TODO: Get the dissolve duration from a shared datasource somewhere...
			GetWorld()->GetTimerManager().SetTimer(OnHidingPlaceReadyTimer, this, &ADummyRobotGroup::HidingPlaceReadyPostDelay, 0.25f);
			return;
		}
	}

	// If the hiding place is already prepared, just trigger the delegate immediately
	HidingPlaceReadyPostDelay();
	return;
}


void ADummyRobotGroup::HidingPlaceReadyPostDelay()
{
	if (OnHidingPlaceReady.IsBound())
		OnHidingPlaceReady.Broadcast();
}