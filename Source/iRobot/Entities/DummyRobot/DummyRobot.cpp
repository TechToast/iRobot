#include "DummyRobot.h"
#include "iRobot.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/CollisionProfile.h"


ADummyRobot::ADummyRobot()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	// This actor is client only
	SetReplicates(false);

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = Scene;

	DynamicMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DynamicMesh"));
	DynamicMesh->SetCollisionProfileName(TEXT("Debris"));
	DynamicMesh->SetMobility(EComponentMobility::Movable);
	DynamicMesh->SetSimulatePhysics(true);
	DynamicMesh->SetupAttachment(RootComponent);
	DynamicMesh->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	DynamicMesh->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
}


void ADummyRobot::BeginPlay()
{
	Super::BeginPlay();

	// Hook up delegates for taking damage
	PointDelegate.BindUFunction(this, FName(TEXT("OnPointDamageReceived")));
	OnTakePointDamage.Add(PointDelegate);

	RadialDelegate.BindUFunction(this, FName(TEXT("OnRadialDamageReceived")));
	OnTakeRadialDamage.Add(RadialDelegate);

	// Create dynamic material instances
	if (DynamicMesh)
	{
		for (int32 Index = 0; Index < DynamicMesh->GetNumMaterials(); Index++)
		{
			Materials.Add(DynamicMesh->CreateDynamicMaterialInstance(Index));
		}
	}

	if (!TimeBeforeRemovalRange.IsZero())
		GetWorld()->GetTimerManager().SetTimer(RemovalHandle, this, &ADummyRobot::RemoveAfterDelay, FMath::RandRange(TimeBeforeRemovalRange.X, TimeBeforeRemovalRange.Y));
	else
		RemoveAfterDelay();
}


void ADummyRobot::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	OnTakePointDamage.Remove(PointDelegate);
	OnTakeRadialDamage.Remove(RadialDelegate);
}


void ADummyRobot::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ElapsedDissolveTime >= DissolveTime)
	{
		this->Destroy();
	}
	else
	{
		ElapsedDissolveTime += DeltaSeconds;

		float DissolveAmount = ElapsedDissolveTime / DissolveTime;

		if (DissolveParamName != NAME_None)
		{
			for (auto DynMat : Materials)
			{
				if (DynMat.IsValid())
				{
					DynMat->SetScalarParameterValue(DissolveParamName, DissolveAmount);
				}
			}
		}
	}
}


void ADummyRobot::ApplyImpulse(FVector ImpactLocation, FVector ImpactVelocity, FName BoneName)
{
	// If no bone name was found (non skeletal mesh hit) find the closest now
	if (BoneName == NAME_None)
	{
		FVector TempVec;
		BoneName = DynamicMesh->FindClosestBone(ImpactLocation, &TempVec, 0.f, true);
	}

	DynamicMesh->AddImpulseAtLocation(ImpactVelocity, ImpactLocation, BoneName);
}


void ADummyRobot::OnPointDamageReceived(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation,
	UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType,
	AActor* DamageCauser)
{
	ApplyImpulse(HitLocation, ShotFromDirection.GetSafeNormal() * DamageType->DamageImpulse, BoneName);
}


void ADummyRobot::OnRadialDamageReceived(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	FVector Origin, FHitResult HitInfo, AController* InstigatedBy,
	AActor* DamageCauser)
{
	// TODO
}


void ADummyRobot::RemoveAfterDelay()
{
	// Prevent any further replication
	if (GetLocalRole() == ROLE_Authority)
		TearOff();

	// We can now delete this actor on dedicated server
	if (GetNetMode() == NM_DedicatedServer)
	{
		this->Destroy();
	}

	// On clients we need to dissolve it
	else
	{
		DynamicMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		ElapsedDissolveTime = 0;
		SetActorTickEnabled(true);
	}

}