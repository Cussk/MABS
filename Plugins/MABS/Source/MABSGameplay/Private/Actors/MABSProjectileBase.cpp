// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actors/MABSProjectileBase.h"
#include "Components/MABSAbilityComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Data/MABSAbilityDefinition.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AMABSProjectileBase::AMABSProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(CollisionRadius);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Destructible, ECR_Block);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->OnComponentHit.AddDynamic(this, &AMABSProjectileBase::HandleCollisionHit);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMABSProjectileBase::HandleCollisionOverlap);
	RootComponent = CollisionComponent;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 1600.0f;
	ProjectileMovementComponent->MaxSpeed = 1600.0f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;

	InitialLifeSpan = 5.0f;
}

void AMABSProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	ActivateTravelPresentation();
}

void AMABSProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMABSProjectileBase, SourceActor);
	DOREPLIFETIME(AMABSProjectileBase, SourceAbilityDefinition);
	DOREPLIFETIME(AMABSProjectileBase, SourceAbilityTag);
	DOREPLIFETIME(AMABSProjectileBase, SourceAbilityHandle);
}

void AMABSProjectileBase::InitializeProjectile(
	UMABSAbilityComponent* InOwningAbilityComponent,
	AActor* InSourceActor,
	UMABSAbilityDefinition* InSourceAbilityDefinition,
	const FGameplayTag InSourceAbilityTag,
	const FMABSAbilityHandle InSourceAbilityHandle)
{
	OwningAbilityComponent = InOwningAbilityComponent;
	SourceActor = InSourceActor;
	SourceAbilityDefinition = InSourceAbilityDefinition;
	SourceAbilityTag = InSourceAbilityTag;
	SourceAbilityHandle = InSourceAbilityHandle;
	bImpactHandled = false;

	if (CollisionComponent != nullptr)
	{
		CollisionComponent->SetSphereRadius(CollisionRadius);
		if (InSourceActor != nullptr)
		{
			CollisionComponent->IgnoreActorWhenMoving(InSourceActor, true);
		}
	}

	SetOwner(InSourceActor);
	if (APawn* SourcePawn = Cast<APawn>(InSourceActor))
	{
		SetInstigator(SourcePawn);
	}
	else if (InSourceActor != nullptr)
	{
		SetInstigator(InSourceActor->GetInstigator());
	}
}

void AMABSProjectileBase::OnRep_SourceAbilityDefinition()
{
	ActivateTravelPresentation();
}

USphereComponent* AMABSProjectileBase::GetCollisionComponent() const
{
	return CollisionComponent;
}

UProjectileMovementComponent* AMABSProjectileBase::GetProjectileMovementComponent() const
{
	return ProjectileMovementComponent;
}

AActor* AMABSProjectileBase::GetSourceActor() const
{
	return SourceActor;
}

UMABSAbilityDefinition* AMABSProjectileBase::GetSourceAbilityDefinition() const
{
	return SourceAbilityDefinition;
}

FGameplayTag AMABSProjectileBase::GetSourceAbilityTag() const
{
	return SourceAbilityTag;
}

FMABSAbilityHandle AMABSProjectileBase::GetSourceAbilityHandle() const
{
	return SourceAbilityHandle;
}

void AMABSProjectileBase::HandleCollisionHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	HandleImpact(OtherActor, Hit);
}

void AMABSProjectileBase::HandleCollisionOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	HandleImpact(OtherActor, SweepResult);
}

void AMABSProjectileBase::HandleImpact(AActor* OtherActor, const FHitResult& HitResult)
{
	if (bImpactHandled || !HasAuthority())
	{
		return;
	}

	if (OtherActor == this || OtherActor == SourceActor)
	{
		return;
	}

	bImpactHandled = true;

	UMABSAbilityComponent* const AbilityComponent = ResolveOwningAbilityComponent();
	if (AbilityComponent == nullptr)
	{
		Destroy();
		return;
	}

	const EMABSAbilityActivationResult ImpactResult = AbilityComponent->HandleProjectileImpact(*this, OtherActor, HitResult);
	if (ImpactResult == EMABSAbilityActivationResult::Success || bDestroyOnRejectedImpact)
	{
		Destroy();
	}
	else
	{
		bImpactHandled = false;
	}
}

UMABSAbilityComponent* AMABSProjectileBase::ResolveOwningAbilityComponent() const
{
	if (OwningAbilityComponent.IsValid())
	{
		return OwningAbilityComponent.Get();
	}

	return SourceActor != nullptr ? SourceActor->FindComponentByClass<UMABSAbilityComponent>() : nullptr;
}

void AMABSProjectileBase::ActivateTravelPresentation()
{
	if (bTravelPresentationActivated || SourceAbilityDefinition == nullptr)
	{
		return;
	}

	bTravelPresentationActivated = true;
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (RootComponent == nullptr)
	{
		return;
	}

	if (SourceAbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelVFX != nullptr)
	{
		UGameplayStatics::SpawnEmitterAttached(
			SourceAbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelVFX,
			RootComponent);
	}

	if (SourceAbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelSFX != nullptr)
	{
		UGameplayStatics::SpawnSoundAttached(
			SourceAbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelSFX,
			RootComponent);
	}
}
