#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DummyRobotGridData.h"
#include "DummyRobotGroup.generated.h"

class ADummyRobot;
class USceneComponent;
class UHierarchicalInstancedStaticMeshComponent;

DECLARE_DELEGATE_OneParam(FGridCellChangeDelegate, const struct FGridCell2D& /*ChangedGridCell*/);

UCLASS()
class IROBOT_API ADummyRobotGroup : public AActor
{
	GENERATED_BODY()
	
public:	
	ADummyRobotGroup();

	FGridCellChangeDelegate* GetGridCellChangedDelegate() { return &GridCellChangedDelegate; }

	/// How many columns of robots (X Axis)
	UPROPERTY(EditInstanceOnly, Category = "DummyRobot|Placement", meta=(ClampMin=1, ClampMax=255))
	int32 NumColumns = 10;

	/// How many rows of robots (Y Axis)
	UPROPERTY(EditInstanceOnly, Category = "DummyRobot|Placement", meta=(ClampMin=1, ClampMax=255))
	int32 NumRows = 10;

	/// How far apart robots are
	UPROPERTY(EditAnywhere, Category = "DummyRobot|Placement")
	FVector2D Spacing = FVector2D(250.f, 250.f);

	/// How long to wait after being shot before removing from the world
	UPROPERTY(EditDefaultsOnly, Category = "DummyRobot|Mesh")
	float TimeBeforeRemoval = 10.f;

	/// Name of the collision profile to use once the dummy robot is turned to debris
	UPROPERTY(EditDefaultsOnly, Category = "DummyRobot|Mesh")
	FName DebrisProfileName = TEXT("Debris");

	/// How long to wait after being shot before removing from the world
	UPROPERTY(EditDefaultsOnly, Category = "DummyRobot|Damage")
	FVector RadialDamageImpactPoint = FVector(0, 0, 50.f);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/// Root component
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	UHierarchicalInstancedStaticMeshComponent* InstancedMesh;

	/// Dynamic mesh of the robot
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	TSoftClassPtr<ADummyRobot> DummyRobotClass;
	
private:

	/// Get the closest instance to the given location
	bool FindClosestInstance(FVector InLocation, int32& OutInstanceId, FGridCell2D& OutGridCell, FTransform& OutInstanceTransform);

	/// Get all instances within the given radius from the given location
	bool FindInstancesInRange(FVector InLocation, float InRadius, TArray<FGridCell2D>& AffectedCells);

	/// Remove the instance/instances from the instanced mesh
	void RemoveInstance(int32 InstanceId);
	void RemoveInstances(TArray<int32> InstanceIds);

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

	/// Delegate fired when a grid cell has changed
	FGridCellChangeDelegate GridCellChangedDelegate;

	/// Data structure to map the given cell to an instance ID
	TMap<FGridCell2D, int32> CellToInstanceMapping;

	bool bGridInitialised = false;
};
