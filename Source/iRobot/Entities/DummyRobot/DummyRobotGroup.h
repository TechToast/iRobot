#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DummyRobotGridData.h"
#include "Interfaces/IScannable.h"
#include "Entities/HidingPlace/IHideCompatible.h"
#include "DummyRobotGroup.generated.h"

class ADummyRobot;
class USceneComponent;
class UHierarchicalInstancedStaticMeshComponent;
class USoundCue;

DECLARE_DELEGATE_OneParam(FGridCellChangeDelegate, const struct FGridCell2D& /*ChangedGridCell*/);

UCLASS()
class IROBOT_API ADummyRobotGroup : public AActor, public IHideCompatible, public IScannable
{
	GENERATED_BODY()
	
public:	
	ADummyRobotGroup();

	FGridCellChangeDelegate* GetGridCellChangedDelegate() { return &GridCellChangedDelegate; }

	/// Start AHidingPlace interface
	virtual EHidingPlaceType GetHidingPlaceType() override { return EHidingPlaceType::HP_Freeze; }
	virtual bool IsWithinRange(FVector InLocation, float InTolerance) override;
	virtual bool GetHidingPlaceTransform(FVector InLocation, FTransform& OutTransform) override;
	virtual void PrepareHidingPlace(FVector InLocation) override;
	virtual FHidingPlaceReadyEvent* GetOnHidingPlaceReadyEvent() override { return &OnHidingPlaceReady; }
	virtual void OccupyHidingPlace(FVector InLocation);
	virtual void VacateHidingPlace(FVector InLocation);
	virtual bool IsHidingPlaceOccupied(FVector InLocation) override;
	/// End AHidingPlace interface

	/// Start IScannable interface
	virtual void OnScanned(int32 ScannedIndex);
	virtual int32 GetScanIndex(const FVector& ScanHitLocation);
	/// End IScannable interface

	/// How many columns of robots (X Axis)
	UPROPERTY(EditInstanceOnly, Category = "DummyRobot|Placement", meta=(ClampMin=1, ClampMax=255))
	int32 NumColumns = 10;

	/// How many rows of robots (Y Axis)
	UPROPERTY(EditInstanceOnly, Category = "DummyRobot|Placement", meta=(ClampMin=1, ClampMax=255))
	int32 NumRows = 10;

	/// How far apart robots are
	UPROPERTY(EditAnywhere, Category = "DummyRobot|Placement")
	FVector2D Spacing = FVector2D(250.f, 250.f);

	/// The local rotation of a dummy robot in the group
	UPROPERTY(EditDefaultsOnly, Category = "DummyRobot|Placement")
	FRotator DummyRobotLocalRotation = FRotator(0, 180, 0);

	/// How long to wait after being shot before removing from the world
	UPROPERTY(EditDefaultsOnly, Category = "DummyRobot|Mesh")
	float TimeBeforeRemoval = 10.f;

	/// Should robots be restored after being damaged
	UPROPERTY(EditDefaultsOnly, Category = "DummyRobot|Mesh")
	bool bRestoreDummyRobots = true;

	/// How long to wait after being shot before creating a replacement robot
	UPROPERTY(EditDefaultsOnly, Category = "DummyRobot|Mesh", meta=(EditCondition="bRestoreDummyRobots"))
	float TimeBeforeNewRobot = 25.f;

	/// How long to wait before restoring the scanned state
	UPROPERTY(EditDefaultsOnly, Category = "DummyRobot|Mesh")
	float TimeBeforeScanStateRestored = 5.f;

	/// Name of the collision profile to use once the dummy robot is turned to debris
	UPROPERTY(EditDefaultsOnly, Category = "DummyRobot|Mesh")
	FName DebrisProfileName = TEXT("Debris");

	/// How long to wait after being shot before removing from the world
	UPROPERTY(EditDefaultsOnly, Category = "DummyRobot|Damage")
	FVector RadialDamageImpactPoint = FVector(0, 0, 50.f);

	/// Start IInteractable interface
	//virtual void OnInteraction(IInteractor* InInstigator, FHitResult& InteractionHit) override;
	/// End IInteractable interface

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/// Root component
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	UHierarchicalInstancedStaticMeshComponent* InstancedMesh;

	/// Dummy robot to use when damaged
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	TSoftClassPtr<ADummyRobot> DummyRobotClass_Damaged;
	
	/// Dummy robot to use when dissolved
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	TSoftClassPtr<ADummyRobot> DummyRobotClass_Dissolved;

	/// Played when a robot is scanned?
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* ScannedSound;

private:

	/// Find the closest cell to the given location
	bool FindClosestCell(const FVector InLocation, FGridCell2D& OutCell) const;

	/// Find the closest cell to the given location which is not currently occupied
	//FGridCell2D FindClosestUnoccupiedCell(const FVector InLocation) const;

	/// Get the closest instance to the given location
	bool FindClosestInstance(const FVector InLocation, int32& OutInstanceId, FGridCell2D& OutGridCell, FTransform& OutInstanceTransform) const;

	/// Get all instances within the given radius from the given location
	bool FindInstancesInRange(const FVector InLocation, const float InRadius, TArray<FGridCell2D>& AffectedCells) const;

	/// Is the given cell currently occupied by a dummy robot
	//bool IsCellOccupied(const FGridCell2D& InCell) const;

	/// Remove the instance/instances from the instanced mesh
	void RemoveInstance(int32 InstanceId);
	//void RemoveInstances(TArray<int32> InstanceIds);

	/// Add the given transform to a queue to be restored after a delay
	void QueueRestoredRobot(FTransform InTransform);
	void QueueUnscannedRobot(int32 InstanceIndex);

	UFUNCTION()
	void RestoreInstance(FTransform Trans);
	UFUNCTION()
	void RestoreScanState(int32 InstanceIndex);

	/// Create a mapping table of cells to instances
	void CreateCellToInstanceMapping();

	/// Generate the replicated grid
	void GenerateInitialGrid();

	/// Replicated list of grid cells that have been removed
	UPROPERTY(Replicated/*Using=OnRep_GridCells*/)
	FGridDataArray GridCells;

	/// Called when GridCells is replicated
	/*UFUNCTION()
	void OnRep_GridCells();*/

	/// Called when the a grid cell changes
	UFUNCTION()
	void OnGridCellChange(const struct FGridCell2D& ChangedGridCell);

	/// Called when the hiding place has been prepared
	void HidingPlaceReadyPostDelay();

	/// Delegate fired when a grid cell has changed
	FGridCellChangeDelegate GridCellChangedDelegate;

	/// Data structure to map the given cell to an instance ID
	TMap<FGridCell2D, int32> CellToInstanceMapping;
	TMap<int32, FGridCell2D> IndexToCellMapping;

	/// List of currently occupied hiding place cells
	TSet<FGridCell2D> OccupiedHidingPlaceCells;

	/// Cache the transforms of each grid cell
	TMap<FGridCell2D, FTransform> CellTransforms;

	bool bGridInitialised = false;

	FHidingPlaceReadyEvent OnHidingPlaceReady;

	FTimerHandle OnHidingPlaceReadyTimer;
};
