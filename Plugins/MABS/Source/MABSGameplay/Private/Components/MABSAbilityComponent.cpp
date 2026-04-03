// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/MABSAbilityComponent.h"
#include "Actors/MABSProjectileBase.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraShakeBase.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "Data/MABSAbilityDefinition.h"
#include "Data/MABSAbilitySet.h"
#include "Debug/MABSAbilitySystemLogs.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/MABSCostReceiver.h"
#include "Interfaces/MABSInstantEffectReceiver.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

namespace MABSAbilityComponentEventNames
{
	static const FName AbilityBlocked(TEXT("AbilityBlocked"));
	static const FName AbilityGranted(TEXT("AbilityGranted"));
	static const FName AbilityGrantRejected(TEXT("AbilityGrantRejected"));
	static const FName AbilitySetGranted(TEXT("AbilitySetGranted"));
	static const FName AbilitySetGrantFailed(TEXT("AbilitySetGrantFailed"));
	static const FName AbilitySetGrantSkipped(TEXT("AbilitySetGrantSkipped"));
	static const FName AbilityUnblocked(TEXT("AbilityUnblocked"));
	static const FName CommitSucceeded(TEXT("CommitSucceeded"));
	static const FName CooldownRejected(TEXT("CooldownRejected"));
	static const FName CooldownStarted(TEXT("CooldownStarted"));
	static const FName ComboQueued(TEXT("ComboQueued"));
	static const FName ComboRejected(TEXT("ComboRejected"));
	static const FName ComboWindowEnded(TEXT("ComboWindowEnded"));
	static const FName ComboWindowStarted(TEXT("ComboWindowStarted"));
	static const FName DeliveryFailed(TEXT("DeliveryFailed"));
	static const FName DeliveryStarted(TEXT("DeliveryStarted"));
	static const FName EffectApplied(TEXT("EffectApplied"));
	static const FName EffectApplicationFailed(TEXT("EffectApplicationFailed"));
	static const FName AoEResolved(TEXT("AoEResolved"));
	static const FName AoETargetRejected(TEXT("AoETargetRejected"));
	static const FName CostRejected(TEXT("CostRejected"));
	static const FName CostSpent(TEXT("CostSpent"));
	static const FName CostValidated(TEXT("CostValidated"));
	static const FName HitTraceHit(TEXT("HitTraceHit"));
	static const FName HitTraceRejected(TEXT("HitTraceRejected"));
	static const FName MeleeHit(TEXT("MeleeHit"));
	static const FName MeleeRejected(TEXT("MeleeRejected"));
	static const FName MontagePlayFailed(TEXT("MontagePlayFailed"));
	static const FName MontagePlayRequested(TEXT("MontagePlayRequested"));
	static const FName PresentationAssetMissing(TEXT("PresentationAssetMissing"));
	static const FName PresentationCuePolicyFallbackUsed(TEXT("PresentationCuePolicyFallbackUsed"));
	static const FName PresentationCueRealized(TEXT("PresentationCueRealized"));
	static const FName PresentationCueRouted(TEXT("PresentationCueRouted"));
	static const FName PresentationCueSkipped(TEXT("PresentationCueSkipped"));
	static const FName PresentationSocketFallbackUsed(TEXT("PresentationSocketFallbackUsed"));
	static const FName ProjectileImpact(TEXT("ProjectileImpact"));
	static const FName ProjectileImpactRejected(TEXT("ProjectileImpactRejected"));
	static const FName ProjectileTravelPresentationTriggered(TEXT("ProjectileTravelPresentationTriggered"));
	static const FName ProjectileSpawned(TEXT("ProjectileSpawned"));
	static const FName ProjectileSpawnFailed(TEXT("ProjectileSpawnFailed"));
	static const FName PeriodicEffectApplied(TEXT("PeriodicEffectApplied"));
	static const FName PeriodicEffectExpired(TEXT("PeriodicEffectExpired"));
	static const FName PeriodicEffectRefreshed(TEXT("PeriodicEffectRefreshed"));
	static const FName PeriodicEffectTick(TEXT("PeriodicEffectTick"));
	static const FName RecoveryCompleted(TEXT("RecoveryCompleted"));
	static const FName RecoveryStarted(TEXT("RecoveryStarted"));
	static const FName RequestAccepted(TEXT("RequestAccepted"));
	static const FName RequestRejected(TEXT("RequestRejected"));
	static const FName RequestSentToServer(TEXT("RequestSentToServer"));
	static const FName RequestStarted(TEXT("RequestStarted"));
	static const FName SocketFallbackUsed(TEXT("SocketFallbackUsed"));
	static const FName SocketResolved(TEXT("SocketResolved"));
	static const FName StartupStarted(TEXT("StartupStarted"));
	static const FName StartupPresentationTriggered(TEXT("StartupPresentationTriggered"));
	static const FName TargetTraceHit(TEXT("TargetTraceHit"));
	static const FName TargetTraceRejected(TEXT("TargetTraceRejected"));
	static const FName TargetTraceStarted(TEXT("TargetTraceStarted"));
	static const FName TargetResolved(TEXT("TargetResolved"));
	static const FName TargetResolutionFailed(TEXT("TargetResolutionFailed"));
	static const FName DeliveryScheduled(TEXT("DeliveryScheduled"));
	static const FName DeliveryTriggered(TEXT("DeliveryTriggered"));
	static const FName DeliveryPresentationTriggered(TEXT("DeliveryPresentationTriggered"));
	static const FName ImpactPresentationTriggered(TEXT("ImpactPresentationTriggered"));
	static const FName TracerCueRealized(TEXT("TracerCueRealized"));
	static const FName TracerCueRouted(TEXT("TracerCueRouted"));
	static const FName TracerCueSkipped(TEXT("TracerCueSkipped"));
	static const FName TracerSpawned(TEXT("TracerSpawned"));
	static const FName TracerSpawnFailed(TEXT("TracerSpawnFailed"));
}

