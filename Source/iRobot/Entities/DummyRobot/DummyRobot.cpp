#include "DummyRobot.h"
#include "iRobot.h"
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
	DynamicMesh->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	DynamicMesh->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
}


void ADummyRobot::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(RemovalHandle, this, &ADummyRobot::RemoveAfterDelay, TimeBeforeRemoval);

	// Hook up delegates for taking damage
	PointDelegate.BindUFunction(this, FName(TEXT("OnPointDamageReceived")));
	OnTakePointDamage.Add(PointDelegate);

	RadialDelegate.BindUFunction(this, FName(TEXT("OnRadialDamageReceived")));
	OnTakeRadialDamage.Add(RadialDelegate);
}


void ADummyRobot::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	OnTakePointDamage.Remove(PointDelegate);
	OnTakeRadialDamage.Remove(RadialDelegate);
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
	this->Destroy();
}