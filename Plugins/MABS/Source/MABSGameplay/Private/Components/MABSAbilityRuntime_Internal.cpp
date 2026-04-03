// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/MABSAbilityRuntime_Internal.h"

namespace MABSAbilityComponentEventNames
{
	const FName AbilityBlocked(TEXT("AbilityBlocked"));
	const FName AbilityGranted(TEXT("AbilityGranted"));
	const FName AbilityGrantRejected(TEXT("AbilityGrantRejected"));
	const FName AbilitySetGranted(TEXT("AbilitySetGranted"));
	const FName AbilitySetGrantFailed(TEXT("AbilitySetGrantFailed"));
	const FName AbilitySetGrantSkipped(TEXT("AbilitySetGrantSkipped"));
	const FName AbilityUnblocked(TEXT("AbilityUnblocked"));
	const FName CommitSucceeded(TEXT("CommitSucceeded"));
	const FName CooldownRejected(TEXT("CooldownRejected"));
	const FName CooldownStarted(TEXT("CooldownStarted"));
	const FName ComboQueued(TEXT("ComboQueued"));
	const FName ComboRejected(TEXT("ComboRejected"));
	const FName ComboWindowEnded(TEXT("ComboWindowEnded"));
	const FName ComboWindowStarted(TEXT("ComboWindowStarted"));
	const FName DeliveryFailed(TEXT("DeliveryFailed"));
	const FName DeliveryStarted(TEXT("DeliveryStarted"));
	const FName EffectApplied(TEXT("EffectApplied"));
	const FName EffectApplicationFailed(TEXT("EffectApplicationFailed"));
	const FName AoEResolved(TEXT("AoEResolved"));
	const FName AoETargetRejected(TEXT("AoETargetRejected"));
	const FName CostRejected(TEXT("CostRejected"));
	const FName CostSpent(TEXT("CostSpent"));
	const FName CostValidated(TEXT("CostValidated"));
	const FName HitTraceHit(TEXT("HitTraceHit"));
	const FName HitTraceRejected(TEXT("HitTraceRejected"));
	const FName MeleeHit(TEXT("MeleeHit"));
	const FName MeleeRejected(TEXT("MeleeRejected"));
	const FName MontagePlayFailed(TEXT("MontagePlayFailed"));
	const FName MontagePlayRequested(TEXT("MontagePlayRequested"));
	const FName PresentationAssetMissing(TEXT("PresentationAssetMissing"));
	const FName PresentationCuePolicyFallbackUsed(TEXT("PresentationCuePolicyFallbackUsed"));
	const FName PresentationCueRealized(TEXT("PresentationCueRealized"));
	const FName PresentationCueRouted(TEXT("PresentationCueRouted"));
	const FName PresentationCueSkipped(TEXT("PresentationCueSkipped"));
	const FName PresentationSocketFallbackUsed(TEXT("PresentationSocketFallbackUsed"));
	const FName ProjectileImpact(TEXT("ProjectileImpact"));
	const FName ProjectileImpactRejected(TEXT("ProjectileImpactRejected"));
	const FName ProjectileTravelPresentationTriggered(TEXT("ProjectileTravelPresentationTriggered"));
	const FName ProjectileSpawned(TEXT("ProjectileSpawned"));
	const FName ProjectileSpawnFailed(TEXT("ProjectileSpawnFailed"));
	const FName PeriodicEffectApplied(TEXT("PeriodicEffectApplied"));
	const FName PeriodicEffectExpired(TEXT("PeriodicEffectExpired"));
	const FName PeriodicEffectRefreshed(TEXT("PeriodicEffectRefreshed"));
	const FName PeriodicEffectTick(TEXT("PeriodicEffectTick"));
	const FName RecoveryCompleted(TEXT("RecoveryCompleted"));
	const FName RecoveryStarted(TEXT("RecoveryStarted"));
	const FName RequestAccepted(TEXT("RequestAccepted"));
	const FName RequestRejected(TEXT("RequestRejected"));
	const FName RequestSentToServer(TEXT("RequestSentToServer"));
	const FName RequestStarted(TEXT("RequestStarted"));
	const FName SocketFallbackUsed(TEXT("SocketFallbackUsed"));
	const FName SocketResolved(TEXT("SocketResolved"));
	const FName StartupStarted(TEXT("StartupStarted"));
	const FName StartupPresentationTriggered(TEXT("StartupPresentationTriggered"));
	const FName TargetTraceHit(TEXT("TargetTraceHit"));
	const FName TargetTraceRejected(TEXT("TargetTraceRejected"));
	const FName TargetTraceStarted(TEXT("TargetTraceStarted"));
	const FName TargetResolved(TEXT("TargetResolved"));
	const FName TargetResolutionFailed(TEXT("TargetResolutionFailed"));
	const FName DeliveryScheduled(TEXT("DeliveryScheduled"));
	const FName DeliveryTriggered(TEXT("DeliveryTriggered"));
	const FName DeliveryPresentationTriggered(TEXT("DeliveryPresentationTriggered"));
	const FName ImpactPresentationTriggered(TEXT("ImpactPresentationTriggered"));
	const FName TracerCueRealized(TEXT("TracerCueRealized"));
	const FName TracerCueRouted(TEXT("TracerCueRouted"));
	const FName TracerCueSkipped(TEXT("TracerCueSkipped"));
	const FName TracerSpawned(TEXT("TracerSpawned"));
	const FName TracerSpawnFailed(TEXT("TracerSpawnFailed"));
}

namespace MABSAbilityRuntimeInternal
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