namespace
{
	FString DescribeActivationResult(const EMABSAbilityActivationResult ActivationResult)
	{
		switch (ActivationResult)
		{
		case EMABSAbilityActivationResult::InvalidAbility:
			return TEXT("Activation rejected because the requested ability was invalid.");

		case EMABSAbilityActivationResult::ComboRejected:
			return TEXT("Activation rejected because the requested combo follow-up was not valid.");

		case EMABSAbilityActivationResult::NotGranted:
			return TEXT("Activation rejected because the ability has not been granted.");

		case EMABSAbilityActivationResult::AlreadyActive:
			return TEXT("Activation rejected because the ability is already active.");

		case EMABSAbilityActivationResult::ComboQueued:
			return TEXT("Activation was accepted and queued as a combo follow-up.");

		case EMABSAbilityActivationResult::Blocked:
			return TEXT("Activation rejected because the ability is currently blocked.");

		case EMABSAbilityActivationResult::OnCooldown:
			return TEXT("Activation rejected because the ability is on cooldown.");

		case EMABSAbilityActivationResult::InsufficientResources:
			return TEXT("Activation rejected because the owner cannot afford the resource cost.");

		case EMABSAbilityActivationResult::AuthorityRejected:
			return TEXT("Activation rejected because the request must be handled by authority.");

		case EMABSAbilityActivationResult::DeliveryFailed:
			return TEXT("Activation rejected because the authored delivery step failed.");

		case EMABSAbilityActivationResult::TargetResolutionFailed:
			return TEXT("Activation rejected because no valid target could be resolved.");

		case EMABSAbilityActivationResult::EffectApplicationFailed:
			return TEXT("Activation rejected because the instant effect could not be applied.");

		default:
			return TEXT("Activation rejected.");
		}
	}

