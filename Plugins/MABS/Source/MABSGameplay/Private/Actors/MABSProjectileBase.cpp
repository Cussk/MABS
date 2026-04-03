// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actors/MABSProjectileBase.h"
#include "Components/MABSAbilityComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Data/MABSAbilityDefinition.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"

namespace
{
	FString GetPresentationVisibilityPolicyLabel(const EMABSPresentationCueVisibilityPolicy VisibilityPolicy)
	{
		switch (VisibilityPolicy)
		{
		case EMABSPresentationCueVisibilityPolicy::OwnerOnly:
			return TEXT("OwnerOnly");

		case EMABSPresentationCueVisibilityPolicy::LocalOnly:
			return TEXT("LocalOnly");

		default:
			return TEXT("RelevantClients");
		}
	}

	FString DescribeProjectileTravelCueAssets(const FMABSProjectileTravelCueEvent& CueEvent)
	{
		TArray<FString> AssetLabels;
		if (CueEvent.VFX != nullptr)
		{
			AssetLabels.Add(FString::Printf(TEXT("VFX=%s"), *GetNameSafe(CueEvent.VFX)));
		}
		if (CueEvent.SFX != nullptr)
		{
			AssetLabels.Add(FString::Printf(TEXT("SFX=%s"), *GetNameSafe(CueEvent.SFX)));
		}

		return AssetLabels.IsEmpty() ? TEXT("no presentation assets") : FString::Join(AssetLabels, TEXT(", "));
	}
}

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

void AMABSProjectileBase::OnRep_SourceActor()
{
	ActivateTravelPresentation();
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

bool AMABSProjectileBase::BuildProjectileTravelCueEvent(FMABSProjectileTravelCueEvent& OutCueEvent) const
{
	if (SourceAbilityDefinition == nullptr || !SourceAbilityDefinition->DeliveryPresentation.ProjectileTravel.HasAnyPresentation())
	{
		return false;
	}

	OutCueEvent.AbilityTag = SourceAbilityTag;
	OutCueEvent.AbilityHandle = SourceAbilityHandle;
	OutCueEvent.RuntimeState = EMABSAbilityRuntimeState::Active;
	OutCueEvent.VisibilityPolicy = SourceAbilityDefinition->DeliveryPresentation.ProjectileTravel.VisibilityPolicy;
	OutCueEvent.VFX = SourceAbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelVFX;
	OutCueEvent.SFX = SourceAbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelSFX;
	return true;
}

bool AMABSProjectileBase::ShouldRealizeTravelCueLocally(
	const FMABSProjectileTravelCueEvent& CueEvent,
	bool& bOutShouldRetry,
	FString& OutSkipReason) const
{
	bOutShouldRetry = false;

	if (GetNetMode() == NM_DedicatedServer)
	{
		OutSkipReason = TEXT("Projectile travel cue was skipped because dedicated servers do not realize cosmetic cues.");
		return false;
	}

	switch (CueEvent.VisibilityPolicy)
	{
	case EMABSPresentationCueVisibilityPolicy::OwnerOnly:
	case EMABSPresentationCueVisibilityPolicy::LocalOnly:
		if (GetNetMode() == NM_Standalone)
		{
			return true;
		}

		if (SourceActor == nullptr)
		{
			bOutShouldRetry = true;
			OutSkipReason = TEXT("Projectile travel cue is waiting for the replicated source actor before applying local visibility policy.");
			return false;
		}

		if (IsSourceActorLocallyControlled())
		{
			return true;
		}

		OutSkipReason = FString::Printf(
			TEXT("Projectile travel cue with policy %s was skipped because the source actor is not locally controlled on this instance."),
			*GetPresentationVisibilityPolicyLabel(CueEvent.VisibilityPolicy));
		return false;

	case EMABSPresentationCueVisibilityPolicy::RelevantClients:
	default:
		return true;
	}
}

bool AMABSProjectileBase::IsSourceActorLocallyControlled() const
{
	if (GetNetMode() == NM_Standalone)
	{
		return true;
	}

	if (const APawn* const SourcePawn = Cast<APawn>(SourceActor))
	{
		return SourcePawn->IsLocallyControlled();
	}

	if (const AController* const SourceController = Cast<AController>(SourceActor))
	{
		return SourceController->IsLocalController();
	}

	if (SourceActor != nullptr)
	{
		if (const APawn* const InstigatorPawn = SourceActor->GetInstigator())
		{
			return InstigatorPawn->IsLocallyControlled();
		}

		const AActor* const OuterOwner = SourceActor->GetOwner();
		if (const APawn* const OuterOwnerPawn = Cast<APawn>(OuterOwner))
		{
			return OuterOwnerPawn->IsLocallyControlled();
		}
		if (const AController* const OuterOwnerController = Cast<AController>(OuterOwner))
		{
			return OuterOwnerController->IsLocalController();
		}
	}

	return false;
}

void AMABSProjectileBase::RealizeTravelCueLocally(const FMABSProjectileTravelCueEvent& CueEvent)
{
	if (RootComponent == nullptr)
	{
		return;
	}

	if (CueEvent.VFX != nullptr)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			CueEvent.VFX,
			RootComponent,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true);
	}

	if (CueEvent.SFX != nullptr)
	{
		UGameplayStatics::SpawnSoundAttached(CueEvent.SFX, RootComponent);
	}
}

void AMABSProjectileBase::ActivateTravelPresentation()
{
	if (bTravelPresentationActivated || SourceAbilityDefinition == nullptr)
	{
		return;
	}

	FMABSProjectileTravelCueEvent CueEvent;
	if (!BuildProjectileTravelCueEvent(CueEvent))
	{
		bTravelPresentationActivated = true;
		return;
	}

	bool bShouldRetry = false;
	FString SkipReason;
	if (!ShouldRealizeTravelCueLocally(CueEvent, bShouldRetry, SkipReason))
	{
		if (bShouldRetry)
		{
			return;
		}

		bTravelPresentationActivated = true;
		if (UMABSAbilityComponent* const AbilityComponent = ResolveOwningAbilityComponent())
		{
			AbilityComponent->EmitDebugEvent(
				FName(TEXT("PresentationCueSkipped")),
				CueEvent.AbilityTag,
				CueEvent.AbilityHandle,
				CueEvent.RuntimeState,
				EMABSAbilityActivationResult::Success,
				SkipReason);
		}
		return;
	}

	bTravelPresentationActivated = true;
	RealizeTravelCueLocally(CueEvent);

	if (UMABSAbilityComponent* const AbilityComponent = ResolveOwningAbilityComponent())
	{
		AbilityComponent->EmitDebugEvent(
			FName(TEXT("PresentationCueRealized")),
			CueEvent.AbilityTag,
			CueEvent.AbilityHandle,
			CueEvent.RuntimeState,
			EMABSAbilityActivationResult::Success,
			FString::Printf(
				TEXT("Realized ProjectileTravel cue locally with policy %s. Assets: %s."),
				*GetPresentationVisibilityPolicyLabel(CueEvent.VisibilityPolicy),
				*DescribeProjectileTravelCueAssets(CueEvent)));
	}
}
