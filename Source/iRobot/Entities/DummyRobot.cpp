#include "DummyRobot.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/CollisionProfile.h"


ADummyRobot::ADummyRobot()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// This actor is client only
	SetReplicates(false);

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = Scene;

	DynamicMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DynamicMesh"));
	DynamicMesh->SetCollisionProfileName(TEXT("Debris"));
	DynamicMesh->SetMobility(EComponentMobility::Movable);
	DynamicMesh->SetSimulatePhysics(true);
	DynamicMesh->SetupAttachment(RootComponent);
}


void ADummyRobot::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(RemovalHandle, this, &ADummyRobot::RemoveAfterDelay, TimeBeforeRemoval);
}


void ADummyRobot::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}


void ADummyRobot::OnShot(FVector ImpactLocation, FVector ImpactVelocity, FName BoneName)
{
	DynamicMesh->SetSimulatePhysics(true);
	
	// If no bone name was found (non skeletal mesh hit) find the closest now
	if (BoneName == NAME_None)
	{
		FVector TempVec;
		BoneName = DynamicMesh->FindClosestBone(ImpactLocation, &TempVec, 0.f, true);
	}

	DynamicMesh->AddImpulseAtLocation(ImpactVelocity, ImpactLocation, BoneName);
}


void ADummyRobot::RemoveAfterDelay()
{
	this->Destroy();
}