	FString GetTargetTypeLabel(const EMABSTargetType TargetType)
	{
		switch (TargetType)
		{
		case EMABSTargetType::Self:
			return TEXT("Self");

		case EMABSTargetType::Actor:
			return TEXT("Actor");

		case EMABSTargetType::Location:
			return TEXT("Location");

		default:
			return TEXT("None");
		}
	}

	FString GetDeliveryModeLabel(const EMABSDeliveryMode DeliveryMode)
	{
		switch (DeliveryMode)
		{
		case EMABSDeliveryMode::HitTrace:
			return TEXT("HitTrace");

		case EMABSDeliveryMode::Melee:
			return TEXT("Melee");

		case EMABSDeliveryMode::Projectile:
			return TEXT("Projectile");

		default:
			return TEXT("Direct");
		}
	}

	FString GetRuntimeStateLabel(const EMABSAbilityRuntimeState RuntimeState)
	{
		switch (RuntimeState)
		{
		case EMABSAbilityRuntimeState::Startup:
			return TEXT("Startup");

		case EMABSAbilityRuntimeState::Active:
			return TEXT("Active");

		case EMABSAbilityRuntimeState::Recovery:
			return TEXT("Recovery");

		case EMABSAbilityRuntimeState::Blocked:
			return TEXT("Blocked");

		case EMABSAbilityRuntimeState::Idle:
			return TEXT("Idle");

		default:
			return TEXT("None");
		}
	}

	FString GetPresentationCuePhaseLabel(const EMABSPresentationCuePhase CuePhase)
	{
		switch (CuePhase)
		{
		case EMABSPresentationCuePhase::Delivery:
			return TEXT("Delivery");

		case EMABSPresentationCuePhase::Tracer:
			return TEXT("Tracer");

		case EMABSPresentationCuePhase::ProjectileTravel:
			return TEXT("ProjectileTravel");

		case EMABSPresentationCuePhase::Impact:
			return TEXT("Impact");

		default:
			return TEXT("Startup");
		}
	}

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

	FString GetTargetTraceModeLabel(const EMABSTargetTraceMode TraceMode)
	{
		switch (TraceMode)
		{
		case EMABSTargetTraceMode::Sphere:
			return TEXT("Sphere");

		default:
			return TEXT("Line");
		}
	}

	FString GetInstantEffectTypeLabel(const EMABSInstantEffectType EffectType)
	{
		switch (EffectType)
		{
		case EMABSInstantEffectType::Damage:
			return TEXT("Damage");

		case EMABSInstantEffectType::Heal:
			return TEXT("Heal");

		default:
			return TEXT("None");
		}
	}

	FString GetPeriodicEffectTypeLabel(const EMABSPeriodicEffectType EffectType)
	{
		switch (EffectType)
		{
		case EMABSPeriodicEffectType::HOT:
			return TEXT("HOT");

		default:
			return TEXT("DOT");
		}
	}

	FString GetAoEShapeLabel(const EMABSAoEShape Shape)
	{
		switch (Shape)
		{
		case EMABSAoEShape::Box:
			return TEXT("Box");

		case EMABSAoEShape::Capsule:
			return TEXT("Capsule");

		default:
			return TEXT("Sphere");
		}
	}

	EMABSDebugEventCategory GetDebugEventCategory(const FName EventName)
	{
		if (EventName == MABSAbilityComponentEventNames::AbilityGranted
			|| EventName == MABSAbilityComponentEventNames::AbilityGrantRejected
			|| EventName == MABSAbilityComponentEventNames::AbilitySetGranted
			|| EventName == MABSAbilityComponentEventNames::AbilitySetGrantFailed
			|| EventName == MABSAbilityComponentEventNames::AbilitySetGrantSkipped)
		{
			return EMABSDebugEventCategory::General;
		}

		if (EventName == MABSAbilityComponentEventNames::RequestStarted
			|| EventName == MABSAbilityComponentEventNames::RequestAccepted
			|| EventName == MABSAbilityComponentEventNames::RequestRejected
			|| EventName == MABSAbilityComponentEventNames::RequestSentToServer
			|| EventName == MABSAbilityComponentEventNames::StartupStarted
			|| EventName == MABSAbilityComponentEventNames::AbilityBlocked
			|| EventName == MABSAbilityComponentEventNames::AbilityUnblocked)
		{
			return EMABSDebugEventCategory::Activation;
		}

		if (EventName == MABSAbilityComponentEventNames::TargetTraceStarted
			|| EventName == MABSAbilityComponentEventNames::TargetTraceHit
			|| EventName == MABSAbilityComponentEventNames::TargetTraceRejected
			|| EventName == MABSAbilityComponentEventNames::TargetResolved
			|| EventName == MABSAbilityComponentEventNames::TargetResolutionFailed
			|| EventName == MABSAbilityComponentEventNames::SocketResolved
			|| EventName == MABSAbilityComponentEventNames::SocketFallbackUsed)
		{
			return EMABSDebugEventCategory::Targeting;
		}

		if (EventName == MABSAbilityComponentEventNames::DeliveryScheduled
			|| EventName == MABSAbilityComponentEventNames::DeliveryTriggered
			|| EventName == MABSAbilityComponentEventNames::DeliveryStarted
			|| EventName == MABSAbilityComponentEventNames::DeliveryFailed
			|| EventName == MABSAbilityComponentEventNames::HitTraceHit
			|| EventName == MABSAbilityComponentEventNames::HitTraceRejected
			|| EventName == MABSAbilityComponentEventNames::MeleeHit
			|| EventName == MABSAbilityComponentEventNames::MeleeRejected
			|| EventName == MABSAbilityComponentEventNames::ProjectileSpawned
			|| EventName == MABSAbilityComponentEventNames::ProjectileSpawnFailed
			|| EventName == MABSAbilityComponentEventNames::ProjectileImpact
			|| EventName == MABSAbilityComponentEventNames::ProjectileImpactRejected
			|| EventName == MABSAbilityComponentEventNames::AoEResolved
			|| EventName == MABSAbilityComponentEventNames::AoETargetRejected
			|| EventName == MABSAbilityComponentEventNames::EffectApplied
			|| EventName == MABSAbilityComponentEventNames::EffectApplicationFailed
			|| EventName == MABSAbilityComponentEventNames::CommitSucceeded
			|| EventName == MABSAbilityComponentEventNames::RecoveryStarted
			|| EventName == MABSAbilityComponentEventNames::RecoveryCompleted)
		{
			return EMABSDebugEventCategory::Delivery;
		}

		if (EventName == MABSAbilityComponentEventNames::CooldownRejected
			|| EventName == MABSAbilityComponentEventNames::CooldownStarted
			|| EventName == MABSAbilityComponentEventNames::CostRejected
			|| EventName == MABSAbilityComponentEventNames::CostSpent
			|| EventName == MABSAbilityComponentEventNames::CostValidated)
		{
			return EMABSDebugEventCategory::CostCooldown;
		}

		if (EventName == MABSAbilityComponentEventNames::ComboQueued
			|| EventName == MABSAbilityComponentEventNames::ComboRejected
			|| EventName == MABSAbilityComponentEventNames::ComboWindowStarted
			|| EventName == MABSAbilityComponentEventNames::ComboWindowEnded)
		{
			return EMABSDebugEventCategory::Combo;
		}

		if (EventName == MABSAbilityComponentEventNames::PeriodicEffectApplied
			|| EventName == MABSAbilityComponentEventNames::PeriodicEffectRefreshed
			|| EventName == MABSAbilityComponentEventNames::PeriodicEffectTick
			|| EventName == MABSAbilityComponentEventNames::PeriodicEffectExpired)
		{
			return EMABSDebugEventCategory::Periodic;
		}

		if (EventName == MABSAbilityComponentEventNames::MontagePlayRequested
			|| EventName == MABSAbilityComponentEventNames::MontagePlayFailed
			|| EventName == MABSAbilityComponentEventNames::PresentationAssetMissing
			|| EventName == MABSAbilityComponentEventNames::PresentationCuePolicyFallbackUsed
			|| EventName == MABSAbilityComponentEventNames::PresentationCueRealized
			|| EventName == MABSAbilityComponentEventNames::PresentationCueRouted
			|| EventName == MABSAbilityComponentEventNames::PresentationCueSkipped
			|| EventName == MABSAbilityComponentEventNames::PresentationSocketFallbackUsed
			|| EventName == MABSAbilityComponentEventNames::ProjectileTravelPresentationTriggered
			|| EventName == MABSAbilityComponentEventNames::StartupPresentationTriggered
			|| EventName == MABSAbilityComponentEventNames::DeliveryPresentationTriggered
			|| EventName == MABSAbilityComponentEventNames::ImpactPresentationTriggered
			|| EventName == MABSAbilityComponentEventNames::TracerCueRealized
			|| EventName == MABSAbilityComponentEventNames::TracerCueRouted
			|| EventName == MABSAbilityComponentEventNames::TracerCueSkipped
			|| EventName == MABSAbilityComponentEventNames::TracerSpawned
			|| EventName == MABSAbilityComponentEventNames::TracerSpawnFailed)
		{
			return EMABSDebugEventCategory::Presentation;
		}

		return EMABSDebugEventCategory::General;
	}

	FString GetAbilityLabel(const UMABSAbilityDefinition* AbilityDefinition)
	{
		if (AbilityDefinition == nullptr)
		{
			return TEXT("InvalidAbility");
		}

		return AbilityDefinition->DisplayName.IsEmpty()
			? GetNameSafe(AbilityDefinition)
			: AbilityDefinition->DisplayName.ToString();
	}

	FText GetAbilityDebugDisplayName(const UMABSAbilityDefinition* AbilityDefinition)
	{
		if (AbilityDefinition == nullptr)
		{
			return FText::FromString(TEXT("InvalidAbility"));
		}

		return AbilityDefinition->DisplayName.IsEmpty()
			? FText::FromString(GetNameSafe(AbilityDefinition))
			: AbilityDefinition->DisplayName;
	}

	FString GetAbilitySetLabel(const UMABSAbilitySet* AbilitySet)
	{
		return AbilitySet != nullptr ? GetNameSafe(AbilitySet) : TEXT("InvalidAbilitySet");
	}

	FString FormatVectorForDebug(const FVector& Vector)
	{
		return FString::Printf(TEXT("(X=%.0f Y=%.0f Z=%.0f)"), Vector.X, Vector.Y, Vector.Z);
	}

	FString DescribeActorList(const TArray<AActor*>& Actors)
	{
		TArray<FString> ActorLabels;
		ActorLabels.Reserve(Actors.Num());
		for (AActor* const Actor : Actors)
		{
			ActorLabels.Add(GetNameSafe(Actor));
		}

		return ActorLabels.IsEmpty() ? TEXT("None") : FString::Join(ActorLabels, TEXT(", "));
	}

	FString GetHitActorLabel(const AActor* Actor)
	{
		return Actor != nullptr ? GetNameSafe(Actor) : TEXT("World");
	}

	FString DescribeCueAssets(const FMABSPresentationCueData& CueData)
	{
		TArray<FString> AssetLabels;
		if (CueData.VFX != nullptr)
		{
			AssetLabels.Add(FString::Printf(TEXT("VFX=%s"), *GetNameSafe(CueData.VFX)));
		}
		if (CueData.SFX != nullptr)
		{
			AssetLabels.Add(FString::Printf(TEXT("SFX=%s"), *GetNameSafe(CueData.SFX)));
		}
		if (CueData.CameraShake.HasCameraShake())
		{
			AssetLabels.Add(FString::Printf(TEXT("CameraShake=%s"), *GetNameSafe(CueData.CameraShake.CameraShakeClass)));
		}

		return AssetLabels.IsEmpty() ? TEXT("no presentation assets") : FString::Join(AssetLabels, TEXT(", "));
	}

	FString DescribeCueEventAssets(const FMABSPresentationCueEvent& CueEvent)
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
		if (CueEvent.CameraShakeClass != nullptr)
		{
			AssetLabels.Add(FString::Printf(TEXT("CameraShake=%s"), *GetNameSafe(CueEvent.CameraShakeClass)));
		}

		return AssetLabels.IsEmpty() ? TEXT("no presentation assets") : FString::Join(AssetLabels, TEXT(", "));
	}

	FString DescribeTracerAssets(const FMABSHitTraceTracerPresentationData& TracerData)
	{
		TArray<FString> AssetLabels;
		if (TracerData.TracerVFX != nullptr)
		{
			AssetLabels.Add(FString::Printf(TEXT("VFX=%s"), *GetNameSafe(TracerData.TracerVFX)));
		}
		if (TracerData.TracerSFX != nullptr)
		{
			AssetLabels.Add(FString::Printf(TEXT("SFX=%s"), *GetNameSafe(TracerData.TracerSFX)));
		}

		return AssetLabels.IsEmpty() ? TEXT("no tracer assets") : FString::Join(AssetLabels, TEXT(", "));
	}

	FString DescribeTracerCueAssets(const FMABSTracerCueEvent& TracerEvent)
	{
		TArray<FString> AssetLabels;
		if (TracerEvent.VFX != nullptr)
		{
			AssetLabels.Add(FString::Printf(TEXT("VFX=%s"), *GetNameSafe(TracerEvent.VFX)));
		}
		if (TracerEvent.SFX != nullptr)
		{
			AssetLabels.Add(FString::Printf(TEXT("SFX=%s"), *GetNameSafe(TracerEvent.SFX)));
		}

		return AssetLabels.IsEmpty() ? TEXT("no tracer assets") : FString::Join(AssetLabels, TEXT(", "));
	}

	FName GetActivationFailureEventName(const EMABSAbilityActivationResult ActivationResult)
	{
		switch (ActivationResult)
		{
		case EMABSAbilityActivationResult::ComboRejected:
			return MABSAbilityComponentEventNames::ComboRejected;

		case EMABSAbilityActivationResult::OnCooldown:
			return MABSAbilityComponentEventNames::CooldownRejected;

		case EMABSAbilityActivationResult::InsufficientResources:
			return MABSAbilityComponentEventNames::CostRejected;

		case EMABSAbilityActivationResult::DeliveryFailed:
			return MABSAbilityComponentEventNames::DeliveryFailed;

		default:
			return MABSAbilityComponentEventNames::RequestRejected;
		}
	}

	EMABSTargetTraceMode GetTraceModeForRadius(const float TraceRadius)
	{
		return TraceRadius > 0.0f ? EMABSTargetTraceMode::Sphere : EMABSTargetTraceMode::Line;
	}

	FCollisionObjectQueryParams MakeTargetTraceObjectQueryParams(const bool bIgnoreNonTargetWorldHits)
	{
		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Destructible);

		if (!bIgnoreNonTargetWorldHits)
		{
			ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
		}

		return ObjectQueryParams;
	}

	const FName NiagaraTracerTraceStartParameter(TEXT("User.MABS_TraceStart"));
	const FName NiagaraTracerTraceEndParameter(TEXT("User.MABS_TraceEnd"));
	const FName NiagaraTracerImpactPointParameter(TEXT("User.MABS_ImpactPoint"));
}

UMABSAbilityComponent::UMABSAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

#include "Components/MABSAbilityComponent_Core.inl"
#include "Components/MABSAbilityComponent_Granting.inl"
#include "Components/MABSAbilityComponent_Activation.inl"
#include "Components/MABSAbilityComponent_Delivery.inl"
#include "Components/MABSAbilityComponent_Effects.inl"
#include "Components/MABSAbilityComponent_Debug.inl"
#include "Components/MABSAbilityComponent_Presentation.inl"
