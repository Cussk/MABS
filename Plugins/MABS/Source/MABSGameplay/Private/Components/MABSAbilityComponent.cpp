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

void UMABSAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMABSAbilityComponent, GrantedAbilities);
	DOREPLIFETIME(UMABSAbilityComponent, CooldownGroupStates);
	DOREPLIFETIME_CONDITION(UMABSAbilityComponent, bReplicateDebugDataToOwningClient, COND_OwnerOnly);
}

FMABSAbilityHandle UMABSAbilityComponent::GrantAbility(UMABSAbilityDefinition* AbilityDefinition)
{
	const FMABSAbilityHandle InvalidHandle;

	if (AbilityDefinition == nullptr)
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::AbilityGrantRejected,
			FGameplayTag(),
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::InvalidAbility,
			TEXT("GrantAbility rejected because the ability definition was null."));
		return InvalidHandle;
	}

	if (!CanMutateAbilityState())
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::AbilityGrantRejected,
			AbilityDefinition->AbilityTag,
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::AuthorityRejected,
			TEXT("GrantAbility must run on the authoritative owner."));
		return InvalidHandle;
	}

	if (!AbilityDefinition->AbilityTag.IsValid())
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::AbilityGrantRejected,
			AbilityDefinition->AbilityTag,
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::InvalidAbility,
			TEXT("GrantAbility rejected because the ability definition does not have a valid ability tag."));
		return InvalidHandle;
	}

	if (const FMABSAbilitySpec* ExistingSpec = FindGrantedAbilitySpecByTagInternal(AbilityDefinition->AbilityTag))
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::AbilityGrantRejected,
			AbilityDefinition->AbilityTag,
			ExistingSpec->Handle,
			ExistingSpec->RuntimeState,
			EMABSAbilityActivationResult::InvalidAbility,
			TEXT("GrantAbility rejected because that ability tag is already granted."));
		return InvalidHandle;
	}

	FMABSAbilitySpec AbilitySpec;
	AbilitySpec.Handle = MakeNextAbilityHandle();
	AbilitySpec.AbilityDefinition = AbilityDefinition;
	AbilitySpec.AbilityTag = AbilityDefinition->AbilityTag;
	AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
	AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::None;
	AbilitySpec.CooldownEndTime = 0.0f;
	AbilitySpec.ActivationStartTime = 0.0f;
	AbilitySpec.ScheduledDeliveryTime = 0.0f;
	AbilitySpec.RecoveryEndTime = 0.0f;
	AbilitySpec.ComboWindowStartTime = 0.0f;
	AbilitySpec.ComboWindowEndTime = 0.0f;
	AbilitySpec.QueuedComboAbilityTag = FGameplayTag();
	AbilitySpec.ComboInputRoutingTag = FGameplayTag();
	GrantedAbilities.Add(AbilitySpec);

	EmitDebugEvent(
		MABSAbilityComponentEventNames::AbilityGranted,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(TEXT("Granted ability '%s'."), *GetAbilityLabel(AbilityDefinition)));

	return AbilitySpec.Handle;
}

bool UMABSAbilityComponent::SetAbilityBlockedByTag(FGameplayTag AbilityTag, const bool bBlocked)
{
	const FMABSAbilityHandle InvalidHandle;

	if (!AbilityTag.IsValid())
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::RequestRejected,
			AbilityTag,
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::InvalidAbility,
			TEXT("SetAbilityBlockedByTag requires a valid ability tag."));
		return false;
	}

	if (!CanMutateAbilityState())
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::RequestRejected,
			AbilityTag,
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::AuthorityRejected,
			TEXT("SetAbilityBlockedByTag must run on the authoritative owner."));
		return false;
	}

	FMABSAbilitySpec* AbilitySpec = FindGrantedAbilitySpecByTagMutable(AbilityTag);
	if (AbilitySpec == nullptr)
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::RequestRejected,
			AbilityTag,
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::NotGranted,
			TEXT("SetAbilityBlockedByTag failed because the ability has not been granted."));
		return false;
	}

	ClearAbilityExecutionContext(AbilitySpec->Handle, true);

	AbilitySpec->RuntimeState = bBlocked
		? EMABSAbilityRuntimeState::Blocked
		: EMABSAbilityRuntimeState::Idle;

	EmitDebugEvent(
		bBlocked ? MABSAbilityComponentEventNames::AbilityBlocked : MABSAbilityComponentEventNames::AbilityUnblocked,
		AbilitySpec->AbilityTag,
		AbilitySpec->Handle,
		AbilitySpec->RuntimeState,
		EMABSAbilityActivationResult::Success,
		bBlocked
			? TEXT("Ability runtime state marked as blocked.")
			: TEXT("Ability runtime state returned to idle."));

	return true;
}

EMABSAbilityActivationResult UMABSAbilityComponent::TryActivateAbilityByTag(const FGameplayTag AbilityTag)
{
	const FMABSAbilityHandle InvalidHandle;

	if (!AbilityTag.IsValid())
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::RequestRejected,
			AbilityTag,
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::InvalidAbility,
			TEXT("TryActivateAbilityByTag requires a valid ability tag."));
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (CanMutateAbilityState())
	{
		return HandleTryActivateAbility(AbilityTag, false, AbilityTag);
	}

	const FMABSAbilitySpec* AbilitySpec = FindGrantedAbilitySpecByTagInternal(AbilityTag);
	const FMABSAbilityHandle AbilityHandle = AbilitySpec != nullptr ? AbilitySpec->Handle : InvalidHandle;
	const EMABSAbilityRuntimeState RuntimeState = AbilitySpec != nullptr
		? AbilitySpec->RuntimeState
		: EMABSAbilityRuntimeState::None;

	EmitDebugEvent(
		MABSAbilityComponentEventNames::RequestStarted,
		AbilityTag,
		AbilityHandle,
		RuntimeState,
		EMABSAbilityActivationResult::None,
		TEXT("Client started an ability activation request."));

	EmitDebugEvent(
		MABSAbilityComponentEventNames::RequestSentToServer,
		AbilityTag,
		AbilityHandle,
		RuntimeState,
		EMABSAbilityActivationResult::RequestSentToServer,
		TEXT("Client sent the ability activation request to the server."));

	ServerTryActivateAbilityByTag(AbilityTag);
	return EMABSAbilityActivationResult::RequestSentToServer;
}

TArray<FMABSAbilitySpec> UMABSAbilityComponent::GetGrantedAbilities() const
{
	return GrantedAbilities;
}

bool UMABSAbilityComponent::FindGrantedAbilitySpecByTag(const FGameplayTag AbilityTag, FMABSAbilitySpec& OutAbilitySpec) const
{
	if (const FMABSAbilitySpec* AbilitySpec = FindGrantedAbilitySpecByTagInternal(AbilityTag))
	{
		OutAbilitySpec = *AbilitySpec;
		return true;
	}

	return false;
}

float UMABSAbilityComponent::GetCooldownRemainingByTag(const FGameplayTag AbilityTag) const
{
	const FMABSAbilitySpec* AbilitySpec = FindGrantedAbilitySpecByTagInternal(AbilityTag);
	return AbilitySpec != nullptr ? GetCooldownRemainingForAbilitySpec(*AbilitySpec) : 0.0f;
}

bool UMABSAbilityComponent::IsAbilityOnCooldown(const FGameplayTag AbilityTag) const
{
	return GetCooldownRemainingByTag(AbilityTag) > 0.0f;
}

float UMABSAbilityComponent::GetCooldownGroupRemaining(const FGameplayTag CooldownGroupTag) const
{
	return GetCooldownGroupRemainingInternal(CooldownGroupTag);
}

bool UMABSAbilityComponent::IsCooldownGroupActive(const FGameplayTag CooldownGroupTag) const
{
	return GetCooldownGroupRemainingInternal(CooldownGroupTag) > 0.0f;
}

TArray<FMABSAbilityDebugEvent> UMABSAbilityComponent::GetRecentDebugEvents() const
{
	return RecentDebugEvents;
}

FMABSTargetTraceDebugInfo UMABSAbilityComponent::GetLatestTargetTraceDebugInfo() const
{
	return LatestTargetTraceDebugInfo;
}

TArray<FMABSActivePeriodicEffect> UMABSAbilityComponent::GetActivePeriodicEffects() const
{
	return ActivePeriodicEffects;
}

void UMABSAbilityComponent::SetDebugReplicationEnabled(const bool bEnabled)
{
	if (!CanMutateAbilityState())
	{
		return;
	}

	bReplicateDebugDataToOwningClient = bEnabled;
}

bool UMABSAbilityComponent::IsDebugReplicationEnabled() const
{
	return bReplicateDebugDataToOwningClient;
}

void UMABSAbilityComponent::ServerTryActivateAbilityByTag_Implementation(const FGameplayTag AbilityTag)
{
	HandleTryActivateAbility(AbilityTag, true, AbilityTag);
}

void UMABSAbilityComponent::ClientReceiveAbilityDebugEvent_Implementation(const FMABSAbilityDebugEvent& DebugEvent)
{
	RecordDebugEvent(DebugEvent);
}

void UMABSAbilityComponent::ClientReceiveTargetTraceDebugInfo_Implementation(const FMABSTargetTraceDebugInfo& DebugInfo)
{
	RecordLatestTargetTraceDebugInfo(DebugInfo);
}

void UMABSAbilityComponent::MulticastPlayActivationMontage_Implementation(UAnimMontage* Montage, const float PlayRate)
{
	PlayActivationMontageLocally(Montage, PlayRate);
}

void UMABSAbilityComponent::ClientPlayPresentationCue_Implementation(const FMABSPresentationCueEvent& CueEvent)
{
	PlayPresentationCueLocally(CueEvent);
}

void UMABSAbilityComponent::MulticastPlayPresentationCue_Implementation(const FMABSPresentationCueEvent& CueEvent)
{
	PlayPresentationCueLocally(CueEvent);
}

void UMABSAbilityComponent::ClientSpawnTracerPresentation_Implementation(const FMABSTracerCueEvent& TracerEvent)
{
	SpawnTracerPresentationLocally(TracerEvent);
}

void UMABSAbilityComponent::MulticastSpawnTracerPresentation_Implementation(const FMABSTracerCueEvent& TracerEvent)
{
	SpawnTracerPresentationLocally(TracerEvent);
}

EMABSAbilityActivationResult UMABSAbilityComponent::HandleTryActivateAbility(
	const FGameplayTag& AbilityTag,
	const bool bNotifyOwningClient,
	const FGameplayTag& ComboInputRoutingTag)
{
	const FMABSAbilityHandle InvalidHandle;

	PruneExpiredCooldownGroupStates();

	if (!AbilityTag.IsValid())
	{
		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::RequestRejected,
			AbilityTag,
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::InvalidAbility,
			TEXT("Activation rejected because the requested ability tag was invalid."));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DebugEvent);
		}

		return EMABSAbilityActivationResult::InvalidAbility;
	}

	const EMABSAbilityActivationResult ComboQueueResult = TryQueueComboFollowup(AbilityTag, bNotifyOwningClient);
	if (ComboQueueResult != EMABSAbilityActivationResult::None)
	{
		return ComboQueueResult;
	}

	FMABSAbilitySpec* AbilitySpec = FindGrantedAbilitySpecByTagMutable(AbilityTag);
	if (AbilitySpec == nullptr)
	{
		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::RequestRejected,
			AbilityTag,
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::NotGranted,
			TEXT("Activation rejected because the ability has not been granted."));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DebugEvent);
		}

		return EMABSAbilityActivationResult::NotGranted;
	}

	EmitDebugEvent(
		MABSAbilityComponentEventNames::RequestStarted,
		AbilitySpec->AbilityTag,
		AbilitySpec->Handle,
		AbilitySpec->RuntimeState,
		EMABSAbilityActivationResult::None,
		TEXT("Authority started ability activation validation."));

	FString ActivationDebugMessage;
	const EMABSAbilityActivationResult ActivationResult = CanActivateAbility(*AbilitySpec, ActivationDebugMessage);
	AbilitySpec->LastActivationResult = ActivationResult;

	if (ActivationResult != EMABSAbilityActivationResult::Success)
	{
		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			GetActivationFailureEventName(ActivationResult),
			AbilitySpec->AbilityTag,
			AbilitySpec->Handle,
			AbilitySpec->RuntimeState,
			ActivationResult,
			ActivationDebugMessage.IsEmpty() ? DescribeActivationResult(ActivationResult) : ActivationDebugMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DebugEvent);
		}

		return ActivationResult;
	}

	if (ShouldValidateAbilityCost(*AbilitySpec))
	{
		const FString CostValidatedMessage = FString::Printf(
			TEXT("Validated resource cost %.2f for ability '%s'."),
			AbilitySpec->AbilityDefinition->ResourceCost,
			*GetAbilityLabel(AbilitySpec->AbilityDefinition));
		const FMABSAbilityDebugEvent CostValidatedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::CostValidated,
			AbilitySpec->AbilityTag,
			AbilitySpec->Handle,
			AbilitySpec->RuntimeState,
			EMABSAbilityActivationResult::Success,
			CostValidatedMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(CostValidatedEvent);
		}
	}

	const FMABSAbilityDebugEvent AcceptedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::RequestAccepted,
		AbilitySpec->AbilityTag,
		AbilitySpec->Handle,
		AbilitySpec->RuntimeState,
		EMABSAbilityActivationResult::Success,
		TEXT("Authority accepted the ability activation request."));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(AcceptedEvent);
	}

	return BeginAbilityStartup(
		*AbilitySpec,
		bNotifyOwningClient,
		ComboInputRoutingTag.IsValid() ? ComboInputRoutingTag : AbilityTag);
}

EMABSAbilityActivationResult UMABSAbilityComponent::CanActivateAbility(const FMABSAbilitySpec& AbilitySpec, FString& OutDebugMessage) const
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	if (AbilityDefinition == nullptr || !AbilitySpec.AbilityTag.IsValid())
	{
		OutDebugMessage = TEXT("Activation rejected because the ability spec is missing a valid definition or tag.");
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (AbilityDefinition->AbilityTag != AbilitySpec.AbilityTag)
	{
		OutDebugMessage = TEXT("Activation rejected because the granted ability tag no longer matches the authored definition.");
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (!HasAuthoredGameplayEffect(AbilityDefinition))
	{
		OutDebugMessage = TEXT("Activation rejected because the ability does not author a valid instant or periodic gameplay effect.");
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (AbilityDefinition->InstantEffectType != EMABSInstantEffectType::None
		&& AbilityDefinition->EffectMagnitude <= 0.0f)
	{
		OutDebugMessage = TEXT("Activation rejected because the authored instant effect magnitude must be greater than zero.");
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (AbilityDefinition->PeriodicEffect.bEnabled && !AbilityDefinition->PeriodicEffect.IsValid())
	{
		OutDebugMessage = TEXT("Activation rejected because the authored periodic effect data is invalid.");
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (AbilityDefinition->AoE.bEnabled && !AbilityDefinition->AoE.IsValid())
	{
		OutDebugMessage = TEXT("Activation rejected because the authored AoE data is invalid.");
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (AbilityDefinition->Combo.NextComboAbilityTag.IsValid())
	{
		if (AbilityDefinition->DeliveryMode != EMABSDeliveryMode::Melee)
		{
			OutDebugMessage = TEXT("Activation rejected because combo follow-ups are only supported for Melee delivery in Phase 7.");
			return EMABSAbilityActivationResult::InvalidAbility;
		}

		if (!AbilityDefinition->Combo.IsEnabled())
		{
			OutDebugMessage = TEXT("Activation rejected because the authored combo window is invalid.");
			return EMABSAbilityActivationResult::InvalidAbility;
		}
	}

	if (AbilityDefinition->TargetType == EMABSTargetType::Location)
	{
		OutDebugMessage = TEXT("Activation rejected because location target intent is not supported in Phase 7.");
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	switch (AbilityDefinition->DeliveryMode)
	{
	case EMABSDeliveryMode::Direct:
		if (AbilityDefinition->TargetType != EMABSTargetType::Self
			&& AbilityDefinition->TargetType != EMABSTargetType::Actor)
		{
			OutDebugMessage = TEXT("Activation rejected because direct delivery only supports Self or Actor target intent.");
			return EMABSAbilityActivationResult::InvalidAbility;
		}

		if (AbilityDefinition->TargetType == EMABSTargetType::Actor
			&& AbilityDefinition->TargetTraceDistance <= 0.0f)
		{
			OutDebugMessage = TEXT("Activation rejected because direct actor targeting requires a positive trace distance.");
			return EMABSAbilityActivationResult::InvalidAbility;
		}

		if (AbilityDefinition->TargetType == EMABSTargetType::Actor
			&& AbilityDefinition->ActorTargetTraceMode == EMABSTargetTraceMode::Sphere
			&& AbilityDefinition->TargetTraceRadius <= 0.0f)
		{
			OutDebugMessage = TEXT("Activation rejected because direct sphere targeting requires a positive trace radius.");
			return EMABSAbilityActivationResult::InvalidAbility;
		}
		break;

	case EMABSDeliveryMode::HitTrace:
		if (AbilityDefinition->TargetType != EMABSTargetType::Actor)
		{
			OutDebugMessage = TEXT("Activation rejected because HitTrace delivery requires Actor target intent.");
			return EMABSAbilityActivationResult::InvalidAbility;
		}

		if (AbilityDefinition->HitTraceDistance <= 0.0f)
		{
			OutDebugMessage = TEXT("Activation rejected because HitTrace delivery requires a positive trace distance.");
			return EMABSAbilityActivationResult::InvalidAbility;
		}
		break;

	case EMABSDeliveryMode::Melee:
		if (AbilityDefinition->TargetType != EMABSTargetType::Actor)
		{
			OutDebugMessage = TEXT("Activation rejected because Melee delivery requires Actor target intent.");
			return EMABSAbilityActivationResult::InvalidAbility;
		}

		if (AbilityDefinition->MeleeRange <= 0.0f)
		{
			OutDebugMessage = TEXT("Activation rejected because Melee delivery requires a positive range.");
			return EMABSAbilityActivationResult::InvalidAbility;
		}

		if (AbilityDefinition->MeleeRadius <= 0.0f)
		{
			OutDebugMessage = TEXT("Activation rejected because Melee delivery requires a positive radius.");
			return EMABSAbilityActivationResult::InvalidAbility;
		}
		break;

	case EMABSDeliveryMode::Projectile:
		if (AbilityDefinition->TargetType != EMABSTargetType::Actor)
		{
			OutDebugMessage = TEXT("Activation rejected because Projectile delivery requires Actor target intent.");
			return EMABSAbilityActivationResult::InvalidAbility;
		}

		if (AbilityDefinition->ProjectileActorClass == nullptr)
		{
			OutDebugMessage = TEXT("Activation rejected because Projectile delivery requires a projectile actor class.");
			return EMABSAbilityActivationResult::InvalidAbility;
		}

		if (!AbilityDefinition->ProjectileActorClass->IsChildOf(AMABSProjectileBase::StaticClass()))
		{
			OutDebugMessage = TEXT("Activation rejected because Projectile delivery requires a class derived from AMABSProjectileBase.");
			return EMABSAbilityActivationResult::InvalidAbility;
		}
		break;

	default:
		OutDebugMessage = TEXT("Activation rejected because the authored delivery mode is not supported.");
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Blocked)
	{
		return EMABSAbilityActivationResult::Blocked;
	}

	if (AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Startup
		|| AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Active
		|| AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Recovery)
	{
		OutDebugMessage = FString::Printf(
			TEXT("Activation rejected because ability '%s' is already in runtime state '%s'."),
			*GetAbilityLabel(AbilityDefinition),
			*GetRuntimeStateLabel(AbilitySpec.RuntimeState));
		return EMABSAbilityActivationResult::AlreadyActive;
	}

	const float AbilityCooldownRemaining = GetCooldownRemainingForAbilitySpec(AbilitySpec);
	if (AbilityCooldownRemaining > 0.0f)
	{
		const FGameplayTag CooldownGroupTag = AbilitySpec.AbilityDefinition->CooldownGroupTag;
		if (CooldownGroupTag.IsValid())
		{
			const float GroupCooldownRemaining = GetCooldownGroupRemainingInternal(CooldownGroupTag);
			if (GroupCooldownRemaining > 0.0f)
			{
				OutDebugMessage = FString::Printf(
					TEXT("Activation rejected because cooldown group '%s' is active for %.2f more seconds."),
					*CooldownGroupTag.ToString(),
					GroupCooldownRemaining);
				return EMABSAbilityActivationResult::OnCooldown;
			}
		}

		OutDebugMessage = FString::Printf(
			TEXT("Activation rejected because ability '%s' is on cooldown for %.2f more seconds."),
			*GetAbilityLabel(AbilitySpec.AbilityDefinition),
			AbilityCooldownRemaining);
		return EMABSAbilityActivationResult::OnCooldown;
	}

	if (ShouldValidateAbilityCost(AbilitySpec))
	{
		AActor* const Owner = GetOwner();
		if (Owner == nullptr || !Owner->GetClass()->ImplementsInterface(UMABSCostReceiver::StaticClass()))
		{
			OutDebugMessage = FString::Printf(
				TEXT("Activation rejected because owner '%s' does not implement IMABSCostReceiver for resource cost %.2f."),
				*GetNameSafe(Owner),
				AbilityDefinition->ResourceCost);
			return EMABSAbilityActivationResult::InsufficientResources;
		}

		const bool bCanAffordCost = IMABSCostReceiver::Execute_CanAffordMABSCost(
			Owner,
			AbilityDefinition->ResourceCost,
			const_cast<UMABSAbilityDefinition*>(AbilityDefinition));
		if (!bCanAffordCost)
		{
			OutDebugMessage = FString::Printf(
				TEXT("Activation rejected because owner '%s' cannot afford resource cost %.2f for ability '%s'."),
				*GetNameSafe(Owner),
				AbilityDefinition->ResourceCost,
				*GetAbilityLabel(AbilityDefinition));
			return EMABSAbilityActivationResult::InsufficientResources;
		}
	}

	return EMABSAbilityActivationResult::Success;
}

EMABSAbilityActivationResult UMABSAbilityComponent::TryQueueComboFollowup(const FGameplayTag& RequestedAbilityTag, const bool bNotifyOwningClient)
{
	FMABSAbilitySpec* const ComboSourceSpec = FindActiveComboSourceSpecForRequest(RequestedAbilityTag);
	if (ComboSourceSpec == nullptr)
	{
		return EMABSAbilityActivationResult::None;
	}

	const UMABSAbilityDefinition* const SourceDefinition = ComboSourceSpec->AbilityDefinition;
	if (SourceDefinition == nullptr || !SourceDefinition->Combo.NextComboAbilityTag.IsValid())
	{
		return EMABSAbilityActivationResult::None;
	}

	const FGameplayTag RequestedFollowupAbilityTag = SourceDefinition->Combo.NextComboAbilityTag;

	FMABSAbilitySpec* const RequestedAbilitySpec = FindGrantedAbilitySpecByTagMutable(RequestedFollowupAbilityTag);
	if (RequestedAbilitySpec == nullptr)
	{
		const FMABSAbilityDebugEvent RejectedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::ComboRejected,
			ComboSourceSpec->AbilityTag,
			ComboSourceSpec->Handle,
			ComboSourceSpec->RuntimeState,
			EMABSAbilityActivationResult::NotGranted,
			FString::Printf(
				TEXT("Rejected combo follow-up '%s' because it has not been granted."),
				*RequestedFollowupAbilityTag.ToString()));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(RejectedEvent);
		}

		return EMABSAbilityActivationResult::NotGranted;
	}

	UWorld* const World = GetWorld();
	if (SourceDefinition == nullptr || World == nullptr)
	{
		RequestedAbilitySpec->LastActivationResult = EMABSAbilityActivationResult::ComboRejected;
		return EMABSAbilityActivationResult::ComboRejected;
	}

	const float CurrentWorldTime = World->GetTimeSeconds();
	const bool bWindowOpen = 
		ComboSourceSpec->ComboWindowEndTime > ComboSourceSpec->ComboWindowStartTime
		&& CurrentWorldTime >= ComboSourceSpec->ComboWindowStartTime
		&& CurrentWorldTime <= ComboSourceSpec->ComboWindowEndTime;
	if (!bWindowOpen)
	{
		RequestedAbilitySpec->LastActivationResult = EMABSAbilityActivationResult::ComboRejected;

		const FMABSAbilityDebugEvent RejectedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::ComboRejected,
			ComboSourceSpec->AbilityTag,
			ComboSourceSpec->Handle,
			ComboSourceSpec->RuntimeState,
			EMABSAbilityActivationResult::ComboRejected,
			FString::Printf(
				TEXT("Rejected combo follow-up '%s' because the combo window for ability '%s' is not open."),
				*RequestedFollowupAbilityTag.ToString(),
				*GetAbilityLabel(SourceDefinition)));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(RejectedEvent);
		}

		return EMABSAbilityActivationResult::ComboRejected;
	}

	if (!SourceDefinition->Combo.bBufferComboInput
		&& ComboSourceSpec->RuntimeState != EMABSAbilityRuntimeState::Recovery)
	{
		RequestedAbilitySpec->LastActivationResult = EMABSAbilityActivationResult::ComboRejected;

		const FMABSAbilityDebugEvent RejectedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::ComboRejected,
			ComboSourceSpec->AbilityTag,
			ComboSourceSpec->Handle,
			ComboSourceSpec->RuntimeState,
			EMABSAbilityActivationResult::ComboRejected,
			FString::Printf(
				TEXT("Rejected combo follow-up '%s' because input buffering is disabled and ability '%s' is not yet in recovery."),
				*RequestedFollowupAbilityTag.ToString(),
				*GetAbilityLabel(SourceDefinition)));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(RejectedEvent);
		}

		return EMABSAbilityActivationResult::ComboRejected;
	}

	FString ValidationMessage;
	const EMABSAbilityActivationResult ValidationResult = CanActivateAbility(*RequestedAbilitySpec, ValidationMessage);
	if (ValidationResult != EMABSAbilityActivationResult::Success)
	{
		RequestedAbilitySpec->LastActivationResult = ValidationResult;

		const FMABSAbilityDebugEvent RejectedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::ComboRejected,
			ComboSourceSpec->AbilityTag,
			ComboSourceSpec->Handle,
			ComboSourceSpec->RuntimeState,
			ValidationResult,
			FString::Printf(
				TEXT("Rejected combo follow-up '%s': %s"),
				*RequestedFollowupAbilityTag.ToString(),
				*ValidationMessage));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(RejectedEvent);
		}

		return ValidationResult;
	}

	ComboSourceSpec->QueuedComboAbilityTag = RequestedFollowupAbilityTag;
	RequestedAbilitySpec->LastActivationResult = EMABSAbilityActivationResult::ComboQueued;

	const FMABSAbilityDebugEvent QueuedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::ComboQueued,
		ComboSourceSpec->AbilityTag,
		ComboSourceSpec->Handle,
		ComboSourceSpec->RuntimeState,
		EMABSAbilityActivationResult::ComboQueued,
		FString::Printf(
			TEXT("Queued combo follow-up '%s' from ability '%s' after request '%s'."),
			*RequestedFollowupAbilityTag.ToString(),
			*GetAbilityLabel(SourceDefinition),
			*RequestedAbilityTag.ToString()));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(QueuedEvent);
	}

	return EMABSAbilityActivationResult::ComboQueued;
}

EMABSAbilityActivationResult UMABSAbilityComponent::BeginAbilityStartup(
	FMABSAbilitySpec& AbilitySpec,
	const bool bNotifyOwningClient,
	const FGameplayTag& ComboInputRoutingTag)
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	UWorld* const World = GetWorld();
	if (AbilityDefinition == nullptr || World == nullptr)
	{
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::InvalidAbility;
		ClearAbilityRuntimeTimes(AbilitySpec);

		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::RequestRejected,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::InvalidAbility,
			TEXT("Ability startup could not begin because the world or ability definition was unavailable."));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DebugEvent);
		}

		return EMABSAbilityActivationResult::InvalidAbility;
	}

	ClearAbilityExecutionContext(AbilitySpec.Handle, true);

	FMABSAbilityExecutionContext& ExecutionContext = ActiveAbilityExecutionContexts.FindOrAdd(AbilitySpec.Handle);
	ExecutionContext.bNotifyOwningClient = bNotifyOwningClient;

	const float CurrentWorldTime = World->GetTimeSeconds();
	const float DeliveryDelay = GetEffectiveDeliveryDelay(*AbilityDefinition);
	AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Startup;
	AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::Success;
	AbilitySpec.ActivationStartTime = CurrentWorldTime;
	AbilitySpec.ScheduledDeliveryTime = CurrentWorldTime + DeliveryDelay;
	AbilitySpec.RecoveryEndTime = 0.0f;
	AbilitySpec.ComboWindowStartTime = 0.0f;
	AbilitySpec.ComboWindowEndTime = 0.0f;
	AbilitySpec.QueuedComboAbilityTag = FGameplayTag();
	AbilitySpec.ComboInputRoutingTag = ComboInputRoutingTag;

	const FMABSAbilityDebugEvent StartupStartedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::StartupStarted,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Ability '%s' entered startup. Authored startup is %.2f seconds and delivery is scheduled %.2f seconds after activation start."),
			*GetAbilityLabel(AbilityDefinition),
			FMath::Max(0.0f, AbilityDefinition->StartupDuration),
			DeliveryDelay));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(StartupStartedEvent);
	}

	TriggerStartupPresentation(AbilitySpec, bNotifyOwningClient);
	RequestAbilityMontagePlayback(AbilitySpec, bNotifyOwningClient);

	const FMABSAbilityDebugEvent DeliveryScheduledEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::DeliveryScheduled,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Scheduled %s delivery for ability '%s' at world time %.2f."),
			*GetDeliveryModeLabel(AbilityDefinition->DeliveryMode),
			*GetAbilityLabel(AbilityDefinition),
			AbilitySpec.ScheduledDeliveryTime));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(DeliveryScheduledEvent);
	}

	if (AbilityDefinition->Combo.IsEnabled())
	{
		const float ComboWindowStartDelay = FMath::Max(0.0f, AbilityDefinition->Combo.ComboWindowStart);
		const float ComboWindowEndDelay = FMath::Max(ComboWindowStartDelay, AbilityDefinition->Combo.ComboWindowEnd);
		AbilitySpec.ComboWindowStartTime = CurrentWorldTime + ComboWindowStartDelay;
		AbilitySpec.ComboWindowEndTime = CurrentWorldTime + ComboWindowEndDelay;

		if (ComboWindowStartDelay <= KINDA_SMALL_NUMBER)
		{
			OpenComboWindow(AbilitySpec.Handle);
		}
		else
		{
			FTimerDelegate ComboWindowStartDelegate = FTimerDelegate::CreateUObject(
				this,
				&UMABSAbilityComponent::OpenComboWindow,
				AbilitySpec.Handle);
			World->GetTimerManager().SetTimer(
				ExecutionContext.ComboWindowStartTimerHandle,
				ComboWindowStartDelegate,
				ComboWindowStartDelay,
				false);
		}

		if (ComboWindowEndDelay <= KINDA_SMALL_NUMBER)
		{
			CloseComboWindow(AbilitySpec.Handle);
		}
		else
		{
			FTimerDelegate ComboWindowEndDelegate = FTimerDelegate::CreateUObject(
				this,
				&UMABSAbilityComponent::CloseComboWindow,
				AbilitySpec.Handle);
			World->GetTimerManager().SetTimer(
				ExecutionContext.ComboWindowEndTimerHandle,
				ComboWindowEndDelegate,
				ComboWindowEndDelay,
				false);
		}
	}

	if (DeliveryDelay <= KINDA_SMALL_NUMBER)
	{
		TriggerScheduledAbilityDelivery(AbilitySpec.Handle);
	}
	else
	{
		FTimerDelegate DeliveryDelegate = FTimerDelegate::CreateUObject(
			this,
			&UMABSAbilityComponent::TriggerScheduledAbilityDelivery,
			AbilitySpec.Handle);
		World->GetTimerManager().SetTimer(
			ExecutionContext.DeliveryTimerHandle,
			DeliveryDelegate,
			DeliveryDelay,
			false);
	}

	return EMABSAbilityActivationResult::Success;
}

void UMABSAbilityComponent::TriggerScheduledAbilityDelivery(const FMABSAbilityHandle AbilityHandle)
{
	FMABSAbilityExecutionContext* const ExecutionContext = ActiveAbilityExecutionContexts.Find(AbilityHandle);
	FMABSAbilitySpec* const AbilitySpec = FindGrantedAbilitySpecByHandle(AbilityHandle);
	if (ExecutionContext == nullptr || AbilitySpec == nullptr)
	{
		if (ExecutionContext != nullptr)
		{
			ActiveAbilityExecutionContexts.Remove(AbilityHandle);
		}
		return;
	}

	if (AbilitySpec->RuntimeState != EMABSAbilityRuntimeState::Startup)
	{
		ClearAbilityExecutionContext(AbilityHandle, false);
		return;
	}

	AbilitySpec->RuntimeState = EMABSAbilityRuntimeState::Active;
	AbilitySpec->ScheduledDeliveryTime = 0.0f;

	const FMABSAbilityDebugEvent DeliveryTriggeredEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::DeliveryTriggered,
		AbilitySpec->AbilityTag,
		AbilitySpec->Handle,
		AbilitySpec->RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Triggered %s delivery for ability '%s' at world time %.2f."),
			AbilitySpec->AbilityDefinition != nullptr
				? *GetDeliveryModeLabel(AbilitySpec->AbilityDefinition->DeliveryMode)
				: TEXT("Unknown"),
			*GetAbilityLabel(AbilitySpec->AbilityDefinition),
			GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : 0.0f));
	if (ExecutionContext->bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(DeliveryTriggeredEvent);
	}

	CommitAbility(*AbilitySpec, ExecutionContext->bNotifyOwningClient);
}

void UMABSAbilityComponent::OpenComboWindow(const FMABSAbilityHandle AbilityHandle)
{
	FMABSAbilityExecutionContext* const ExecutionContext = ActiveAbilityExecutionContexts.Find(AbilityHandle);
	FMABSAbilitySpec* const AbilitySpec = FindGrantedAbilitySpecByHandle(AbilityHandle);
	if (ExecutionContext == nullptr || AbilitySpec == nullptr || AbilitySpec->AbilityDefinition == nullptr)
	{
		return;
	}

	if (AbilitySpec->RuntimeState == EMABSAbilityRuntimeState::Idle
		|| AbilitySpec->RuntimeState == EMABSAbilityRuntimeState::Blocked)
	{
		return;
	}

	if (ExecutionContext->bIsComboWindowOpen)
	{
		return;
	}

	ExecutionContext->bIsComboWindowOpen = true;

	const FMABSAbilityDebugEvent ComboWindowStartedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::ComboWindowStarted,
		AbilitySpec->AbilityTag,
		AbilitySpec->Handle,
		AbilitySpec->RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Opened combo window for ability '%s' until world time %.2f. Next combo ability is '%s'."),
			*GetAbilityLabel(AbilitySpec->AbilityDefinition),
			AbilitySpec->ComboWindowEndTime,
			*AbilitySpec->AbilityDefinition->Combo.NextComboAbilityTag.ToString()));
	if (ExecutionContext->bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(ComboWindowStartedEvent);
	}
}

void UMABSAbilityComponent::CloseComboWindow(const FMABSAbilityHandle AbilityHandle)
{
	FMABSAbilityExecutionContext* const ExecutionContext = ActiveAbilityExecutionContexts.Find(AbilityHandle);
	FMABSAbilitySpec* const AbilitySpec = FindGrantedAbilitySpecByHandle(AbilityHandle);
	if (AbilitySpec == nullptr)
	{
		return;
	}

	const bool bNotifyOwningClient = ExecutionContext != nullptr && ExecutionContext->bNotifyOwningClient;
	const bool bWasComboWindowOpen = ExecutionContext != nullptr && ExecutionContext->bIsComboWindowOpen;
	if (ExecutionContext != nullptr)
	{
		ExecutionContext->bIsComboWindowOpen = false;
	}

	AbilitySpec->ComboWindowStartTime = 0.0f;
	AbilitySpec->ComboWindowEndTime = 0.0f;

	if (!bWasComboWindowOpen)
	{
		return;
	}

	const FMABSAbilityDebugEvent ComboWindowEndedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::ComboWindowEnded,
		AbilitySpec->AbilityTag,
		AbilitySpec->Handle,
		AbilitySpec->RuntimeState,
		EMABSAbilityActivationResult::Success,
		AbilitySpec->QueuedComboAbilityTag.IsValid()
			? FString::Printf(
				TEXT("Closed combo window for ability '%s' with queued follow-up '%s'."),
				*GetAbilityLabel(AbilitySpec->AbilityDefinition),
				*AbilitySpec->QueuedComboAbilityTag.ToString())
			: FString::Printf(
				TEXT("Closed combo window for ability '%s' with no queued follow-up."),
				*GetAbilityLabel(AbilitySpec->AbilityDefinition)));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(ComboWindowEndedEvent);
	}
}

void UMABSAbilityComponent::BeginAbilityRecovery(FMABSAbilitySpec& AbilitySpec, const bool bNotifyOwningClient)
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	UWorld* const World = GetWorld();
	if (AbilityDefinition == nullptr || World == nullptr)
	{
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		ClearAbilityExecutionContext(AbilitySpec.Handle, true);
		return;
	}

	const float CurrentWorldTime = World->GetTimeSeconds();
	const float RecoveryDuration = FMath::Max(0.0f, AbilityDefinition->RecoveryDuration);
	const float ComboWindowEndTime = AbilityDefinition->Combo.IsEnabled()
		? FMath::Max(0.0f, AbilityDefinition->Combo.ComboWindowEnd)
		: 0.0f;
	const float EffectiveAbilityEndTime = FMath::Max(RecoveryDuration, ComboWindowEndTime);
	const float ActivationElapsedTime = AbilitySpec.ActivationStartTime > 0.0f
		? FMath::Max(0.0f, CurrentWorldTime - AbilitySpec.ActivationStartTime)
		: 0.0f;
	const float RemainingRecoveryTime = FMath::Max(0.0f, EffectiveAbilityEndTime - ActivationElapsedTime);
	AbilitySpec.ScheduledDeliveryTime = 0.0f;
	AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Recovery;
	AbilitySpec.RecoveryEndTime = RemainingRecoveryTime > KINDA_SMALL_NUMBER
		? CurrentWorldTime + RemainingRecoveryTime
		: CurrentWorldTime;

	const FMABSAbilityDebugEvent RecoveryStartedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::RecoveryStarted,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Ability '%s' entered recovery with %.2f seconds remaining. Effective total ability time is %.2f seconds from activation start."),
			*GetAbilityLabel(AbilityDefinition),
			RemainingRecoveryTime,
			EffectiveAbilityEndTime));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(RecoveryStartedEvent);
	}

	if (RemainingRecoveryTime <= KINDA_SMALL_NUMBER)
	{
		CompleteAbilityRecovery(AbilitySpec.Handle);
		return;
	}

	FMABSAbilityExecutionContext& ExecutionContext = ActiveAbilityExecutionContexts.FindOrAdd(AbilitySpec.Handle);
	ExecutionContext.bNotifyOwningClient = bNotifyOwningClient;

	FTimerDelegate RecoveryDelegate = FTimerDelegate::CreateUObject(
		this,
		&UMABSAbilityComponent::CompleteAbilityRecovery,
		AbilitySpec.Handle);
	World->GetTimerManager().SetTimer(
		ExecutionContext.RecoveryTimerHandle,
		RecoveryDelegate,
		RemainingRecoveryTime,
		false);
}

void UMABSAbilityComponent::CompleteAbilityRecovery(const FMABSAbilityHandle AbilityHandle)
{
	FMABSAbilityExecutionContext* const ExecutionContext = ActiveAbilityExecutionContexts.Find(AbilityHandle);
	FMABSAbilitySpec* const AbilitySpec = FindGrantedAbilitySpecByHandle(AbilityHandle);
	if (AbilitySpec == nullptr)
	{
		if (ExecutionContext != nullptr)
		{
			ActiveAbilityExecutionContexts.Remove(AbilityHandle);
		}
		return;
	}

	const bool bNotifyOwningClient = ExecutionContext != nullptr ? ExecutionContext->bNotifyOwningClient : false;
	const FGameplayTag QueuedComboAbilityTag = AbilitySpec->QueuedComboAbilityTag;
	const FGameplayTag ComboInputRoutingTag = AbilitySpec->ComboInputRoutingTag;
	CloseComboWindow(AbilityHandle);
	AbilitySpec->RuntimeState = EMABSAbilityRuntimeState::Idle;
	AbilitySpec->RecoveryEndTime = 0.0f;
	AbilitySpec->ActivationStartTime = 0.0f;
	AbilitySpec->ScheduledDeliveryTime = 0.0f;
	AbilitySpec->QueuedComboAbilityTag = FGameplayTag();
	AbilitySpec->ComboInputRoutingTag = FGameplayTag();

	const FMABSAbilityDebugEvent RecoveryCompletedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::RecoveryCompleted,
		AbilitySpec->AbilityTag,
		AbilitySpec->Handle,
		AbilitySpec->RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Ability '%s' completed recovery and returned to idle."),
			*GetAbilityLabel(AbilitySpec->AbilityDefinition)));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(RecoveryCompletedEvent);
	}

	ClearAbilityExecutionContext(AbilityHandle, false);

	if (QueuedComboAbilityTag.IsValid())
	{
		HandleTryActivateAbility(
			QueuedComboAbilityTag,
			bNotifyOwningClient,
			ComboInputRoutingTag.IsValid() ? ComboInputRoutingTag : QueuedComboAbilityTag);
	}
}

void UMABSAbilityComponent::ClearAbilityExecutionContext(const FMABSAbilityHandle AbilityHandle, const bool bResetRuntimeTimes)
{
	if (FMABSAbilityExecutionContext* const ExecutionContext = ActiveAbilityExecutionContexts.Find(AbilityHandle))
	{
		if (ExecutionContext->bIsComboWindowOpen)
		{
			CloseComboWindow(AbilityHandle);
		}

		if (UWorld* const World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ExecutionContext->DeliveryTimerHandle);
			World->GetTimerManager().ClearTimer(ExecutionContext->RecoveryTimerHandle);
			World->GetTimerManager().ClearTimer(ExecutionContext->ComboWindowStartTimerHandle);
			World->GetTimerManager().ClearTimer(ExecutionContext->ComboWindowEndTimerHandle);
		}

		ActiveAbilityExecutionContexts.Remove(AbilityHandle);
	}

	if (bResetRuntimeTimes)
	{
		if (FMABSAbilitySpec* const AbilitySpec = FindGrantedAbilitySpecByHandle(AbilityHandle))
		{
			ClearAbilityRuntimeTimes(*AbilitySpec);
		}
	}
}

void UMABSAbilityComponent::ClearAbilityRuntimeTimes(FMABSAbilitySpec& AbilitySpec)
{
	AbilitySpec.ActivationStartTime = 0.0f;
	AbilitySpec.ScheduledDeliveryTime = 0.0f;
	AbilitySpec.RecoveryEndTime = 0.0f;
	AbilitySpec.ComboWindowStartTime = 0.0f;
	AbilitySpec.ComboWindowEndTime = 0.0f;
	AbilitySpec.QueuedComboAbilityTag = FGameplayTag();
	AbilitySpec.ComboInputRoutingTag = FGameplayTag();
}

EMABSAbilityActivationResult UMABSAbilityComponent::CommitAbility(FMABSAbilitySpec& AbilitySpec, const bool bNotifyOwningClient)
{
	ClearLatestTargetTraceDebugInfo(bNotifyOwningClient);

	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	if (AbilityDefinition == nullptr)
	{
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::InvalidAbility;
		ClearAbilityExecutionContext(AbilitySpec.Handle, true);

		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::RequestRejected,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::InvalidAbility,
			TEXT("CommitAbility failed because the granted ability definition was invalid."));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DebugEvent);
		}

		return EMABSAbilityActivationResult::InvalidAbility;
	}

	const EMABSDeliveryMode DeliveryMode = AbilityDefinition->DeliveryMode;
	const FMABSAbilityDebugEvent DeliveryStartedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::DeliveryStarted,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Started %s delivery for ability '%s'."),
			*GetDeliveryModeLabel(DeliveryMode),
			*GetAbilityLabel(AbilityDefinition)));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(DeliveryStartedEvent);
	}

	switch (DeliveryMode)
	{
	case EMABSDeliveryMode::Direct:
		{
			FString DeliveryDebugMessage;
			const FMABSResolvedAbilityTarget ResolvedTarget = ExecuteDirectDelivery(
				AbilitySpec,
				DeliveryDebugMessage,
				bNotifyOwningClient);
			if (ResolvedTarget.TargetActor == nullptr)
			{
				AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
				AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::TargetResolutionFailed;
				ClearAbilityExecutionContext(AbilitySpec.Handle, true);

				const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
					MABSAbilityComponentEventNames::TargetResolutionFailed,
					AbilitySpec.AbilityTag,
					AbilitySpec.Handle,
					AbilitySpec.RuntimeState,
					EMABSAbilityActivationResult::TargetResolutionFailed,
					DeliveryDebugMessage);
				if (bNotifyOwningClient)
				{
					EmitDebugEventToOwningClient(DebugEvent);
				}

				return EMABSAbilityActivationResult::TargetResolutionFailed;
			}

			const FMABSAbilityDebugEvent TargetResolvedEvent = EmitDebugEvent(
				MABSAbilityComponentEventNames::TargetResolved,
				AbilitySpec.AbilityTag,
				AbilitySpec.Handle,
				AbilitySpec.RuntimeState,
				EMABSAbilityActivationResult::Success,
				DeliveryDebugMessage);
			if (bNotifyOwningClient)
			{
				EmitDebugEventToOwningClient(TargetResolvedEvent);
			}

			return CompleteResolvedTargetAbility(AbilitySpec, ResolvedTarget, DeliveryMode, bNotifyOwningClient);
		}

	case EMABSDeliveryMode::HitTrace:
		{
			FString DeliveryDebugMessage;
			const FMABSResolvedAbilityTarget ResolvedTarget = ExecuteHitTraceDelivery(
				AbilitySpec,
				DeliveryDebugMessage,
				bNotifyOwningClient);
			if (ResolvedTarget.TargetActor == nullptr)
			{
				AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
				AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::DeliveryFailed;
				ClearAbilityExecutionContext(AbilitySpec.Handle, true);

				const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
					MABSAbilityComponentEventNames::DeliveryFailed,
					AbilitySpec.AbilityTag,
					AbilitySpec.Handle,
					AbilitySpec.RuntimeState,
					EMABSAbilityActivationResult::DeliveryFailed,
					DeliveryDebugMessage);
				if (bNotifyOwningClient)
				{
					EmitDebugEventToOwningClient(DebugEvent);
				}

				return EMABSAbilityActivationResult::DeliveryFailed;
			}

			return CompleteResolvedTargetAbility(AbilitySpec, ResolvedTarget, DeliveryMode, bNotifyOwningClient);
		}

	case EMABSDeliveryMode::Melee:
		{
			FString DeliveryDebugMessage;
			const FMABSResolvedAbilityTarget ResolvedTarget = ExecuteMeleeDelivery(
				AbilitySpec,
				DeliveryDebugMessage,
				bNotifyOwningClient);
			if (ResolvedTarget.TargetActor == nullptr)
			{
				// Melee swings can still commit on a whiff so recovery, combo windows, and queued follow-ups survive.
				return FinalizeAbilityCommit(AbilitySpec, DeliveryMode, bNotifyOwningClient);
			}

			return CompleteResolvedTargetAbility(AbilitySpec, ResolvedTarget, DeliveryMode, bNotifyOwningClient);
		}

	case EMABSDeliveryMode::Projectile:
		return ExecuteProjectileDelivery(AbilitySpec, bNotifyOwningClient);

	default:
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::InvalidAbility;
		ClearAbilityExecutionContext(AbilitySpec.Handle, true);

		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::DeliveryFailed,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::InvalidAbility,
			TEXT("CommitAbility failed because the authored delivery mode is not supported."));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DebugEvent);
		}

		return EMABSAbilityActivationResult::InvalidAbility;
	}
}

FMABSResolvedAbilityTarget UMABSAbilityComponent::ExecuteDirectDelivery(
	const FMABSAbilitySpec& AbilitySpec,
	FString& OutDebugMessage,
	const bool bNotifyOwningClient)
{
	FMABSResolvedAbilityTarget ResolvedTarget;
	TriggerDeliveryPresentation(AbilitySpec, EMABSDeliveryMode::Direct, bNotifyOwningClient);
	ResolvedTarget.TargetActor = ResolveAbilityTarget(AbilitySpec, OutDebugMessage, bNotifyOwningClient);
	if (ResolvedTarget.TargetActor != nullptr)
	{
		ResolvedTarget.ImpactHitResult.Location = ResolvedTarget.TargetActor->GetActorLocation();
		ResolvedTarget.ImpactHitResult.ImpactPoint = ResolvedTarget.TargetActor->GetActorLocation();
		ResolvedTarget.ImpactHitResult.ImpactNormal = GetOwner() != nullptr
			? -GetOwner()->GetActorForwardVector()
			: FVector::UpVector;
		ResolvedTarget.bHasImpactHitResult = true;
	}

	return ResolvedTarget;
}

FMABSResolvedAbilityTarget UMABSAbilityComponent::ExecuteHitTraceDelivery(
	const FMABSAbilitySpec& AbilitySpec,
	FString& OutDebugMessage,
	const bool bNotifyOwningClient)
{
	FMABSResolvedAbilityTarget ResolvedTarget;
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	if (AbilityDefinition == nullptr)
	{
		OutDebugMessage = TEXT("HitTrace delivery failed because the ability definition was invalid.");
		return ResolvedTarget;
	}

	FTransform OriginTransform = FTransform::Identity;
	FString OriginDescription;
	if (!ResolveDeliveryOriginTransform(
		*AbilityDefinition,
		EMABSDeliveryMode::HitTrace,
		OriginTransform,
		OriginDescription,
		bNotifyOwningClient))
	{
		OutDebugMessage = TEXT("HitTrace delivery failed because no valid delivery origin could be resolved.");
		return ResolvedTarget;
	}

	TriggerDeliveryPresentation(AbilitySpec, EMABSDeliveryMode::HitTrace, bNotifyOwningClient);

	FVector AimTraceStart = FVector::ZeroVector;
	FRotator TraceRotation = OriginTransform.GetRotation().Rotator();
	FString ViewPointDescription = OriginDescription;
	if (GetTargetTraceViewPoint(AimTraceStart, TraceRotation, ViewPointDescription))
	{
		ViewPointDescription = FString::Printf(TEXT("%s + %sAim"), *OriginDescription, *ViewPointDescription);
	}
	else
	{
		ViewPointDescription = FString::Printf(TEXT("%s + OriginRotation"), *OriginDescription);
	}

	const FVector TraceStart = OriginTransform.GetLocation();
	const FVector TraceEnd = TraceStart + (TraceRotation.Vector() * AbilityDefinition->HitTraceDistance);
	FVector TracerEndPoint = TraceEnd;
	ResolvedTarget.TargetActor = ResolveActorTargetFromTrace(
		AbilitySpec,
		TraceStart,
		TraceEnd,
		EMABSDeliveryMode::HitTrace,
		GetTraceModeForRadius(AbilityDefinition->HitTraceRadius),
		AbilityDefinition->HitTraceRadius,
		false,
		TEXT("HitTrace delivery"),
		ViewPointDescription,
		NAME_None,
		MABSAbilityComponentEventNames::HitTraceHit,
		MABSAbilityComponentEventNames::HitTraceRejected,
		OutDebugMessage,
		&ResolvedTarget.ImpactHitResult,
		&TracerEndPoint,
		bNotifyOwningClient);
	ResolvedTarget.bHasImpactHitResult = ResolvedTarget.TargetActor != nullptr;
	TriggerTracerPresentation(AbilitySpec, TraceStart, TracerEndPoint, bNotifyOwningClient);
	return ResolvedTarget;
}

FMABSResolvedAbilityTarget UMABSAbilityComponent::ExecuteMeleeDelivery(
	const FMABSAbilitySpec& AbilitySpec,
	FString& OutDebugMessage,
	const bool bNotifyOwningClient)
{
	FMABSResolvedAbilityTarget ResolvedTarget;
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	if (AbilityDefinition == nullptr)
	{
		OutDebugMessage = TEXT("Melee delivery failed because the ability definition was invalid.");
		return ResolvedTarget;
	}

	FTransform OriginTransform = FTransform::Identity;
	FString OriginDescription;
	if (!ResolveDeliveryOriginTransform(
		*AbilityDefinition,
		EMABSDeliveryMode::Melee,
		OriginTransform,
		OriginDescription,
		bNotifyOwningClient))
	{
		OutDebugMessage = TEXT("Melee delivery failed because no valid delivery origin could be resolved.");
		return ResolvedTarget;
	}

	TriggerDeliveryPresentation(AbilitySpec, EMABSDeliveryMode::Melee, bNotifyOwningClient);

	const FVector ForwardVector = ResolveMeleeTraceDirection();
	const FVector TraceStart = OriginTransform.GetLocation() + (ForwardVector * AbilityDefinition->MeleeForwardOffset);
	const FVector TraceEnd = TraceStart + (ForwardVector * AbilityDefinition->MeleeRange);
	FVector UnusedTraceEndPoint = TraceEnd;
	ResolvedTarget.TargetActor = ResolveActorTargetFromTrace(
		AbilitySpec,
		TraceStart,
		TraceEnd,
		EMABSDeliveryMode::Melee,
		EMABSTargetTraceMode::Sphere,
		AbilityDefinition->MeleeRadius,
		false,
		TEXT("Melee delivery"),
		FString::Printf(TEXT("%s + OwnerFacing"), *OriginDescription),
		NAME_None,
		MABSAbilityComponentEventNames::MeleeHit,
		MABSAbilityComponentEventNames::MeleeRejected,
		OutDebugMessage,
		&ResolvedTarget.ImpactHitResult,
		&UnusedTraceEndPoint,
		bNotifyOwningClient);
	ResolvedTarget.bHasImpactHitResult = ResolvedTarget.TargetActor != nullptr;
	return ResolvedTarget;
}

EMABSAbilityActivationResult UMABSAbilityComponent::ExecuteProjectileDelivery(
	FMABSAbilitySpec& AbilitySpec,
	const bool bNotifyOwningClient)
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	UWorld* const World = GetWorld();
	if (AbilityDefinition == nullptr || World == nullptr)
	{
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::DeliveryFailed;
		ClearAbilityExecutionContext(AbilitySpec.Handle, true);

		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::ProjectileSpawnFailed,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::DeliveryFailed,
			TEXT("Projectile delivery failed because the world or ability definition was unavailable."));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DebugEvent);
		}

		const FMABSAbilityDebugEvent DeliveryFailedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::DeliveryFailed,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::DeliveryFailed,
			TEXT("Projectile delivery failed before the projectile could spawn."));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DeliveryFailedEvent);
		}

		return EMABSAbilityActivationResult::DeliveryFailed;
	}

	FTransform SpawnTransform = FTransform::Identity;
	FString SpawnTransformMessage;
	if (!GetProjectileSpawnTransform(*AbilityDefinition, SpawnTransform, SpawnTransformMessage, bNotifyOwningClient))
	{
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::DeliveryFailed;
		ClearAbilityExecutionContext(AbilitySpec.Handle, true);

		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::ProjectileSpawnFailed,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::DeliveryFailed,
			SpawnTransformMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DebugEvent);
		}

		const FMABSAbilityDebugEvent DeliveryFailedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::DeliveryFailed,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::DeliveryFailed,
			SpawnTransformMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DeliveryFailedEvent);
		}

		return EMABSAbilityActivationResult::DeliveryFailed;
	}

	TriggerDeliveryPresentation(AbilitySpec, EMABSDeliveryMode::Projectile, bNotifyOwningClient);
	AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Active;

	AMABSProjectileBase* Projectile = World->SpawnActorDeferred<AMABSProjectileBase>(
		AbilityDefinition->ProjectileActorClass,
		SpawnTransform,
		GetOwner(),
		GetOwner() != nullptr ? GetOwner()->GetInstigator() : nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
	if (Projectile == nullptr)
	{
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::DeliveryFailed;
		ClearAbilityExecutionContext(AbilitySpec.Handle, true);

		const FString SpawnFailedMessage = FString::Printf(
			TEXT("Projectile delivery failed because class '%s' could not be spawned. %s"),
			*GetNameSafe(AbilityDefinition->ProjectileActorClass),
			*SpawnTransformMessage);
		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::ProjectileSpawnFailed,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::DeliveryFailed,
			SpawnFailedMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DebugEvent);
		}

		const FMABSAbilityDebugEvent DeliveryFailedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::DeliveryFailed,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::DeliveryFailed,
			SpawnFailedMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DeliveryFailedEvent);
		}

		return EMABSAbilityActivationResult::DeliveryFailed;
	}

	Projectile->InitializeProjectile(this, GetOwner(), AbilitySpec.AbilityDefinition, AbilitySpec.AbilityTag, AbilitySpec.Handle);
	UGameplayStatics::FinishSpawningActor(Projectile, SpawnTransform);

	const FMABSAbilityDebugEvent ProjectileSpawnedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::ProjectileSpawned,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Spawned projectile '%s' for ability '%s' at %s."),
			*GetNameSafe(Projectile),
			*GetAbilityLabel(AbilityDefinition),
			*FormatVectorForDebug(SpawnTransform.GetLocation())));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(ProjectileSpawnedEvent);
	}

	if (AbilityDefinition->DeliveryPresentation.ProjectileTravel.HasAnyPresentation())
	{
		const FMABSAbilityDebugEvent CueRoutedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::PresentationCueRouted,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::Success,
			FString::Printf(
				TEXT("Routed ProjectileTravel cue with policy %s through the replicated projectile '%s'. Assets: VFX=%s, SFX=%s."),
				*GetPresentationVisibilityPolicyLabel(AbilityDefinition->DeliveryPresentation.ProjectileTravel.VisibilityPolicy),
				*GetNameSafe(Projectile),
				*GetNameSafe(AbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelVFX),
				*GetNameSafe(AbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelSFX)));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(CueRoutedEvent);
		}

		const FMABSAbilityDebugEvent TravelPresentationEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::ProjectileTravelPresentationTriggered,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::Success,
			FString::Printf(
				TEXT("Hooked projectile travel cue for projectile '%s' with policy %s. Assets: VFX=%s, SFX=%s."),
				*GetNameSafe(Projectile),
				*GetPresentationVisibilityPolicyLabel(AbilityDefinition->DeliveryPresentation.ProjectileTravel.VisibilityPolicy),
				*GetNameSafe(AbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelVFX),
				*GetNameSafe(AbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelSFX)));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(TravelPresentationEvent);
		}
	}

	return FinalizeAbilityCommit(AbilitySpec, EMABSDeliveryMode::Projectile, bNotifyOwningClient);
}

EMABSAbilityActivationResult UMABSAbilityComponent::CompleteResolvedTargetAbility(
	FMABSAbilitySpec& AbilitySpec,
	const FMABSResolvedAbilityTarget& ResolvedTarget,
	const EMABSDeliveryMode DeliveryMode,
	const bool bNotifyOwningClient)
{
	AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Active;

	TArray<AActor*> TargetActors;
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	FString TargetResolutionMessage;
	if (AbilityDefinition != nullptr && AbilityDefinition->AoE.IsValid())
	{
		if (!ResolveAoETargets(AbilitySpec, ResolvedTarget, TargetActors, TargetResolutionMessage, bNotifyOwningClient))
		{
			const EMABSAbilityActivationResult FailureResult = DeliveryMode == EMABSDeliveryMode::Direct
				? EMABSAbilityActivationResult::TargetResolutionFailed
				: EMABSAbilityActivationResult::DeliveryFailed;
			const FName FailureEventName = DeliveryMode == EMABSDeliveryMode::Direct
				? MABSAbilityComponentEventNames::TargetResolutionFailed
				: MABSAbilityComponentEventNames::DeliveryFailed;

			AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
			AbilitySpec.LastActivationResult = FailureResult;
			ClearAbilityExecutionContext(AbilitySpec.Handle, true);

			const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
				FailureEventName,
				AbilitySpec.AbilityTag,
				AbilitySpec.Handle,
				AbilitySpec.RuntimeState,
				FailureResult,
				TargetResolutionMessage);
			if (bNotifyOwningClient)
			{
				EmitDebugEventToOwningClient(DebugEvent);
			}

			return FailureResult;
		}
	}
	else if (ResolvedTarget.TargetActor != nullptr)
	{
		TargetActors.Add(ResolvedTarget.TargetActor);
	}

	TArray<AActor*> AffectedTargets;
	FString EffectFailureMessage;
	if (!ApplyGameplayEffectsToTargets(AbilitySpec, TargetActors, AffectedTargets, EffectFailureMessage, bNotifyOwningClient))
	{
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::EffectApplicationFailed;
		ClearAbilityExecutionContext(AbilitySpec.Handle, true);

		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::EffectApplicationFailed,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::EffectApplicationFailed,
			EffectFailureMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DebugEvent);
		}

		return EMABSAbilityActivationResult::EffectApplicationFailed;
	}

	TriggerImpactPresentation(AbilitySpec, ResolvedTarget, DeliveryMode, bNotifyOwningClient);
	return FinalizeAbilityCommit(AbilitySpec, DeliveryMode, bNotifyOwningClient);
}

EMABSAbilityActivationResult UMABSAbilityComponent::FinalizeAbilityCommit(
	FMABSAbilitySpec& AbilitySpec,
	const EMABSDeliveryMode DeliveryMode,
	const bool bNotifyOwningClient)
{
	FString CostDebugMessage;
	const EMABSAbilityActivationResult CostSpendResult = SpendAbilityCost(AbilitySpec, CostDebugMessage);
	if (CostSpendResult != EMABSAbilityActivationResult::Success)
	{
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		AbilitySpec.LastActivationResult = CostSpendResult;
		ClearAbilityExecutionContext(AbilitySpec.Handle, true);

		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::CostRejected,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			CostSpendResult,
			CostDebugMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DebugEvent);
		}

		return CostSpendResult;
	}

	if (ShouldValidateAbilityCost(AbilitySpec))
	{
		const FMABSAbilityDebugEvent CostSpentEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::CostSpent,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::Success,
			CostDebugMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(CostSpentEvent);
		}
	}

	StartAbilityCooldowns(AbilitySpec, bNotifyOwningClient);
	AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::Success;

	const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::CommitSucceeded,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Ability activation committed successfully using %s delivery."),
			*GetDeliveryModeLabel(DeliveryMode)));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(DebugEvent);
	}

	BeginAbilityRecovery(AbilitySpec, bNotifyOwningClient);

	return EMABSAbilityActivationResult::Success;
}

AActor* UMABSAbilityComponent::ResolveAbilityTarget(
	const FMABSAbilitySpec& AbilitySpec,
	FString& OutDebugMessage,
	const bool bNotifyOwningClient)
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	AActor* const Owner = GetOwner();
	if (AbilityDefinition == nullptr || Owner == nullptr)
	{
		OutDebugMessage = TEXT("Failed to resolve a target because the ability definition or owning actor was invalid.");
		return nullptr;
	}

	switch (AbilityDefinition->TargetType)
	{
	case EMABSTargetType::Self:
		OutDebugMessage = FString::Printf(
			TEXT("Resolved target '%s' using target type '%s'."),
			*GetNameSafe(Owner),
			*GetTargetTypeLabel(AbilityDefinition->TargetType));
		return Owner;

	case EMABSTargetType::Actor:
		{
			FVector TraceStart = FVector::ZeroVector;
			FRotator TraceRotation = FRotator::ZeroRotator;
			FString ViewPointDescription;
			if (!GetTargetTraceViewPoint(TraceStart, TraceRotation, ViewPointDescription))
			{
				OutDebugMessage = TEXT("Failed to resolve an actor target because the trace viewpoint was unavailable.");
				return nullptr;
			}

			FVector GameplayTraceStart = TraceStart;
			FRotator UnusedTraceRotation = TraceRotation;
			Owner->GetActorEyesViewPoint(GameplayTraceStart, UnusedTraceRotation);
			const FVector TraceEnd = GameplayTraceStart + (TraceRotation.Vector() * AbilityDefinition->TargetTraceDistance);
			AActor* const ResolvedActor = ResolveActorTargetFromTrace(
				AbilitySpec,
				GameplayTraceStart,
				TraceEnd,
				EMABSDeliveryMode::Direct,
				AbilityDefinition->ActorTargetTraceMode,
				AbilityDefinition->ActorTargetTraceMode == EMABSTargetTraceMode::Sphere
					? AbilityDefinition->TargetTraceRadius
					: 0.0f,
				AbilityDefinition->bIgnoreNonTargetWorldHits,
				TEXT("Direct target trace"),
				FString::Printf(TEXT("%sAim + OwnerEyesStart"), *ViewPointDescription),
				MABSAbilityComponentEventNames::TargetTraceStarted,
				MABSAbilityComponentEventNames::TargetTraceHit,
				MABSAbilityComponentEventNames::TargetTraceRejected,
				OutDebugMessage,
				nullptr,
				nullptr,
				bNotifyOwningClient);

			if (ResolvedActor != nullptr)
			{
				OutDebugMessage = FString::Printf(
					TEXT("Resolved target '%s' using target type '%s'. %s"),
					*GetNameSafe(ResolvedActor),
					*GetTargetTypeLabel(AbilityDefinition->TargetType),
					*OutDebugMessage);
			}

			return ResolvedActor;
		}

	default:
		OutDebugMessage = FString::Printf(
			TEXT("Failed to resolve a target because target type '%s' is not supported for direct delivery."),
			*GetTargetTypeLabel(AbilityDefinition->TargetType));
		return nullptr;
	}
}

AActor* UMABSAbilityComponent::ResolveActorTargetFromTrace(
	const FMABSAbilitySpec& AbilitySpec,
	const FVector& TraceStart,
	const FVector& TraceEnd,
	const EMABSDeliveryMode DeliveryMode,
	const EMABSTargetTraceMode TraceMode,
	const float TraceRadius,
	const bool bIgnoreNonTargetWorldHits,
	const FString& TraceLabel,
	const FString& ViewPointDescription,
	const FName TraceStartedEventName,
	const FName TraceHitEventName,
	const FName TraceRejectedEventName,
	FString& OutDebugMessage,
	FHitResult* OutAcceptedHitResult,
	FVector* OutTraceEndPoint,
	const bool bNotifyOwningClient)
{
	if (OutAcceptedHitResult != nullptr)
	{
		*OutAcceptedHitResult = FHitResult();
	}

	if (OutTraceEndPoint != nullptr)
	{
		*OutTraceEndPoint = TraceEnd;
	}

	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	AActor* const Owner = GetOwner();
	UWorld* const World = GetWorld();
	if (AbilityDefinition == nullptr || Owner == nullptr || World == nullptr)
	{
		OutDebugMessage = FString::Printf(TEXT("%s failed because the world, owner, or ability definition was invalid."), *TraceLabel);
		return nullptr;
	}

	FMABSTargetTraceDebugInfo TraceDebugInfo;
	TraceDebugInfo.bHasTraceData = true;
	TraceDebugInfo.AbilityTag = AbilitySpec.AbilityTag;
	TraceDebugInfo.AbilityHandle = AbilitySpec.Handle;
	TraceDebugInfo.DeliveryMode = DeliveryMode;
	TraceDebugInfo.TraceMode = TraceMode;
	TraceDebugInfo.TraceStart = TraceStart;
	TraceDebugInfo.TraceEnd = TraceEnd;
	TraceDebugInfo.TraceRadius = TraceMode == EMABSTargetTraceMode::Sphere ? TraceRadius : 0.0f;
	TraceDebugInfo.WorldTimeSeconds = World->GetTimeSeconds();
	TraceDebugInfo.TraceLabel = TraceLabel;
	TraceDebugInfo.ViewPointDescription = ViewPointDescription;
	const EMABSAbilityActivationResult TraceFailureResult =
		DeliveryMode == EMABSDeliveryMode::Direct
			? EMABSAbilityActivationResult::TargetResolutionFailed
			: EMABSAbilityActivationResult::DeliveryFailed;

	if (TraceStartedEventName != NAME_None)
	{
		const FMABSAbilityDebugEvent TraceStartedEvent = EmitDebugEvent(
			TraceStartedEventName,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::Success,
			FString::Printf(
				TEXT("Started %s using %s trace from %s to %s with viewpoint '%s' and radius %.0f."),
				*TraceLabel,
				*GetTargetTraceModeLabel(TraceMode),
				*FormatVectorForDebug(TraceStart),
				*FormatVectorForDebug(TraceEnd),
				*ViewPointDescription,
				TraceDebugInfo.TraceRadius));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(TraceStartedEvent);
		}
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MABSAbilityDeliveryTrace), false);
	QueryParams.AddIgnoredActor(Owner);
	const FCollisionObjectQueryParams ObjectQueryParams = MakeTargetTraceObjectQueryParams(bIgnoreNonTargetWorldHits);

	TArray<FHitResult> HitResults;
	if (TraceMode == EMABSTargetTraceMode::Sphere && TraceRadius > 0.0f)
	{
		const FCollisionShape TraceShape = FCollisionShape::MakeSphere(TraceRadius);
		World->SweepMultiByObjectType(
			HitResults,
			TraceStart,
			TraceEnd,
			(TraceEnd - TraceStart).GetSafeNormal().ToOrientationQuat(),
			ObjectQueryParams,
			TraceShape,
			QueryParams);
	}
	else
	{
		World->LineTraceMultiByObjectType(
			HitResults,
			TraceStart,
			TraceEnd,
			ObjectQueryParams,
			QueryParams);
	}

	if (HitResults.IsEmpty())
	{
		TraceDebugInfo.ResultMessage = FString::Printf(
			TEXT("%s found no blocking hit between %s and %s."),
			*TraceLabel,
			*FormatVectorForDebug(TraceStart),
			*FormatVectorForDebug(TraceEnd));
		RecordLatestTargetTraceDebugInfo(TraceDebugInfo);
		if (bNotifyOwningClient)
		{
			EmitLatestTargetTraceDebugInfoToOwningClient(TraceDebugInfo);
		}
		DrawTargetTraceDebug(*AbilityDefinition, TraceDebugInfo);
		OutDebugMessage = TraceDebugInfo.ResultMessage;
		return nullptr;
	}

	for (const FHitResult& HitResult : HitResults)
	{
		AActor* const HitActor = HitResult.GetActor();
		if (OutTraceEndPoint != nullptr)
		{
			*OutTraceEndPoint = HitResult.ImpactPoint;
		}

		TraceDebugInfo.bHit = true;
		TraceDebugInfo.HitLocation = HitResult.ImpactPoint;
		TraceDebugInfo.HitDistance = FVector::Distance(TraceStart, HitResult.ImpactPoint);
		TraceDebugInfo.HitActorName = GetHitActorLabel(HitActor);
		TraceDebugInfo.HitComponentName = GetNameSafe(HitResult.GetComponent());
		TraceDebugInfo.WorldTimeSeconds = World->GetTimeSeconds();

		if (HitActor == nullptr)
		{
			TraceDebugInfo.bAcceptedTarget = false;
			TraceDebugInfo.ResultMessage = bIgnoreNonTargetWorldHits
				? FString::Printf(
					TEXT("%s rejected world hit on component '%s' at %.0f units and continued searching."),
					*TraceLabel,
					*TraceDebugInfo.HitComponentName,
					TraceDebugInfo.HitDistance)
				: FString::Printf(
					TEXT("%s rejected world hit on component '%s' at %.0f units."),
					*TraceLabel,
					*TraceDebugInfo.HitComponentName,
					TraceDebugInfo.HitDistance);

			const FMABSAbilityDebugEvent TraceRejectedEvent = EmitDebugEvent(
				TraceRejectedEventName,
				AbilitySpec.AbilityTag,
				AbilitySpec.Handle,
				AbilitySpec.RuntimeState,
				TraceFailureResult,
				TraceDebugInfo.ResultMessage);
			if (bNotifyOwningClient)
			{
				EmitDebugEventToOwningClient(TraceRejectedEvent);
			}

			RecordLatestTargetTraceDebugInfo(TraceDebugInfo);
			if (bNotifyOwningClient)
			{
				EmitLatestTargetTraceDebugInfoToOwningClient(TraceDebugInfo);
			}

			if (bIgnoreNonTargetWorldHits)
			{
				continue;
			}

			DrawTargetTraceDebug(*AbilityDefinition, TraceDebugInfo);
			OutDebugMessage = TraceDebugInfo.ResultMessage;
			return nullptr;
		}

		FString RejectionReason;
		const bool bRequireValidActorTarget =
			DeliveryMode == EMABSDeliveryMode::Direct ? AbilityDefinition->bRequireValidActorTarget : true;
		if (!ValidateTargetActorForAbility(AbilityDefinition, Owner, HitActor, bRequireValidActorTarget, false, RejectionReason))
		{
			TraceDebugInfo.bAcceptedTarget = false;
			TraceDebugInfo.ResultMessage = FString::Printf(
				TEXT("%s rejected actor '%s' on component '%s' at %.0f units: %s"),
				*TraceLabel,
				*TraceDebugInfo.HitActorName,
				*TraceDebugInfo.HitComponentName,
				TraceDebugInfo.HitDistance,
				*RejectionReason);

			const FMABSAbilityDebugEvent TraceRejectedEvent = EmitDebugEvent(
				TraceRejectedEventName,
				AbilitySpec.AbilityTag,
				AbilitySpec.Handle,
				AbilitySpec.RuntimeState,
				TraceFailureResult,
				TraceDebugInfo.ResultMessage);
			if (bNotifyOwningClient)
			{
				EmitDebugEventToOwningClient(TraceRejectedEvent);
			}

			RecordLatestTargetTraceDebugInfo(TraceDebugInfo);
			if (bNotifyOwningClient)
			{
				EmitLatestTargetTraceDebugInfoToOwningClient(TraceDebugInfo);
			}
			DrawTargetTraceDebug(*AbilityDefinition, TraceDebugInfo);
			OutDebugMessage = TraceDebugInfo.ResultMessage;
			return nullptr;
		}

		TraceDebugInfo.bAcceptedTarget = true;
		TraceDebugInfo.ResultMessage = FString::Printf(
			TEXT("%s accepted actor '%s' on component '%s' at %.0f units using %s trace."),
			*TraceLabel,
			*TraceDebugInfo.HitActorName,
			*TraceDebugInfo.HitComponentName,
			TraceDebugInfo.HitDistance,
			*GetTargetTraceModeLabel(TraceMode));

		const FMABSAbilityDebugEvent TraceHitEvent = EmitDebugEvent(
			TraceHitEventName,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::Success,
			TraceDebugInfo.ResultMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(TraceHitEvent);
		}

		RecordLatestTargetTraceDebugInfo(TraceDebugInfo);
		if (bNotifyOwningClient)
		{
			EmitLatestTargetTraceDebugInfoToOwningClient(TraceDebugInfo);
		}

		if (OutAcceptedHitResult != nullptr)
		{
			*OutAcceptedHitResult = HitResult;
		}
		DrawTargetTraceDebug(*AbilityDefinition, TraceDebugInfo);
		OutDebugMessage = TraceDebugInfo.ResultMessage;
		return HitActor;
	}

	DrawTargetTraceDebug(*AbilityDefinition, TraceDebugInfo);
	OutDebugMessage = TraceDebugInfo.ResultMessage.IsEmpty()
		? FString::Printf(TEXT("%s failed to resolve a valid actor target."), *TraceLabel)
		: TraceDebugInfo.ResultMessage;
	return nullptr;
}

EMABSAbilityActivationResult UMABSAbilityComponent::ApplyInstantEffect(
	const FMABSAbilitySpec& AbilitySpec,
	AActor* TargetActor,
	FString& OutDebugMessage) const
{
	return ApplyInstantEffectFromSource(AbilitySpec.AbilityDefinition, GetOwner(), TargetActor, OutDebugMessage);
}

EMABSAbilityActivationResult UMABSAbilityComponent::ApplyInstantEffectFromSource(
	const UMABSAbilityDefinition* AbilityDefinition,
	AActor* SourceActor,
	AActor* TargetActor,
	FString& OutDebugMessage) const
{
	if (AbilityDefinition == nullptr || SourceActor == nullptr || TargetActor == nullptr)
	{
		OutDebugMessage = TEXT("Failed to apply the instant effect because the ability definition, source actor, or target was invalid.");
		return EMABSAbilityActivationResult::EffectApplicationFailed;
	}

	switch (AbilityDefinition->InstantEffectType)
	{
	case EMABSInstantEffectType::Damage:
		{
			const float AppliedDamage = UGameplayStatics::ApplyDamage(
				TargetActor,
				AbilityDefinition->EffectMagnitude,
				SourceActor->GetInstigatorController(),
				SourceActor,
				UDamageType::StaticClass());

			if (AppliedDamage <= 0.0f)
			{
				OutDebugMessage = FString::Printf(
					TEXT("Failed to apply instant effect '%s' to '%s'."),
					*GetInstantEffectTypeLabel(AbilityDefinition->InstantEffectType),
					*GetNameSafe(TargetActor));
				return EMABSAbilityActivationResult::EffectApplicationFailed;
			}

			OutDebugMessage = FString::Printf(
				TEXT("Applied instant effect '%s' with magnitude %.2f to '%s'."),
				*GetInstantEffectTypeLabel(AbilityDefinition->InstantEffectType),
				AppliedDamage,
				*GetNameSafe(TargetActor));
			return EMABSAbilityActivationResult::Success;
		}

	case EMABSInstantEffectType::Heal:
		{
			if (!TargetActor->GetClass()->ImplementsInterface(UMABSInstantEffectReceiver::StaticClass()))
			{
				OutDebugMessage = FString::Printf(
					TEXT("Failed to apply instant effect '%s' to '%s' because the target does not implement IMABSInstantEffectReceiver."),
					*GetInstantEffectTypeLabel(AbilityDefinition->InstantEffectType),
					*GetNameSafe(TargetActor));
				return EMABSAbilityActivationResult::EffectApplicationFailed;
			}

			const bool bAppliedHeal = IMABSInstantEffectReceiver::Execute_ApplyMABSHeal(
				TargetActor,
				AbilityDefinition->EffectMagnitude,
				SourceActor,
				const_cast<UMABSAbilityDefinition*>(AbilityDefinition));
			if (!bAppliedHeal)
			{
				OutDebugMessage = FString::Printf(
					TEXT("Failed to apply instant effect '%s' to '%s' because the target rejected the heal."),
					*GetInstantEffectTypeLabel(AbilityDefinition->InstantEffectType),
					*GetNameSafe(TargetActor));
				return EMABSAbilityActivationResult::EffectApplicationFailed;
			}

			OutDebugMessage = FString::Printf(
				TEXT("Applied instant effect '%s' with magnitude %.2f to '%s'."),
				*GetInstantEffectTypeLabel(AbilityDefinition->InstantEffectType),
				AbilityDefinition->EffectMagnitude,
				*GetNameSafe(TargetActor));
			return EMABSAbilityActivationResult::Success;
		}

	default:
		OutDebugMessage = FString::Printf(
			TEXT("Failed to apply the instant effect because effect type '%s' is not supported."),
			*GetInstantEffectTypeLabel(AbilityDefinition->InstantEffectType));
		return EMABSAbilityActivationResult::EffectApplicationFailed;
	}
}

EMABSAbilityActivationResult UMABSAbilityComponent::ApplyPeriodicEffect(
	const FMABSAbilitySpec& AbilitySpec,
	AActor* TargetActor,
	const bool bNotifyOwningClient)
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	AActor* const SourceActor = GetOwner();
	UWorld* const World = GetWorld();
	if (AbilityDefinition == nullptr || SourceActor == nullptr || TargetActor == nullptr || World == nullptr)
	{
		return EMABSAbilityActivationResult::EffectApplicationFailed;
	}

	if (!HasPeriodicGameplayEffect(AbilityDefinition))
	{
		return EMABSAbilityActivationResult::EffectApplicationFailed;
	}

	const float CurrentWorldTime = World->GetTimeSeconds();
	const float ExpirationWorldTime = CurrentWorldTime + AbilityDefinition->PeriodicEffect.Duration;
	FMABSPeriodicEffectRuntime* ExistingRuntime = FindPeriodicEffectRuntime(AbilitySpec, TargetActor);
	const bool bWasRefresh = ExistingRuntime != nullptr;
	if (ExistingRuntime == nullptr)
	{
		FMABSPeriodicEffectRuntime NewRuntime;
		NewRuntime.State.RuntimeId = MakeNextPeriodicEffectRuntimeId();
		ExistingRuntime = &ActivePeriodicEffectRuntimes.Add(NewRuntime.State.RuntimeId, MoveTemp(NewRuntime));
	}

	if (ExistingRuntime == nullptr)
	{
		return EMABSAbilityActivationResult::EffectApplicationFailed;
	}

	if (bWasRefresh)
	{
		World->GetTimerManager().ClearTimer(ExistingRuntime->TickTimerHandle);
		World->GetTimerManager().ClearTimer(ExistingRuntime->ExpirationTimerHandle);
	}

	ExistingRuntime->State.AbilityTag = AbilitySpec.AbilityTag;
	ExistingRuntime->State.AbilityHandle = AbilitySpec.Handle;
	ExistingRuntime->State.AbilityDefinition = const_cast<UMABSAbilityDefinition*>(AbilityDefinition);
	ExistingRuntime->State.SourceActor = SourceActor;
	ExistingRuntime->State.TargetActor = TargetActor;
	ExistingRuntime->State.EffectType = AbilityDefinition->PeriodicEffect.EffectType;
	ExistingRuntime->State.TickMagnitude = AbilityDefinition->PeriodicEffect.TickMagnitude;
	ExistingRuntime->State.TickInterval = AbilityDefinition->PeriodicEffect.TickInterval;
	ExistingRuntime->State.AppliedWorldTime = CurrentWorldTime;
	ExistingRuntime->State.ExpirationWorldTime = ExpirationWorldTime;
	ExistingRuntime->bNotifyOwningClient = ExistingRuntime->bNotifyOwningClient || bNotifyOwningClient;

	FTimerDelegate TickDelegate = FTimerDelegate::CreateUObject(
		this,
		&UMABSAbilityComponent::TickPeriodicEffect,
		ExistingRuntime->State.RuntimeId);
	World->GetTimerManager().SetTimer(
		ExistingRuntime->TickTimerHandle,
		TickDelegate,
		AbilityDefinition->PeriodicEffect.TickInterval,
		true);

	FTimerDelegate ExpirationDelegate = FTimerDelegate::CreateUObject(
		this,
		&UMABSAbilityComponent::ExpirePeriodicEffect,
		ExistingRuntime->State.RuntimeId);
	World->GetTimerManager().SetTimer(
		ExistingRuntime->ExpirationTimerHandle,
		ExpirationDelegate,
		AbilityDefinition->PeriodicEffect.Duration,
		false);

	RefreshActivePeriodicEffects();

	const FMABSAbilityDebugEvent PeriodicEvent = EmitDebugEvent(
		bWasRefresh
			? MABSAbilityComponentEventNames::PeriodicEffectRefreshed
			: MABSAbilityComponentEventNames::PeriodicEffectApplied,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("%s %s on '%s' for %.2f seconds with %.2f magnitude every %.2f seconds."),
			bWasRefresh ? TEXT("Refreshed") : TEXT("Applied"),
			*GetPeriodicEffectTypeLabel(AbilityDefinition->PeriodicEffect.EffectType),
			*GetNameSafe(TargetActor),
			AbilityDefinition->PeriodicEffect.Duration,
			AbilityDefinition->PeriodicEffect.TickMagnitude,
			AbilityDefinition->PeriodicEffect.TickInterval));
	if (ExistingRuntime->bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(PeriodicEvent);
	}

	return EMABSAbilityActivationResult::Success;
}

EMABSAbilityActivationResult UMABSAbilityComponent::ApplyPeriodicEffectTick(
	const FMABSActivePeriodicEffect& ActiveEffect,
	FString& OutDebugMessage) const
{
	const UMABSAbilityDefinition* const AbilityDefinition = ActiveEffect.AbilityDefinition;
	AActor* const SourceActor = ActiveEffect.SourceActor;
	AActor* const TargetActor = ActiveEffect.TargetActor;
	if (AbilityDefinition == nullptr || SourceActor == nullptr || TargetActor == nullptr)
	{
		OutDebugMessage = TEXT("Periodic effect tick failed because the effect source, target, or definition was invalid.");
		return EMABSAbilityActivationResult::EffectApplicationFailed;
	}

	switch (ActiveEffect.EffectType)
	{
	case EMABSPeriodicEffectType::DOT:
		{
			const float AppliedDamage = UGameplayStatics::ApplyDamage(
				TargetActor,
				ActiveEffect.TickMagnitude,
				SourceActor->GetInstigatorController(),
				SourceActor,
				UDamageType::StaticClass());
			if (AppliedDamage <= 0.0f)
			{
				OutDebugMessage = FString::Printf(
					TEXT("Periodic DOT tick failed on '%s'."),
					*GetNameSafe(TargetActor));
				return EMABSAbilityActivationResult::EffectApplicationFailed;
			}

			OutDebugMessage = FString::Printf(
				TEXT("Periodic DOT tick applied %.2f damage to '%s'."),
				AppliedDamage,
				*GetNameSafe(TargetActor));
			return EMABSAbilityActivationResult::Success;
		}

	case EMABSPeriodicEffectType::HOT:
		{
			if (!TargetActor->GetClass()->ImplementsInterface(UMABSInstantEffectReceiver::StaticClass()))
			{
				OutDebugMessage = FString::Printf(
					TEXT("Periodic HOT tick failed on '%s' because the target does not implement IMABSInstantEffectReceiver."),
					*GetNameSafe(TargetActor));
				return EMABSAbilityActivationResult::EffectApplicationFailed;
			}

			const bool bAppliedHeal = IMABSInstantEffectReceiver::Execute_ApplyMABSHeal(
				TargetActor,
				ActiveEffect.TickMagnitude,
				SourceActor,
				const_cast<UMABSAbilityDefinition*>(AbilityDefinition));
			if (!bAppliedHeal)
			{
				OutDebugMessage = FString::Printf(
					TEXT("Periodic HOT tick failed on '%s' because the target rejected the heal."),
					*GetNameSafe(TargetActor));
				return EMABSAbilityActivationResult::EffectApplicationFailed;
			}

			OutDebugMessage = FString::Printf(
				TEXT("Periodic HOT tick applied %.2f healing to '%s'."),
				ActiveEffect.TickMagnitude,
				*GetNameSafe(TargetActor));
			return EMABSAbilityActivationResult::Success;
		}

	default:
		OutDebugMessage = TEXT("Periodic effect tick failed because the effect type was invalid.");
		return EMABSAbilityActivationResult::EffectApplicationFailed;
	}
}

bool UMABSAbilityComponent::ApplyGameplayEffectsToTargets(
	const FMABSAbilitySpec& AbilitySpec,
	const TArray<AActor*>& TargetActors,
	TArray<AActor*>& OutAffectedTargets,
	FString& OutFailureMessage,
	const bool bNotifyOwningClient)
{
	OutAffectedTargets.Reset();

	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	if (AbilityDefinition == nullptr)
	{
		OutFailureMessage = TEXT("Gameplay effect application failed because the ability definition was invalid.");
		return false;
	}

	if (TargetActors.IsEmpty())
	{
		OutFailureMessage = TEXT("Gameplay effect application failed because no valid targets were resolved.");
		return false;
	}

	const bool bHasInstantEffect = HasInstantGameplayEffect(AbilityDefinition);
	const bool bHasPeriodicEffect = HasPeriodicGameplayEffect(AbilityDefinition);
	FString LastFailureMessage;
	for (AActor* const TargetActor : TargetActors)
	{
		bool bAppliedToTarget = false;
		bool bInstantApplied = false;

		if (bHasInstantEffect)
		{
			FString EffectDebugMessage;
			const EMABSAbilityActivationResult InstantResult = ApplyInstantEffect(AbilitySpec, TargetActor, EffectDebugMessage);
			if (InstantResult == EMABSAbilityActivationResult::Success)
			{
				bAppliedToTarget = true;
				bInstantApplied = true;

				const FMABSAbilityDebugEvent EffectAppliedEvent = EmitDebugEvent(
					MABSAbilityComponentEventNames::EffectApplied,
					AbilitySpec.AbilityTag,
					AbilitySpec.Handle,
					AbilitySpec.RuntimeState,
					EMABSAbilityActivationResult::Success,
					EffectDebugMessage);
				if (bNotifyOwningClient)
				{
					EmitDebugEventToOwningClient(EffectAppliedEvent);
				}
			}
			else
			{
				LastFailureMessage = EffectDebugMessage;

				const FMABSAbilityDebugEvent FailureEvent = EmitDebugEvent(
					MABSAbilityComponentEventNames::EffectApplicationFailed,
					AbilitySpec.AbilityTag,
					AbilitySpec.Handle,
					AbilitySpec.RuntimeState,
					InstantResult,
					EffectDebugMessage);
				if (bNotifyOwningClient)
				{
					EmitDebugEventToOwningClient(FailureEvent);
				}
			}
		}

		if (bHasPeriodicEffect && (!bHasInstantEffect || bInstantApplied))
		{
			const EMABSAbilityActivationResult PeriodicResult = ApplyPeriodicEffect(AbilitySpec, TargetActor, bNotifyOwningClient);
			if (PeriodicResult == EMABSAbilityActivationResult::Success)
			{
				bAppliedToTarget = true;
			}
			else
			{
				LastFailureMessage = FString::Printf(
					TEXT("Failed to apply periodic effect to '%s'."),
					*GetNameSafe(TargetActor));

				const FMABSAbilityDebugEvent FailureEvent = EmitDebugEvent(
					MABSAbilityComponentEventNames::EffectApplicationFailed,
					AbilitySpec.AbilityTag,
					AbilitySpec.Handle,
					AbilitySpec.RuntimeState,
					PeriodicResult,
					LastFailureMessage);
				if (bNotifyOwningClient)
				{
					EmitDebugEventToOwningClient(FailureEvent);
				}
			}
		}

		if (bAppliedToTarget)
		{
			OutAffectedTargets.Add(TargetActor);
		}
	}

	if (OutAffectedTargets.IsEmpty())
	{
		OutFailureMessage = LastFailureMessage.IsEmpty()
			? FString::Printf(
				TEXT("Gameplay effect application failed for resolved targets: %s."),
				*DescribeActorList(TargetActors))
			: LastFailureMessage;
		return false;
	}

	return true;
}

EMABSAbilityActivationResult UMABSAbilityComponent::HandleProjectileImpact(
	AMABSProjectileBase& Projectile,
	AActor* HitActor,
	const FHitResult& HitResult)
{
	UMABSAbilityDefinition* const AbilityDefinition = Projectile.GetSourceAbilityDefinition();
	AActor* const SourceActor = Projectile.GetSourceActor();
	if (AbilityDefinition == nullptr || SourceActor == nullptr)
	{
		return EMABSAbilityActivationResult::DeliveryFailed;
	}

	FMABSAbilitySpec ProjectileSpec;
	ProjectileSpec.AbilityDefinition = AbilityDefinition;
	ProjectileSpec.AbilityTag = Projectile.GetSourceAbilityTag();
	ProjectileSpec.Handle = Projectile.GetSourceAbilityHandle();
	ProjectileSpec.RuntimeState = EMABSAbilityRuntimeState::Idle;

	FMABSResolvedAbilityTarget ResolvedTarget;
	ResolvedTarget.TargetActor = HitActor;
	ResolvedTarget.ImpactHitResult = HitResult;
	ResolvedTarget.bHasImpactHitResult = true;

	TArray<AActor*> TargetActors;
	if (AbilityDefinition->AoE.IsValid())
	{
		FString AoEDebugMessage;
		if (!ResolveAoETargets(ProjectileSpec, ResolvedTarget, TargetActors, AoEDebugMessage, true))
		{
			const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
				MABSAbilityComponentEventNames::ProjectileImpactRejected,
				Projectile.GetSourceAbilityTag(),
				Projectile.GetSourceAbilityHandle(),
				EMABSAbilityRuntimeState::Idle,
				EMABSAbilityActivationResult::DeliveryFailed,
				AoEDebugMessage);
			EmitDebugEventToOwningClient(DebugEvent);
			return EMABSAbilityActivationResult::DeliveryFailed;
		}
	}
	else
	{
		FString RejectionReason;
		if (!ValidateTargetActorForAbility(AbilityDefinition, SourceActor, HitActor, true, false, RejectionReason))
		{
			const FString ImpactDebugMessage = HitActor == nullptr
				? FString::Printf(
					TEXT("Projectile impact was rejected at %s because no valid actor was hit."),
					*FormatVectorForDebug(HitResult.ImpactPoint))
				: FString::Printf(
					TEXT("Projectile impact on '%s' was rejected: %s"),
					*GetNameSafe(HitActor),
					*RejectionReason);

			const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
				MABSAbilityComponentEventNames::ProjectileImpactRejected,
				Projectile.GetSourceAbilityTag(),
				Projectile.GetSourceAbilityHandle(),
				EMABSAbilityRuntimeState::Idle,
				EMABSAbilityActivationResult::DeliveryFailed,
				ImpactDebugMessage);
			EmitDebugEventToOwningClient(DebugEvent);
			return EMABSAbilityActivationResult::DeliveryFailed;
		}

		TargetActors.Add(HitActor);
	}

	TArray<AActor*> AffectedTargets;
	FString EffectFailureMessage;
	if (!ApplyGameplayEffectsToTargets(ProjectileSpec, TargetActors, AffectedTargets, EffectFailureMessage, true))
	{
		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::ProjectileImpactRejected,
			Projectile.GetSourceAbilityTag(),
			Projectile.GetSourceAbilityHandle(),
			EMABSAbilityRuntimeState::Idle,
			EMABSAbilityActivationResult::EffectApplicationFailed,
			EffectFailureMessage);
		EmitDebugEventToOwningClient(DebugEvent);
		return EMABSAbilityActivationResult::EffectApplicationFailed;
	}

	TriggerProjectileImpactPresentation(
		*AbilityDefinition,
		Projectile.GetSourceAbilityTag(),
		Projectile.GetSourceAbilityHandle(),
		HitResult,
		HitActor);

	const FMABSAbilityDebugEvent ProjectileImpactEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::ProjectileImpact,
		Projectile.GetSourceAbilityTag(),
		Projectile.GetSourceAbilityHandle(),
		EMABSAbilityRuntimeState::Idle,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Projectile impacted at %s and affected %d target(s): %s."),
			*FormatVectorForDebug(HitResult.ImpactPoint),
			AffectedTargets.Num(),
			*DescribeActorList(AffectedTargets)));
	EmitDebugEventToOwningClient(ProjectileImpactEvent);

	return EMABSAbilityActivationResult::Success;
}

FMABSAbilitySpec* UMABSAbilityComponent::FindActiveComboSourceSpecForRequest(const FGameplayTag& RequestedAbilityTag)
{
	if (!RequestedAbilityTag.IsValid())
	{
		return nullptr;
	}

	FMABSAbilitySpec* BestMatch = nullptr;
	float BestActivationStartTime = -1.0f;
	for (FMABSAbilitySpec& AbilitySpec : GrantedAbilities)
	{
		if (AbilitySpec.AbilityDefinition == nullptr)
		{
			continue;
		}

		if (AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Idle
			|| AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Blocked)
		{
			continue;
		}

		const bool bMatchesFollowupTag = AbilitySpec.AbilityDefinition->Combo.NextComboAbilityTag == RequestedAbilityTag;
		const bool bMatchesActiveAbilityTag = AbilitySpec.AbilityTag == RequestedAbilityTag;
		const bool bMatchesRoutingTag = AbilitySpec.ComboInputRoutingTag == RequestedAbilityTag;
		if (!bMatchesFollowupTag && !bMatchesActiveAbilityTag && !bMatchesRoutingTag)
		{
			continue;
		}

		if (BestMatch == nullptr || AbilitySpec.ActivationStartTime >= BestActivationStartTime)
		{
			BestMatch = &AbilitySpec;
			BestActivationStartTime = AbilitySpec.ActivationStartTime;
		}
	}

	return BestMatch;
}

FMABSPeriodicEffectRuntime* UMABSAbilityComponent::FindPeriodicEffectRuntime(
	const FMABSAbilitySpec& AbilitySpec,
	AActor* TargetActor)
{
	AActor* const SourceActor = GetOwner();
	for (TPair<int32, FMABSPeriodicEffectRuntime>& Pair : ActivePeriodicEffectRuntimes)
	{
		if (Pair.Value.State.AbilityHandle == AbilitySpec.Handle
			&& Pair.Value.State.TargetActor == TargetActor
			&& Pair.Value.State.SourceActor == SourceActor)
		{
			return &Pair.Value;
		}
	}

	return nullptr;
}

bool UMABSAbilityComponent::HasAuthoredGameplayEffect(const UMABSAbilityDefinition* AbilityDefinition) const
{
	return HasInstantGameplayEffect(AbilityDefinition) || HasPeriodicGameplayEffect(AbilityDefinition);
}

bool UMABSAbilityComponent::HasInstantGameplayEffect(const UMABSAbilityDefinition* AbilityDefinition) const
{
	return AbilityDefinition != nullptr
		&& AbilityDefinition->InstantEffectType != EMABSInstantEffectType::None
		&& AbilityDefinition->EffectMagnitude > 0.0f;
}

bool UMABSAbilityComponent::HasPeriodicGameplayEffect(const UMABSAbilityDefinition* AbilityDefinition) const
{
	return AbilityDefinition != nullptr && AbilityDefinition->PeriodicEffect.IsValid();
}

bool UMABSAbilityComponent::ResolveAoECenterTransform(
	const FMABSResolvedAbilityTarget& ResolvedTarget,
	FTransform& OutCenterTransform) const
{
	if (ResolvedTarget.bHasImpactHitResult)
	{
		const FVector ImpactLocation = !ResolvedTarget.ImpactHitResult.ImpactPoint.IsNearlyZero()
			? ResolvedTarget.ImpactHitResult.ImpactPoint
			: ResolvedTarget.ImpactHitResult.Location;
		const FRotator ImpactRotation = !ResolvedTarget.ImpactHitResult.ImpactNormal.IsNearlyZero()
			? ResolvedTarget.ImpactHitResult.ImpactNormal.Rotation()
			: (GetOwner() != nullptr ? GetOwner()->GetActorRotation() : FRotator::ZeroRotator);
		OutCenterTransform = FTransform(ImpactRotation, ImpactLocation);
		return true;
	}

	if (ResolvedTarget.TargetActor != nullptr)
	{
		OutCenterTransform = FTransform(
			ResolvedTarget.TargetActor->GetActorRotation(),
			ResolvedTarget.TargetActor->GetActorLocation());
		return true;
	}

	if (const AActor* const Owner = GetOwner())
	{
		OutCenterTransform = FTransform(Owner->GetActorRotation(), Owner->GetActorLocation());
		return true;
	}

	return false;
}

bool UMABSAbilityComponent::ResolveAoETargets(
	const FMABSAbilitySpec& AbilitySpec,
	const FMABSResolvedAbilityTarget& ResolvedTarget,
	TArray<AActor*>& OutResolvedTargets,
	FString& OutDebugMessage,
	const bool bNotifyOwningClient)
{
	OutResolvedTargets.Reset();

	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	AActor* const SourceActor = GetOwner();
	UWorld* const World = GetWorld();
	if (AbilityDefinition == nullptr || SourceActor == nullptr || World == nullptr || !AbilityDefinition->AoE.IsValid())
	{
		OutDebugMessage = TEXT("AoE target resolution failed because the ability definition, owner, world, or AoE data was invalid.");
		return false;
	}

	FTransform CenterTransform = FTransform::Identity;
	if (!ResolveAoECenterTransform(ResolvedTarget, CenterTransform))
	{
		OutDebugMessage = TEXT("AoE target resolution failed because no valid AoE center could be resolved.");
		return false;
	}

	CenterTransform.AddToTranslation(CenterTransform.GetRotation().RotateVector(AbilityDefinition->AoE.Offset));

	FCollisionShape AoEShape;
	switch (AbilityDefinition->AoE.Shape)
	{
	case EMABSAoEShape::Box:
		AoEShape = FCollisionShape::MakeBox(AbilityDefinition->AoE.BoxExtent);
		break;

	case EMABSAoEShape::Capsule:
		AoEShape = FCollisionShape::MakeCapsule(
			AbilityDefinition->AoE.CapsuleRadius,
			AbilityDefinition->AoE.CapsuleHalfHeight);
		break;

	case EMABSAoEShape::Sphere:
	default:
		AoEShape = FCollisionShape::MakeSphere(AbilityDefinition->AoE.Radius);
		break;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MABSAoEResolution), false);
	const FCollisionObjectQueryParams ObjectQueryParams = MakeTargetTraceObjectQueryParams(true);

	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByObjectType(
		OverlapResults,
		CenterTransform.GetLocation(),
		CenterTransform.GetRotation(),
		ObjectQueryParams,
		AoEShape,
		QueryParams);

	TArray<AActor*> CandidateActors;
	auto AddUniqueCandidate = [&CandidateActors](AActor* CandidateActor)
	{
		if (CandidateActor != nullptr && !CandidateActors.Contains(CandidateActor))
		{
			CandidateActors.Add(CandidateActor);
		}
	};

	AddUniqueCandidate(ResolvedTarget.TargetActor);
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AddUniqueCandidate(OverlapResult.GetActor());
	}

	const bool bAllowSelfTarget = AbilityDefinition->TargetType == EMABSTargetType::Self;
	for (AActor* const CandidateActor : CandidateActors)
	{
		FString RejectionReason;
		if (!ValidateTargetActorForAbility(AbilityDefinition, SourceActor, CandidateActor, true, bAllowSelfTarget, RejectionReason))
		{
			const FMABSAbilityDebugEvent RejectedEvent = EmitDebugEvent(
				MABSAbilityComponentEventNames::AoETargetRejected,
				AbilitySpec.AbilityTag,
				AbilitySpec.Handle,
				AbilitySpec.RuntimeState,
				EMABSAbilityActivationResult::DeliveryFailed,
				FString::Printf(
					TEXT("AoE rejected actor '%s': %s"),
					*GetNameSafe(CandidateActor),
					*RejectionReason));
			if (bNotifyOwningClient)
			{
				EmitDebugEventToOwningClient(RejectedEvent);
			}
			continue;
		}

		OutResolvedTargets.Add(CandidateActor);
	}

	OutDebugMessage = FString::Printf(
		TEXT("Resolved %d AoE target(s) using %s centered at %s: %s."),
		OutResolvedTargets.Num(),
		*GetAoEShapeLabel(AbilityDefinition->AoE.Shape),
		*FormatVectorForDebug(CenterTransform.GetLocation()),
		*DescribeActorList(OutResolvedTargets));

	const FMABSAbilityDebugEvent ResolvedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::AoEResolved,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		OutDebugMessage);
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(ResolvedEvent);
	}

	if (OutResolvedTargets.IsEmpty())
	{
		OutDebugMessage = FString::Printf(
			TEXT("AoE resolved no valid targets using %s centered at %s."),
			*GetAoEShapeLabel(AbilityDefinition->AoE.Shape),
			*FormatVectorForDebug(CenterTransform.GetLocation()));
		return false;
	}

	return true;
}

void UMABSAbilityComponent::RefreshActivePeriodicEffects()
{
	ActivePeriodicEffects.Reset(ActivePeriodicEffectRuntimes.Num());
	for (const TPair<int32, FMABSPeriodicEffectRuntime>& Pair : ActivePeriodicEffectRuntimes)
	{
		ActivePeriodicEffects.Add(Pair.Value.State);
	}

	ActivePeriodicEffects.Sort([](const FMABSActivePeriodicEffect& Left, const FMABSActivePeriodicEffect& Right)
	{
		return Left.RuntimeId < Right.RuntimeId;
	});
}

void UMABSAbilityComponent::TickPeriodicEffect(const int32 PeriodicEffectRuntimeId)
{
	FMABSPeriodicEffectRuntime* const Runtime = ActivePeriodicEffectRuntimes.Find(PeriodicEffectRuntimeId);
	if (Runtime == nullptr)
	{
		return;
	}

	FString TickDebugMessage;
	const EMABSAbilityActivationResult TickResult = ApplyPeriodicEffectTick(Runtime->State, TickDebugMessage);
	if (TickResult != EMABSAbilityActivationResult::Success)
	{
		const FMABSAbilityDebugEvent ExpiredEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::PeriodicEffectExpired,
			Runtime->State.AbilityTag,
			Runtime->State.AbilityHandle,
			EMABSAbilityRuntimeState::Idle,
			TickResult,
			FString::Printf(
				TEXT("Expired %s on '%s' because a tick failed: %s"),
				*GetPeriodicEffectTypeLabel(Runtime->State.EffectType),
				*GetNameSafe(Runtime->State.TargetActor),
				*TickDebugMessage));
		if (Runtime->bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(ExpiredEvent);
		}

		if (UWorld* const World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(Runtime->TickTimerHandle);
			World->GetTimerManager().ClearTimer(Runtime->ExpirationTimerHandle);
		}

		ActivePeriodicEffectRuntimes.Remove(PeriodicEffectRuntimeId);
		RefreshActivePeriodicEffects();
		return;
	}

	const float CurrentWorldTime = GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float RemainingDuration = FMath::Max(0.0f, Runtime->State.ExpirationWorldTime - CurrentWorldTime);
	const FMABSAbilityDebugEvent TickEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::PeriodicEffectTick,
		Runtime->State.AbilityTag,
		Runtime->State.AbilityHandle,
		EMABSAbilityRuntimeState::Idle,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("%s Remaining duration: %.2f seconds."),
			*TickDebugMessage,
			RemainingDuration));
	if (Runtime->bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(TickEvent);
	}
}

void UMABSAbilityComponent::ExpirePeriodicEffect(const int32 PeriodicEffectRuntimeId)
{
	FMABSPeriodicEffectRuntime* const Runtime = ActivePeriodicEffectRuntimes.Find(PeriodicEffectRuntimeId);
	if (Runtime == nullptr)
	{
		return;
	}

	const FMABSActivePeriodicEffect ExpiringEffect = Runtime->State;
	const bool bNotifyOwningClient = Runtime->bNotifyOwningClient;
	if (UWorld* const World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Runtime->TickTimerHandle);
		World->GetTimerManager().ClearTimer(Runtime->ExpirationTimerHandle);
	}

	ActivePeriodicEffectRuntimes.Remove(PeriodicEffectRuntimeId);
	RefreshActivePeriodicEffects();

	const FMABSAbilityDebugEvent ExpiredEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::PeriodicEffectExpired,
		ExpiringEffect.AbilityTag,
		ExpiringEffect.AbilityHandle,
		EMABSAbilityRuntimeState::Idle,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Expired %s on '%s'."),
			*GetPeriodicEffectTypeLabel(ExpiringEffect.EffectType),
			*GetNameSafe(ExpiringEffect.TargetActor)));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(ExpiredEvent);
	}
}

bool UMABSAbilityComponent::ShouldValidateAbilityCost(const FMABSAbilitySpec& AbilitySpec) const
{
	return AbilitySpec.AbilityDefinition != nullptr && AbilitySpec.AbilityDefinition->ResourceCost > 0.0f;
}

EMABSAbilityActivationResult UMABSAbilityComponent::SpendAbilityCost(
	const FMABSAbilitySpec& AbilitySpec,
	FString& OutDebugMessage) const
{
	if (!ShouldValidateAbilityCost(AbilitySpec))
	{
		OutDebugMessage = TEXT("No resource cost was authored for this ability.");
		return EMABSAbilityActivationResult::Success;
	}

	AActor* const Owner = GetOwner();
	if (Owner == nullptr || !Owner->GetClass()->ImplementsInterface(UMABSCostReceiver::StaticClass()))
	{
		OutDebugMessage = FString::Printf(
			TEXT("Failed to spend resource cost %.2f because owner '%s' does not implement IMABSCostReceiver."),
			AbilitySpec.AbilityDefinition->ResourceCost,
			*GetNameSafe(Owner));
		return EMABSAbilityActivationResult::InsufficientResources;
	}

	const bool bSpentCost = IMABSCostReceiver::Execute_SpendMABSCost(
		Owner,
		AbilitySpec.AbilityDefinition->ResourceCost,
		AbilitySpec.AbilityDefinition);
	if (!bSpentCost)
	{
		OutDebugMessage = FString::Printf(
			TEXT("Failed to spend resource cost %.2f for ability '%s'."),
			AbilitySpec.AbilityDefinition->ResourceCost,
			*GetAbilityLabel(AbilitySpec.AbilityDefinition));
		return EMABSAbilityActivationResult::InsufficientResources;
	}

	OutDebugMessage = FString::Printf(
		TEXT("Spent resource cost %.2f for ability '%s'."),
		AbilitySpec.AbilityDefinition->ResourceCost,
		*GetAbilityLabel(AbilitySpec.AbilityDefinition));
	return EMABSAbilityActivationResult::Success;
}

void UMABSAbilityComponent::StartAbilityCooldowns(FMABSAbilitySpec& AbilitySpec, const bool bNotifyOwningClient)
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	UWorld* const World = GetWorld();
	if (AbilityDefinition == nullptr || World == nullptr)
	{
		return;
	}

	PruneExpiredCooldownGroupStates();

	const float CooldownDuration = FMath::Max(0.0f, AbilityDefinition->CooldownSeconds);
	const float CooldownEndTime = CooldownDuration > 0.0f ? World->GetTimeSeconds() + CooldownDuration : 0.0f;
	AbilitySpec.CooldownEndTime = CooldownEndTime;

	FString CooldownMessage;
	if (CooldownDuration > 0.0f)
	{
		if (AbilityDefinition->CooldownGroupTag.IsValid())
		{
			FMABSCooldownGroupState* CooldownGroupState = FindCooldownGroupStateMutable(AbilityDefinition->CooldownGroupTag);
			if (CooldownGroupState == nullptr)
			{
				FMABSCooldownGroupState NewCooldownGroupState;
				NewCooldownGroupState.CooldownGroupTag = AbilityDefinition->CooldownGroupTag;
				NewCooldownGroupState.CooldownEndTime = CooldownEndTime;
				CooldownGroupStates.Add(NewCooldownGroupState);
			}
			else
			{
				CooldownGroupState->CooldownEndTime = FMath::Max(CooldownGroupState->CooldownEndTime, CooldownEndTime);
			}

			CooldownMessage = FString::Printf(
				TEXT("Started cooldown for ability '%s' for %.2f seconds. Cooldown group '%s' is now active."),
				*GetAbilityLabel(AbilityDefinition),
				CooldownDuration,
				*AbilityDefinition->CooldownGroupTag.ToString());
		}
		else
		{
			CooldownMessage = FString::Printf(
				TEXT("Started cooldown for ability '%s' for %.2f seconds."),
				*GetAbilityLabel(AbilityDefinition),
				CooldownDuration);
		}

		const FMABSAbilityDebugEvent CooldownStartedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::CooldownStarted,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::Success,
			CooldownMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(CooldownStartedEvent);
		}
	}
}

FMABSAbilitySpec* UMABSAbilityComponent::FindGrantedAbilitySpecByTagMutable(const FGameplayTag& AbilityTag)
{
	return GrantedAbilities.FindByPredicate(
		[&AbilityTag](const FMABSAbilitySpec& AbilitySpec)
		{
			return AbilitySpec.AbilityTag == AbilityTag;
		});
}

const FMABSAbilitySpec* UMABSAbilityComponent::FindGrantedAbilitySpecByTagInternal(const FGameplayTag& AbilityTag) const
{
	return GrantedAbilities.FindByPredicate(
		[&AbilityTag](const FMABSAbilitySpec& AbilitySpec)
		{
			return AbilitySpec.AbilityTag == AbilityTag;
		});
}

FMABSAbilitySpec* UMABSAbilityComponent::FindGrantedAbilitySpecByHandle(const FMABSAbilityHandle& AbilityHandle)
{
	return GrantedAbilities.FindByPredicate(
		[&AbilityHandle](const FMABSAbilitySpec& AbilitySpec)
		{
			return AbilitySpec.Handle == AbilityHandle;
		});
}

FMABSCooldownGroupState* UMABSAbilityComponent::FindCooldownGroupStateMutable(const FGameplayTag& CooldownGroupTag)
{
	return CooldownGroupStates.FindByPredicate(
		[&CooldownGroupTag](const FMABSCooldownGroupState& CooldownGroupState)
		{
			return CooldownGroupState.CooldownGroupTag == CooldownGroupTag;
		});
}

const FMABSCooldownGroupState* UMABSAbilityComponent::FindCooldownGroupStateInternal(const FGameplayTag& CooldownGroupTag) const
{
	return CooldownGroupStates.FindByPredicate(
		[&CooldownGroupTag](const FMABSCooldownGroupState& CooldownGroupState)
		{
			return CooldownGroupState.CooldownGroupTag == CooldownGroupTag;
		});
}

void UMABSAbilityComponent::ResetAbilityToIdle(const FMABSAbilityHandle AbilityHandle)
{
	FMABSAbilitySpec* AbilitySpec = FindGrantedAbilitySpecByHandle(AbilityHandle);
	if (AbilitySpec == nullptr)
	{
		return;
	}

	if (AbilitySpec->RuntimeState == EMABSAbilityRuntimeState::Startup
		|| AbilitySpec->RuntimeState == EMABSAbilityRuntimeState::Active
		|| AbilitySpec->RuntimeState == EMABSAbilityRuntimeState::Recovery)
	{
		AbilitySpec->RuntimeState = EMABSAbilityRuntimeState::Idle;
		ClearAbilityExecutionContext(AbilityHandle, true);
	}
}

void UMABSAbilityComponent::PruneExpiredCooldownGroupStates()
{
	UWorld* const World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const float CurrentWorldTime = World->GetTimeSeconds();
	CooldownGroupStates.RemoveAll(
		[CurrentWorldTime](const FMABSCooldownGroupState& CooldownGroupState)
		{
			return !CooldownGroupState.CooldownGroupTag.IsValid() || CooldownGroupState.CooldownEndTime <= CurrentWorldTime;
		});
}

float UMABSAbilityComponent::GetCooldownRemainingForAbilitySpec(const FMABSAbilitySpec& AbilitySpec) const
{
	const UWorld* const World = GetWorld();
	if (World == nullptr)
	{
		return 0.0f;
	}

	const float CurrentWorldTime = World->GetTimeSeconds();
	const float AbilityCooldownRemaining = FMath::Max(0.0f, AbilitySpec.CooldownEndTime - CurrentWorldTime);
	if (AbilitySpec.AbilityDefinition == nullptr || !AbilitySpec.AbilityDefinition->CooldownGroupTag.IsValid())
	{
		return AbilityCooldownRemaining;
	}

	return FMath::Max(AbilityCooldownRemaining, GetCooldownGroupRemainingInternal(AbilitySpec.AbilityDefinition->CooldownGroupTag));
}

float UMABSAbilityComponent::GetCooldownGroupRemainingInternal(const FGameplayTag& CooldownGroupTag) const
{
	if (!CooldownGroupTag.IsValid())
	{
		return 0.0f;
	}

	const UWorld* const World = GetWorld();
	if (World == nullptr)
	{
		return 0.0f;
	}

	const FMABSCooldownGroupState* CooldownGroupState = FindCooldownGroupStateInternal(CooldownGroupTag);
	if (CooldownGroupState == nullptr)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, CooldownGroupState->CooldownEndTime - World->GetTimeSeconds());
}

FMABSAbilityDebugEvent UMABSAbilityComponent::MakeDebugEvent(
	FName EventName,
	const FGameplayTag& AbilityTag,
	const FMABSAbilityHandle& AbilityHandle,
	const EMABSAbilityRuntimeState RuntimeState,
	const EMABSAbilityActivationResult ActivationResult,
	const FString& Message) const
{
	FMABSAbilityDebugEvent DebugEvent;
	DebugEvent.EventName = EventName;
	DebugEvent.AbilityTag = AbilityTag;
	DebugEvent.AbilityHandle = AbilityHandle;
	DebugEvent.RuntimeState = RuntimeState;
	DebugEvent.ActivationResult = ActivationResult;
	DebugEvent.OwnerName = GetNameSafe(GetOwner());
	DebugEvent.WorldTimeSeconds = GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : 0.0f;
	DebugEvent.Message = Message;
	return DebugEvent;
}

void UMABSAbilityComponent::RecordDebugEvent(const FMABSAbilityDebugEvent& DebugEvent)
{
	RecentDebugEvents.Add(DebugEvent);
	if (RecentDebugEvents.Num() > MaxStoredDebugEvents)
	{
		const int32 NumberToRemove = RecentDebugEvents.Num() - MaxStoredDebugEvents;
		RecentDebugEvents.RemoveAt(0, NumberToRemove, EAllowShrinking::No);
	}

	const UEnum* RuntimeStateEnum = StaticEnum<EMABSAbilityRuntimeState>();
	const UEnum* ResultEnum = StaticEnum<EMABSAbilityActivationResult>();
	const FString RuntimeStateName = RuntimeStateEnum != nullptr
		? RuntimeStateEnum->GetNameStringByValue(static_cast<int64>(DebugEvent.RuntimeState))
		: TEXT("Unknown");
	const FString ResultName = ResultEnum != nullptr
		? ResultEnum->GetNameStringByValue(static_cast<int64>(DebugEvent.ActivationResult))
		: TEXT("Unknown");

	UE_LOG(
		LogMABSAbilitySystem,
		Log,
		TEXT("[%s] Owner=%s AbilityTag=%s Handle=%d State=%s Result=%s Message=%s"),
		*DebugEvent.EventName.ToString(),
		*DebugEvent.OwnerName,
		*DebugEvent.AbilityTag.ToString(),
		DebugEvent.AbilityHandle.Value,
		*RuntimeStateName,
		*ResultName,
		*DebugEvent.Message);

	OnAbilityDebugEvent.Broadcast(DebugEvent);
}

FMABSAbilityDebugEvent UMABSAbilityComponent::EmitDebugEvent(
	FName EventName,
	const FGameplayTag& AbilityTag,
	const FMABSAbilityHandle& AbilityHandle,
	const EMABSAbilityRuntimeState RuntimeState,
	const EMABSAbilityActivationResult ActivationResult,
	const FString& Message)
{
	const FMABSAbilityDebugEvent DebugEvent = MakeDebugEvent(
		EventName,
		AbilityTag,
		AbilityHandle,
		RuntimeState,
		ActivationResult,
		Message);
	RecordDebugEvent(DebugEvent);
	return DebugEvent;
}

void UMABSAbilityComponent::EmitDebugEventToOwningClient(const FMABSAbilityDebugEvent& DebugEvent)
{
	if (!bReplicateDebugDataToOwningClient)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (Owner == nullptr || Owner->GetNetOwningPlayer() == nullptr)
	{
		return;
	}

	ClientReceiveAbilityDebugEvent(DebugEvent);
}

void UMABSAbilityComponent::RecordLatestTargetTraceDebugInfo(const FMABSTargetTraceDebugInfo& DebugInfo)
{
	LatestTargetTraceDebugInfo = DebugInfo;
}

void UMABSAbilityComponent::EmitLatestTargetTraceDebugInfoToOwningClient(const FMABSTargetTraceDebugInfo& DebugInfo)
{
	if (!bReplicateDebugDataToOwningClient)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (Owner == nullptr || Owner->GetNetOwningPlayer() == nullptr)
	{
		return;
	}

	ClientReceiveTargetTraceDebugInfo(DebugInfo);
}

void UMABSAbilityComponent::ClearLatestTargetTraceDebugInfo(const bool bNotifyOwningClient)
{
	const FMABSTargetTraceDebugInfo ClearedDebugInfo;
	RecordLatestTargetTraceDebugInfo(ClearedDebugInfo);
	if (bNotifyOwningClient)
	{
		EmitLatestTargetTraceDebugInfoToOwningClient(ClearedDebugInfo);
	}
}

bool UMABSAbilityComponent::GetTargetTraceViewPoint(
	FVector& OutTraceStart,
	FRotator& OutTraceRotation,
	FString& OutViewPointDescription) const
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}

	if (const APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		if (const AController* OwnerController = OwnerPawn->GetController())
		{
			OwnerController->GetPlayerViewPoint(OutTraceStart, OutTraceRotation);
			OutViewPointDescription = TEXT("ControllerViewPoint");
			return true;
		}
	}

	Owner->GetActorEyesViewPoint(OutTraceStart, OutTraceRotation);
	OutViewPointDescription = TEXT("OwnerEyesViewPoint");
	return true;
}

float UMABSAbilityComponent::GetEffectiveDeliveryDelay(const UMABSAbilityDefinition& AbilityDefinition) const
{
	const float StartupDuration = FMath::Max(0.0f, AbilityDefinition.StartupDuration);
	const float DeliveryTime = FMath::Max(0.0f, AbilityDefinition.DeliveryTime);
	if (DeliveryTime <= KINDA_SMALL_NUMBER)
	{
		return StartupDuration;
	}

	return FMath::Max(StartupDuration, DeliveryTime);
}

bool UMABSAbilityComponent::RequestAbilityMontagePlayback(
	const FMABSAbilitySpec& AbilitySpec,
	const bool bNotifyOwningClient)
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	if (AbilityDefinition == nullptr || AbilityDefinition->ActivationMontage == nullptr)
	{
		return false;
	}

	const float PlayRate = FMath::Max(0.01f, AbilityDefinition->MontagePlayRate);
	const FMABSAbilityDebugEvent RequestedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::MontagePlayRequested,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Requested activation montage '%s' at play rate %.2f for ability '%s'."),
			*GetNameSafe(AbilityDefinition->ActivationMontage),
			PlayRate,
			*GetAbilityLabel(AbilityDefinition)));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(RequestedEvent);
	}

	const ENetMode NetMode = GetNetMode();
	if (NetMode == NM_Standalone)
	{
		if (!PlayActivationMontageLocally(AbilityDefinition->ActivationMontage, PlayRate))
		{
			const FMABSAbilityDebugEvent FailedEvent = EmitDebugEvent(
				MABSAbilityComponentEventNames::MontagePlayFailed,
				AbilitySpec.AbilityTag,
				AbilitySpec.Handle,
				AbilitySpec.RuntimeState,
				EMABSAbilityActivationResult::Success,
				FString::Printf(
					TEXT("Activation montage '%s' could not play because no valid skeletal mesh anim instance was found."),
					*GetNameSafe(AbilityDefinition->ActivationMontage)));
			if (bNotifyOwningClient)
			{
				EmitDebugEventToOwningClient(FailedEvent);
			}

			return false;
		}

		return true;
	}

	if (NetMode == NM_ListenServer)
	{
		USkeletalMeshComponent* const SkeletalMeshComponent = ResolveAbilitySkeletalMeshComponent();
		if (SkeletalMeshComponent != nullptr && SkeletalMeshComponent->GetAnimInstance() != nullptr)
		{
			MulticastPlayActivationMontage(AbilityDefinition->ActivationMontage, PlayRate);
			return true;
		}

		const FMABSAbilityDebugEvent FailedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::MontagePlayFailed,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::Success,
			FString::Printf(
				TEXT("Activation montage '%s' was requested, but no valid skeletal mesh anim instance was found on this listen-server instance."),
				*GetNameSafe(AbilityDefinition->ActivationMontage)));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(FailedEvent);
		}
	}

	MulticastPlayActivationMontage(AbilityDefinition->ActivationMontage, PlayRate);
	return true;
}

bool UMABSAbilityComponent::PlayActivationMontageLocally(UAnimMontage* Montage, const float PlayRate) const
{
	if (Montage == nullptr)
	{
		return false;
	}

	USkeletalMeshComponent* const SkeletalMeshComponent = ResolveAbilitySkeletalMeshComponent();
	if (SkeletalMeshComponent == nullptr)
	{
		return false;
	}

	PrepareSkeletalMeshForAbilitySocketQueries(*SkeletalMeshComponent);

	UAnimInstance* const AnimInstance = SkeletalMeshComponent->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}

	return AnimInstance->Montage_Play(Montage, PlayRate) > 0.0f;
}

USkeletalMeshComponent* UMABSAbilityComponent::ResolveAbilitySkeletalMeshComponent() const
{
	const AActor* const Owner = GetOwner();
	if (Owner == nullptr)
	{
		return nullptr;
	}

	if (const ACharacter* CharacterOwner = Cast<ACharacter>(Owner))
	{
		if (USkeletalMeshComponent* const CharacterMesh = CharacterOwner->GetMesh())
		{
			return CharacterMesh;
		}
	}

	return Owner->FindComponentByClass<USkeletalMeshComponent>();
}

void UMABSAbilityComponent::PrepareSkeletalMeshForAbilitySocketQueries(USkeletalMeshComponent& SkeletalMeshComponent) const
{
	// Ability socket origins must stay accurate on authority even when the mesh is not rendered locally.
	if (CanMutateAbilityState()
		&& SkeletalMeshComponent.VisibilityBasedAnimTickOption != EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones)
	{
		SkeletalMeshComponent.VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}

	if (SkeletalMeshComponent.IsRunningParallelEvaluation())
	{
		SkeletalMeshComponent.CompleteParallelAnimationEvaluation(true);
	}

	SkeletalMeshComponent.TickAnimation(0.0f, false);
	SkeletalMeshComponent.RefreshBoneTransforms();
	SkeletalMeshComponent.UpdateComponentToWorld();
}

FVector UMABSAbilityComponent::ResolveMeleeTraceDirection() const
{
	if (const AActor* const Owner = GetOwner())
	{
		FVector OwnerForward = Owner->GetActorForwardVector();
		OwnerForward.Z = 0.0f;
		if (OwnerForward.Normalize())
		{
			return OwnerForward;
		}
	}

	FVector TraceStart = FVector::ZeroVector;
	FRotator TraceRotation = FRotator::ZeroRotator;
	FString ViewPointDescription;
	if (GetTargetTraceViewPoint(TraceStart, TraceRotation, ViewPointDescription))
	{
		FVector HorizontalAimDirection = TraceRotation.Vector();
		HorizontalAimDirection.Z = 0.0f;
		if (HorizontalAimDirection.Normalize())
		{
			return HorizontalAimDirection;
		}
	}

	return FVector::ForwardVector;
}

bool UMABSAbilityComponent::TryResolveSocketTransform(
	const FName& SocketName,
	FTransform& OutSocketTransform,
	FString& OutComponentName) const
{
	if (SocketName.IsNone())
	{
		return false;
	}

	if (USkeletalMeshComponent* const SkeletalMeshComponent = ResolveAbilitySkeletalMeshComponent())
	{
		if (SkeletalMeshComponent->DoesSocketExist(SocketName))
		{
			PrepareSkeletalMeshForAbilitySocketQueries(*SkeletalMeshComponent);
			OutSocketTransform = SkeletalMeshComponent->GetSocketTransform(SocketName, RTS_World);
			OutComponentName = GetNameSafe(SkeletalMeshComponent);
			return true;
		}
	}

	const AActor* const Owner = GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}

	if (USceneComponent* const RootComponent = Owner->GetRootComponent())
	{
		if (RootComponent->DoesSocketExist(SocketName))
		{
			OutSocketTransform = RootComponent->GetSocketTransform(SocketName, RTS_World);
			OutComponentName = GetNameSafe(RootComponent);
			return true;
		}
	}

	return false;
}

bool UMABSAbilityComponent::ResolveDeliveryOriginTransform(
	const UMABSAbilityDefinition& AbilityDefinition,
	const EMABSDeliveryMode DeliveryMode,
	FTransform& OutOriginTransform,
	FString& OutOriginDescription,
	const bool bNotifyOwningClient)
{
	AActor* const Owner = GetOwner();
	if (Owner == nullptr)
	{
		OutOriginDescription = TEXT("No owning actor was available.");
		return false;
	}

	FName SpecificSocketName = NAME_None;
	FVector LocalOffset = FVector::ZeroVector;
	switch (DeliveryMode)
	{
	case EMABSDeliveryMode::HitTrace:
		SpecificSocketName = AbilityDefinition.HitTraceOriginSocketName;
		LocalOffset = AbilityDefinition.HitTraceOriginOffset;
		break;

	case EMABSDeliveryMode::Melee:
		SpecificSocketName = AbilityDefinition.MeleeOriginSocketName;
		LocalOffset = AbilityDefinition.MeleeOriginOffset;
		break;

	case EMABSDeliveryMode::Projectile:
		SpecificSocketName = AbilityDefinition.ProjectileSpawnSocketName;
		LocalOffset = AbilityDefinition.ProjectileSpawnOffset;
		break;

	default:
		break;
	}

	const FName ResolvedSocketName = !SpecificSocketName.IsNone()
		? SpecificSocketName
		: AbilityDefinition.DeliveryOriginSocketName;
	const FMABSAbilitySpec* const AbilitySpec = FindGrantedAbilitySpecByTagInternal(AbilityDefinition.AbilityTag);
	const FMABSAbilityHandle AbilityHandle = AbilitySpec != nullptr ? AbilitySpec->Handle : FMABSAbilityHandle();
	const EMABSAbilityRuntimeState RuntimeState = AbilitySpec != nullptr
		? AbilitySpec->RuntimeState
		: EMABSAbilityRuntimeState::None;

	FTransform ResolvedTransform = FTransform::Identity;
	FString ComponentName;
	if (!ResolvedSocketName.IsNone() && TryResolveSocketTransform(ResolvedSocketName, ResolvedTransform, ComponentName))
	{
		ResolvedTransform.AddToTranslation(ResolvedTransform.GetRotation().RotateVector(LocalOffset));
		OutOriginTransform = ResolvedTransform;
		OutOriginDescription = FString::Printf(
			TEXT("Socket '%s' on '%s'"),
			*ResolvedSocketName.ToString(),
			*ComponentName);

		const FMABSAbilityDebugEvent SocketResolvedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::SocketResolved,
			AbilityDefinition.AbilityTag,
			AbilityHandle,
			RuntimeState,
			EMABSAbilityActivationResult::Success,
			FString::Printf(
				TEXT("Resolved %s origin from socket '%s' on component '%s'."),
				*GetDeliveryModeLabel(DeliveryMode),
				*ResolvedSocketName.ToString(),
				*ComponentName));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(SocketResolvedEvent);
		}

		return true;
	}

	FString FallbackDescription;
	bool bResolvedFallback = false;
	switch (DeliveryMode)
	{
	case EMABSDeliveryMode::HitTrace:
	case EMABSDeliveryMode::Projectile:
		{
			FVector ViewLocation = FVector::ZeroVector;
			FRotator ViewRotation = FRotator::ZeroRotator;
			if (GetTargetTraceViewPoint(ViewLocation, ViewRotation, FallbackDescription))
			{
				ResolvedTransform = FTransform(ViewRotation, ViewLocation);
				bResolvedFallback = true;
				break;
			}
		}
		break;

	case EMABSDeliveryMode::Melee:
	default:
		break;
	}

	if (!bResolvedFallback)
	{
		ResolvedTransform = Owner->GetActorTransform();
		FallbackDescription = TEXT("OwnerActorTransform");
	}

	ResolvedTransform.AddToTranslation(ResolvedTransform.GetRotation().RotateVector(LocalOffset));
	OutOriginTransform = ResolvedTransform;
	OutOriginDescription = FallbackDescription;

	const FMABSAbilityDebugEvent SocketFallbackEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::SocketFallbackUsed,
		AbilityDefinition.AbilityTag,
		AbilityHandle,
		RuntimeState,
		EMABSAbilityActivationResult::Success,
		ResolvedSocketName.IsNone()
			? FString::Printf(
				TEXT("Using fallback origin '%s' for %s delivery because no socket was authored."),
				*FallbackDescription,
				*GetDeliveryModeLabel(DeliveryMode))
			: FString::Printf(
				TEXT("Using fallback origin '%s' for %s delivery because socket '%s' was unavailable."),
				*FallbackDescription,
				*GetDeliveryModeLabel(DeliveryMode),
				*ResolvedSocketName.ToString()));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(SocketFallbackEvent);
	}

	return true;
}

bool UMABSAbilityComponent::GetProjectileSpawnTransform(
	const UMABSAbilityDefinition& AbilityDefinition,
	FTransform& OutSpawnTransform,
	FString& OutDebugMessage,
	const bool bNotifyOwningClient)
{
	FTransform OriginTransform = FTransform::Identity;
	FString OriginDescription;
	if (!ResolveDeliveryOriginTransform(
		AbilityDefinition,
		EMABSDeliveryMode::Projectile,
		OriginTransform,
		OriginDescription,
		bNotifyOwningClient))
	{
		OutDebugMessage = TEXT("Projectile delivery failed because no valid projectile origin could be resolved.");
		return false;
	}

	FVector AimLocation = FVector::ZeroVector;
	FRotator SpawnRotation = OriginTransform.GetRotation().Rotator();
	FString AimDescription = OriginDescription;
	if (GetTargetTraceViewPoint(AimLocation, SpawnRotation, AimDescription))
	{
		AimDescription = FString::Printf(TEXT("%s + %sAim"), *OriginDescription, *AimDescription);
	}
	else
	{
		AimDescription = FString::Printf(TEXT("%s + OriginRotation"), *OriginDescription);
	}

	const FVector SpawnLocation = OriginTransform.GetLocation();
	OutSpawnTransform = FTransform(SpawnRotation, SpawnLocation);
	OutDebugMessage = FString::Printf(
		TEXT("Prepared projectile spawn at %s using origin '%s' and aim '%s'."),
		*FormatVectorForDebug(SpawnLocation),
		*OriginDescription,
		*AimDescription);
	return true;
}

bool UMABSAbilityComponent::ResolvePresentationTransform(
	const UMABSAbilityDefinition& AbilityDefinition,
	const EMABSDeliveryMode DeliveryMode,
	const FMABSPresentationCueData& CueData,
	FTransform& OutPresentationTransform,
	FString& OutOriginDescription,
	const FGameplayTag& AbilityTag,
	const FMABSAbilityHandle& AbilityHandle,
	const EMABSAbilityRuntimeState RuntimeState,
	const bool bNotifyOwningClient)
{
	AActor* const Owner = GetOwner();
	if (Owner == nullptr)
	{
		OutOriginDescription = TEXT("No owning actor was available for presentation.");
		return false;
	}

	FName DeliverySocketName = NAME_None;
	switch (DeliveryMode)
	{
	case EMABSDeliveryMode::HitTrace:
		DeliverySocketName = AbilityDefinition.HitTraceOriginSocketName;
		break;

	case EMABSDeliveryMode::Melee:
		DeliverySocketName = AbilityDefinition.MeleeOriginSocketName;
		break;

	case EMABSDeliveryMode::Projectile:
		DeliverySocketName = AbilityDefinition.ProjectileSpawnSocketName;
		break;

	default:
		break;
	}

	const FName PreferredSocketName = !CueData.SocketName.IsNone()
		? CueData.SocketName
		: (!DeliverySocketName.IsNone() ? DeliverySocketName : AbilityDefinition.DeliveryOriginSocketName);

	FTransform PresentationTransform = FTransform::Identity;
	FString ComponentName;
	if (!PreferredSocketName.IsNone() && TryResolveSocketTransform(PreferredSocketName, PresentationTransform, ComponentName))
	{
		PresentationTransform.ConcatenateRotation(CueData.RotationOffset.Quaternion());
		PresentationTransform.AddToTranslation(PresentationTransform.GetRotation().RotateVector(CueData.LocationOffset));
		OutPresentationTransform = PresentationTransform;
		OutOriginDescription = FString::Printf(
			TEXT("Socket '%s' on '%s'"),
			*PreferredSocketName.ToString(),
			*ComponentName);
		return true;
	}

	FString FallbackDescription;
	bool bResolvedFallback = false;
	switch (DeliveryMode)
	{
	case EMABSDeliveryMode::HitTrace:
	case EMABSDeliveryMode::Projectile:
		{
			FVector ViewLocation = FVector::ZeroVector;
			FRotator ViewRotation = FRotator::ZeroRotator;
			if (GetTargetTraceViewPoint(ViewLocation, ViewRotation, FallbackDescription))
			{
				PresentationTransform = FTransform(ViewRotation, ViewLocation);
				bResolvedFallback = true;
			}
		}
		break;

	case EMABSDeliveryMode::Direct:
	case EMABSDeliveryMode::Melee:
	default:
		break;
	}

	if (!bResolvedFallback)
	{
		PresentationTransform = Owner->GetActorTransform();
		FallbackDescription = TEXT("OwnerActorTransform");
	}

	PresentationTransform.ConcatenateRotation(CueData.RotationOffset.Quaternion());
	PresentationTransform.AddToTranslation(PresentationTransform.GetRotation().RotateVector(CueData.LocationOffset));
	OutPresentationTransform = PresentationTransform;
	OutOriginDescription = FallbackDescription;

	const FMABSAbilityDebugEvent FallbackEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::PresentationSocketFallbackUsed,
		AbilityTag,
		AbilityHandle,
		RuntimeState,
		EMABSAbilityActivationResult::Success,
		PreferredSocketName.IsNone()
			? FString::Printf(
				TEXT("Using fallback presentation origin '%s' for %s because no presentation socket override was authored."),
				*FallbackDescription,
				*GetDeliveryModeLabel(DeliveryMode))
			: FString::Printf(
				TEXT("Using fallback presentation origin '%s' for %s because socket '%s' was unavailable."),
				*FallbackDescription,
				*GetDeliveryModeLabel(DeliveryMode),
				*PreferredSocketName.ToString()));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(FallbackEvent);
	}

	return true;
}

bool UMABSAbilityComponent::ResolveCueVisibilityPolicy(
	const EMABSPresentationCueVisibilityPolicy RequestedPolicy,
	const EMABSPresentationCuePhase CuePhase,
	const FGameplayTag& AbilityTag,
	const FMABSAbilityHandle& AbilityHandle,
	const EMABSAbilityRuntimeState RuntimeState,
	const bool bNotifyOwningClient,
	EMABSPresentationCueVisibilityPolicy& OutResolvedPolicy,
	FString& OutResolutionMessage)
{
	OutResolvedPolicy = RequestedPolicy;

	switch (RequestedPolicy)
	{
	case EMABSPresentationCueVisibilityPolicy::OwnerOnly:
		if (CanRouteCueToOwningClient())
		{
			OutResolutionMessage = TEXT("Routing to the owning client only.");
			return true;
		}

		OutResolvedPolicy = EMABSPresentationCueVisibilityPolicy::RelevantClients;
		OutResolutionMessage = FString::Printf(
			TEXT("%s cue requested OwnerOnly but no owning client was available, so it fell back to RelevantClients."),
			*GetPresentationCuePhaseLabel(CuePhase));
		{
			const FMABSAbilityDebugEvent FallbackEvent = EmitDebugEvent(
				MABSAbilityComponentEventNames::PresentationCuePolicyFallbackUsed,
				AbilityTag,
				AbilityHandle,
				RuntimeState,
				EMABSAbilityActivationResult::Success,
				OutResolutionMessage);
			if (bNotifyOwningClient)
			{
				EmitDebugEventToOwningClient(FallbackEvent);
			}
		}
		return true;

	case EMABSPresentationCueVisibilityPolicy::LocalOnly:
		if (GetNetMode() == NM_DedicatedServer)
		{
			OutResolutionMessage = FString::Printf(
				TEXT("%s cue was skipped because LocalOnly cues are not realized on dedicated servers."),
				*GetPresentationCuePhaseLabel(CuePhase));
			return false;
		}

		if (GetNetMode() == NM_Standalone || IsOwningActorLocallyControlled())
		{
			OutResolutionMessage = TEXT("Routing locally only.");
			return true;
		}

		OutResolutionMessage = FString::Printf(
			TEXT("%s cue was skipped because the owning actor is not locally controlled on this instance."),
			*GetPresentationCuePhaseLabel(CuePhase));
		return false;

	case EMABSPresentationCueVisibilityPolicy::RelevantClients:
	default:
		OutResolutionMessage = TEXT("Routing to relevant clients.");
		return true;
	}
}

bool UMABSAbilityComponent::RoutePresentationCue(
	const FMABSPresentationCueEvent& CueEvent,
	const FGameplayTag& AbilityTag,
	const FMABSAbilityHandle& AbilityHandle,
	const EMABSAbilityRuntimeState RuntimeState,
	const bool bNotifyOwningClient)
{
	if (!CueEvent.HasAnyPresentation())
	{
		const FMABSAbilityDebugEvent SkippedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::PresentationCueSkipped,
			AbilityTag,
			AbilityHandle,
			RuntimeState,
			EMABSAbilityActivationResult::Success,
			FString::Printf(
				TEXT("%s cue was skipped because it did not contain any presentation assets."),
				*GetPresentationCuePhaseLabel(CueEvent.Phase)));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(SkippedEvent);
		}
		return false;
	}

	EMABSPresentationCueVisibilityPolicy ResolvedPolicy = CueEvent.VisibilityPolicy;
	FString ResolutionMessage;
	if (!ResolveCueVisibilityPolicy(
		CueEvent.VisibilityPolicy,
		CueEvent.Phase,
		AbilityTag,
		AbilityHandle,
		RuntimeState,
		bNotifyOwningClient,
		ResolvedPolicy,
		ResolutionMessage))
	{
		const FMABSAbilityDebugEvent SkippedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::PresentationCueSkipped,
			AbilityTag,
			AbilityHandle,
			RuntimeState,
			EMABSAbilityActivationResult::Success,
			ResolutionMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(SkippedEvent);
		}
		return false;
	}

	FMABSPresentationCueEvent RoutedCueEvent = CueEvent;
	RoutedCueEvent.VisibilityPolicy = ResolvedPolicy;

	switch (ResolvedPolicy)
	{
	case EMABSPresentationCueVisibilityPolicy::OwnerOnly:
		if (GetNetMode() == NM_Standalone)
		{
			PlayPresentationCueLocally(RoutedCueEvent);
		}
		else
		{
			ClientPlayPresentationCue(RoutedCueEvent);
		}
		break;

	case EMABSPresentationCueVisibilityPolicy::LocalOnly:
		PlayPresentationCueLocally(RoutedCueEvent);
		break;

	case EMABSPresentationCueVisibilityPolicy::RelevantClients:
	default:
		MulticastPlayPresentationCue(RoutedCueEvent);
		break;
	}

	const FMABSAbilityDebugEvent RoutedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::PresentationCueRouted,
		AbilityTag,
		AbilityHandle,
		RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Routed %s cue with policy %s at %s. %s Assets: %s."),
			*GetPresentationCuePhaseLabel(RoutedCueEvent.Phase),
			*GetPresentationVisibilityPolicyLabel(RoutedCueEvent.VisibilityPolicy),
			*FormatVectorForDebug(RoutedCueEvent.Location),
			*ResolutionMessage,
			*DescribeCueEventAssets(RoutedCueEvent)));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(RoutedEvent);
	}

	return true;
}

bool UMABSAbilityComponent::RouteTracerCue(
	const FMABSTracerCueEvent& TracerEvent,
	const FGameplayTag& AbilityTag,
	const FMABSAbilityHandle& AbilityHandle,
	const EMABSAbilityRuntimeState RuntimeState,
	const bool bNotifyOwningClient)
{
	if (!TracerEvent.HasAnyPresentation())
	{
		const FMABSAbilityDebugEvent SkippedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::TracerCueSkipped,
			AbilityTag,
			AbilityHandle,
			RuntimeState,
			EMABSAbilityActivationResult::Success,
			TEXT("Tracer cue was skipped because it did not contain any presentation assets."));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(SkippedEvent);
		}
		return false;
	}

	EMABSPresentationCueVisibilityPolicy ResolvedPolicy = TracerEvent.VisibilityPolicy;
	FString ResolutionMessage;
	if (!ResolveCueVisibilityPolicy(
		TracerEvent.VisibilityPolicy,
		EMABSPresentationCuePhase::Tracer,
		AbilityTag,
		AbilityHandle,
		RuntimeState,
		bNotifyOwningClient,
		ResolvedPolicy,
		ResolutionMessage))
	{
		const FMABSAbilityDebugEvent SkippedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::TracerCueSkipped,
			AbilityTag,
			AbilityHandle,
			RuntimeState,
			EMABSAbilityActivationResult::Success,
			ResolutionMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(SkippedEvent);
		}
		return false;
	}

	FMABSTracerCueEvent RoutedTracerEvent = TracerEvent;
	RoutedTracerEvent.VisibilityPolicy = ResolvedPolicy;

	switch (ResolvedPolicy)
	{
	case EMABSPresentationCueVisibilityPolicy::OwnerOnly:
		if (GetNetMode() == NM_Standalone)
		{
			SpawnTracerPresentationLocally(RoutedTracerEvent);
		}
		else
		{
			ClientSpawnTracerPresentation(RoutedTracerEvent);
		}
		break;

	case EMABSPresentationCueVisibilityPolicy::LocalOnly:
		SpawnTracerPresentationLocally(RoutedTracerEvent);
		break;

	case EMABSPresentationCueVisibilityPolicy::RelevantClients:
	default:
		MulticastSpawnTracerPresentation(RoutedTracerEvent);
		break;
	}

	const FMABSAbilityDebugEvent RoutedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::TracerCueRouted,
		AbilityTag,
		AbilityHandle,
		RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Routed tracer cue with policy %s from %s to %s. %s Assets: %s."),
			*GetPresentationVisibilityPolicyLabel(RoutedTracerEvent.VisibilityPolicy),
			*FormatVectorForDebug(RoutedTracerEvent.TraceStart),
			*FormatVectorForDebug(RoutedTracerEvent.TraceEnd),
			*ResolutionMessage,
			*DescribeTracerCueAssets(RoutedTracerEvent)));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(RoutedEvent);
	}

	return true;
}

void UMABSAbilityComponent::TriggerStartupPresentation(const FMABSAbilitySpec& AbilitySpec, const bool bNotifyOwningClient)
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	if (AbilityDefinition == nullptr || !AbilityDefinition->StartupPresentation.HasAnyPresentation())
	{
		return;
	}

	FTransform PresentationTransform = FTransform::Identity;
	FString OriginDescription;
	if (!ResolvePresentationTransform(
		*AbilityDefinition,
		AbilityDefinition->DeliveryMode,
		AbilityDefinition->StartupPresentation.Cue,
		PresentationTransform,
		OriginDescription,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		bNotifyOwningClient))
	{
		return;
	}

	FMABSPresentationCueEvent CueEvent;
	CueEvent.AbilityTag = AbilitySpec.AbilityTag;
	CueEvent.AbilityHandle = AbilitySpec.Handle;
	CueEvent.RuntimeState = AbilitySpec.RuntimeState;
	CueEvent.Phase = EMABSPresentationCuePhase::Startup;
	CueEvent.VisibilityPolicy = AbilityDefinition->StartupPresentation.Cue.VisibilityPolicy;
	CueEvent.VFX = AbilityDefinition->StartupPresentation.Cue.VFX;
	CueEvent.SFX = AbilityDefinition->StartupPresentation.Cue.SFX;
	CueEvent.CameraShakeClass = AbilityDefinition->StartupPresentation.Cue.CameraShake.CameraShakeClass;
	CueEvent.Location = PresentationTransform.GetLocation();
	CueEvent.Rotation = PresentationTransform.GetRotation().Rotator();
	CueEvent.CameraShakeInnerRadius = AbilityDefinition->StartupPresentation.Cue.CameraShake.InnerRadius;
	CueEvent.CameraShakeOuterRadius = AbilityDefinition->StartupPresentation.Cue.CameraShake.OuterRadius;
	CueEvent.CameraShakeFalloff = AbilityDefinition->StartupPresentation.Cue.CameraShake.Falloff;
	if (!RoutePresentationCue(CueEvent, AbilitySpec.AbilityTag, AbilitySpec.Handle, AbilitySpec.RuntimeState, bNotifyOwningClient))
	{
		return;
	}

	const FMABSAbilityDebugEvent PresentationEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::StartupPresentationTriggered,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Triggered startup presentation cue for ability '%s' with policy %s at %s using %s. Assets: %s."),
			*GetAbilityLabel(AbilityDefinition),
			*GetPresentationVisibilityPolicyLabel(CueEvent.VisibilityPolicy),
			*FormatVectorForDebug(CueEvent.Location),
			*OriginDescription,
			*DescribeCueAssets(AbilityDefinition->StartupPresentation.Cue)));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(PresentationEvent);
	}
}

void UMABSAbilityComponent::TriggerDeliveryPresentation(
	const FMABSAbilitySpec& AbilitySpec,
	const EMABSDeliveryMode DeliveryMode,
	const bool bNotifyOwningClient)
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	if (AbilityDefinition == nullptr || !AbilityDefinition->DeliveryPresentation.Cue.HasAnyPresentation())
	{
		return;
	}

	FTransform PresentationTransform = FTransform::Identity;
	FString OriginDescription;
	if (!ResolvePresentationTransform(
		*AbilityDefinition,
		DeliveryMode,
		AbilityDefinition->DeliveryPresentation.Cue,
		PresentationTransform,
		OriginDescription,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		bNotifyOwningClient))
	{
		return;
	}

	FMABSPresentationCueEvent CueEvent;
	CueEvent.AbilityTag = AbilitySpec.AbilityTag;
	CueEvent.AbilityHandle = AbilitySpec.Handle;
	CueEvent.RuntimeState = AbilitySpec.RuntimeState;
	CueEvent.Phase = EMABSPresentationCuePhase::Delivery;
	CueEvent.VisibilityPolicy = AbilityDefinition->DeliveryPresentation.Cue.VisibilityPolicy;
	CueEvent.VFX = AbilityDefinition->DeliveryPresentation.Cue.VFX;
	CueEvent.SFX = AbilityDefinition->DeliveryPresentation.Cue.SFX;
	CueEvent.CameraShakeClass = AbilityDefinition->DeliveryPresentation.Cue.CameraShake.CameraShakeClass;
	CueEvent.Location = PresentationTransform.GetLocation();
	CueEvent.Rotation = PresentationTransform.GetRotation().Rotator();
	CueEvent.CameraShakeInnerRadius = AbilityDefinition->DeliveryPresentation.Cue.CameraShake.InnerRadius;
	CueEvent.CameraShakeOuterRadius = AbilityDefinition->DeliveryPresentation.Cue.CameraShake.OuterRadius;
	CueEvent.CameraShakeFalloff = AbilityDefinition->DeliveryPresentation.Cue.CameraShake.Falloff;
	if (!RoutePresentationCue(CueEvent, AbilitySpec.AbilityTag, AbilitySpec.Handle, AbilitySpec.RuntimeState, bNotifyOwningClient))
	{
		return;
	}

	const FMABSAbilityDebugEvent PresentationEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::DeliveryPresentationTriggered,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Triggered delivery presentation cue for %s with policy %s at %s using %s. Assets: %s."),
			*GetDeliveryModeLabel(DeliveryMode),
			*GetPresentationVisibilityPolicyLabel(CueEvent.VisibilityPolicy),
			*FormatVectorForDebug(CueEvent.Location),
			*OriginDescription,
			*DescribeCueAssets(AbilityDefinition->DeliveryPresentation.Cue)));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(PresentationEvent);
	}
}

void UMABSAbilityComponent::TriggerTracerPresentation(
	const FMABSAbilitySpec& AbilitySpec,
	const FVector& TraceStart,
	const FVector& TraceEnd,
	const bool bNotifyOwningClient)
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	if (AbilityDefinition == nullptr || !AbilityDefinition->DeliveryPresentation.HitTraceTracer.HasAnyPresentation())
	{
		return;
	}

	if (TraceStart.Equals(TraceEnd, KINDA_SMALL_NUMBER))
	{
		const FMABSAbilityDebugEvent FailedEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::TracerSpawnFailed,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::DeliveryFailed,
			TEXT("Tracer presentation could not run because the trace start and end points were the same."));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(FailedEvent);
		}
		return;
	}

	FMABSTracerCueEvent TracerEvent;
	TracerEvent.AbilityTag = AbilitySpec.AbilityTag;
	TracerEvent.AbilityHandle = AbilitySpec.Handle;
	TracerEvent.RuntimeState = AbilitySpec.RuntimeState;
	TracerEvent.VisibilityPolicy = AbilityDefinition->DeliveryPresentation.HitTraceTracer.VisibilityPolicy;
	TracerEvent.VFX = AbilityDefinition->DeliveryPresentation.HitTraceTracer.TracerVFX;
	TracerEvent.SFX = AbilityDefinition->DeliveryPresentation.HitTraceTracer.TracerSFX;
	TracerEvent.TraceStart = TraceStart;
	TracerEvent.TraceEnd = TraceEnd;
	if (!RouteTracerCue(TracerEvent, AbilitySpec.AbilityTag, AbilitySpec.Handle, AbilitySpec.RuntimeState, bNotifyOwningClient))
	{
		return;
	}

	const FMABSAbilityDebugEvent TracerSpawnedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::TracerSpawned,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Spawned tracer cue with policy %s from %s to %s. Assets: %s."),
			*GetPresentationVisibilityPolicyLabel(TracerEvent.VisibilityPolicy),
			*FormatVectorForDebug(TraceStart),
			*FormatVectorForDebug(TraceEnd),
			*DescribeTracerAssets(AbilityDefinition->DeliveryPresentation.HitTraceTracer)));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(TracerSpawnedEvent);
	}
}

void UMABSAbilityComponent::TriggerImpactPresentation(
	const FMABSAbilitySpec& AbilitySpec,
	const FMABSResolvedAbilityTarget& ResolvedTarget,
	const EMABSDeliveryMode DeliveryMode,
	const bool bNotifyOwningClient)
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	if (AbilityDefinition == nullptr || !AbilityDefinition->ImpactPresentation.HasAnyPresentation())
	{
		return;
	}

	FVector ImpactLocation = FVector::ZeroVector;
	FRotator ImpactRotation = FRotator::ZeroRotator;
	if (ResolvedTarget.bHasImpactHitResult)
	{
		ImpactLocation = ResolvedTarget.ImpactHitResult.ImpactPoint;
		ImpactRotation = ResolvedTarget.ImpactHitResult.ImpactNormal.Rotation();
	}
	else if (ResolvedTarget.TargetActor != nullptr)
	{
		ImpactLocation = ResolvedTarget.TargetActor->GetActorLocation();
		ImpactRotation = ResolvedTarget.TargetActor->GetActorRotation();
	}
	else if (const AActor* const Owner = GetOwner())
	{
		ImpactLocation = Owner->GetActorLocation();
		ImpactRotation = Owner->GetActorRotation();
	}

	FTransform PresentationTransform(ImpactRotation, ImpactLocation);
	PresentationTransform.ConcatenateRotation(AbilityDefinition->ImpactPresentation.Cue.RotationOffset.Quaternion());
	PresentationTransform.AddToTranslation(
		PresentationTransform.GetRotation().RotateVector(AbilityDefinition->ImpactPresentation.Cue.LocationOffset));

	FMABSPresentationCueEvent CueEvent;
	CueEvent.AbilityTag = AbilitySpec.AbilityTag;
	CueEvent.AbilityHandle = AbilitySpec.Handle;
	CueEvent.RuntimeState = AbilitySpec.RuntimeState;
	CueEvent.Phase = EMABSPresentationCuePhase::Impact;
	CueEvent.VisibilityPolicy = AbilityDefinition->ImpactPresentation.Cue.VisibilityPolicy;
	CueEvent.VFX = AbilityDefinition->ImpactPresentation.Cue.VFX;
	CueEvent.SFX = AbilityDefinition->ImpactPresentation.Cue.SFX;
	CueEvent.CameraShakeClass = AbilityDefinition->ImpactPresentation.Cue.CameraShake.CameraShakeClass;
	CueEvent.Location = PresentationTransform.GetLocation();
	CueEvent.Rotation = PresentationTransform.GetRotation().Rotator();
	CueEvent.CameraShakeInnerRadius = AbilityDefinition->ImpactPresentation.Cue.CameraShake.InnerRadius;
	CueEvent.CameraShakeOuterRadius = AbilityDefinition->ImpactPresentation.Cue.CameraShake.OuterRadius;
	CueEvent.CameraShakeFalloff = AbilityDefinition->ImpactPresentation.Cue.CameraShake.Falloff;
	if (!RoutePresentationCue(CueEvent, AbilitySpec.AbilityTag, AbilitySpec.Handle, AbilitySpec.RuntimeState, bNotifyOwningClient))
	{
		return;
	}

	const FMABSAbilityDebugEvent PresentationEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::ImpactPresentationTriggered,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Triggered impact presentation cue for %s on '%s' with policy %s at %s. Assets: %s."),
			*GetDeliveryModeLabel(DeliveryMode),
			*GetNameSafe(ResolvedTarget.TargetActor),
			*GetPresentationVisibilityPolicyLabel(CueEvent.VisibilityPolicy),
			*FormatVectorForDebug(CueEvent.Location),
			*DescribeCueAssets(AbilityDefinition->ImpactPresentation.Cue)));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(PresentationEvent);
	}
}

void UMABSAbilityComponent::TriggerProjectileImpactPresentation(
	const UMABSAbilityDefinition& AbilityDefinition,
	const FGameplayTag& AbilityTag,
	const FMABSAbilityHandle& AbilityHandle,
	const FHitResult& HitResult,
	AActor* HitActor)
{
	if (!AbilityDefinition.ImpactPresentation.HasAnyPresentation())
	{
		return;
	}

	FTransform PresentationTransform(HitResult.ImpactNormal.Rotation(), HitResult.ImpactPoint);
	PresentationTransform.ConcatenateRotation(AbilityDefinition.ImpactPresentation.Cue.RotationOffset.Quaternion());
	PresentationTransform.AddToTranslation(
		PresentationTransform.GetRotation().RotateVector(AbilityDefinition.ImpactPresentation.Cue.LocationOffset));

	FMABSPresentationCueEvent CueEvent;
	CueEvent.AbilityTag = AbilityTag;
	CueEvent.AbilityHandle = AbilityHandle;
	CueEvent.RuntimeState = EMABSAbilityRuntimeState::Idle;
	CueEvent.Phase = EMABSPresentationCuePhase::Impact;
	CueEvent.VisibilityPolicy = AbilityDefinition.ImpactPresentation.Cue.VisibilityPolicy;
	CueEvent.VFX = AbilityDefinition.ImpactPresentation.Cue.VFX;
	CueEvent.SFX = AbilityDefinition.ImpactPresentation.Cue.SFX;
	CueEvent.CameraShakeClass = AbilityDefinition.ImpactPresentation.Cue.CameraShake.CameraShakeClass;
	CueEvent.Location = PresentationTransform.GetLocation();
	CueEvent.Rotation = PresentationTransform.GetRotation().Rotator();
	CueEvent.CameraShakeInnerRadius = AbilityDefinition.ImpactPresentation.Cue.CameraShake.InnerRadius;
	CueEvent.CameraShakeOuterRadius = AbilityDefinition.ImpactPresentation.Cue.CameraShake.OuterRadius;
	CueEvent.CameraShakeFalloff = AbilityDefinition.ImpactPresentation.Cue.CameraShake.Falloff;
	if (!RoutePresentationCue(CueEvent, AbilityTag, AbilityHandle, EMABSAbilityRuntimeState::Idle, true))
	{
		return;
	}

	const FMABSAbilityDebugEvent PresentationEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::ImpactPresentationTriggered,
		AbilityTag,
		AbilityHandle,
		EMABSAbilityRuntimeState::Idle,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Triggered projectile impact presentation cue on '%s' with policy %s at %s. Assets: %s."),
			*GetNameSafe(HitActor),
			*GetPresentationVisibilityPolicyLabel(CueEvent.VisibilityPolicy),
			*FormatVectorForDebug(CueEvent.Location),
			*DescribeCueAssets(AbilityDefinition.ImpactPresentation.Cue)));
	EmitDebugEventToOwningClient(PresentationEvent);
}

bool UMABSAbilityComponent::CanRouteCueToOwningClient() const
{
	if (GetNetMode() == NM_Standalone)
	{
		return true;
	}

	AActor* const Owner = GetOwner();
	return Owner != nullptr && Owner->GetNetOwningPlayer() != nullptr;
}

bool UMABSAbilityComponent::IsOwningActorLocallyControlled() const
{
	if (GetNetMode() == NM_Standalone)
	{
		return true;
	}

	const AActor* const Owner = GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}

	if (const APawn* const OwnerPawn = Cast<APawn>(Owner))
	{
		return OwnerPawn->IsLocallyControlled();
	}

	if (const AController* const OwnerController = Cast<AController>(Owner))
	{
		return OwnerController->IsLocalController();
	}

	if (const APawn* const InstigatorPawn = Owner->GetInstigator())
	{
		return InstigatorPawn->IsLocallyControlled();
	}

	const AActor* const OuterOwner = Owner->GetOwner();
	if (const APawn* const OuterOwnerPawn = Cast<APawn>(OuterOwner))
	{
		return OuterOwnerPawn->IsLocallyControlled();
	}

	if (const AController* const OuterOwnerController = Cast<AController>(OuterOwner))
	{
		return OuterOwnerController->IsLocalController();
	}

	return false;
}

UNiagaraComponent* UMABSAbilityComponent::AcquireReusableNiagaraComponent(
	UNiagaraSystem* NiagaraSystem,
	const FVector& Location,
	const FRotator& Rotation,
	TMap<TObjectPtr<UNiagaraSystem>, TArray<TWeakObjectPtr<UNiagaraComponent>>>& ComponentPool)
{
	if (NiagaraSystem == nullptr)
	{
		return nullptr;
	}

	UWorld* const World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	TArray<TWeakObjectPtr<UNiagaraComponent>>& PooledComponents = ComponentPool.FindOrAdd(NiagaraSystem);
	UNiagaraComponent* ReusableComponent = nullptr;
	for (int32 ComponentIndex = PooledComponents.Num() - 1; ComponentIndex >= 0; --ComponentIndex)
	{
		if (!PooledComponents[ComponentIndex].IsValid())
		{
			PooledComponents.RemoveAtSwap(ComponentIndex);
			continue;
		}

		UNiagaraComponent* const CandidateComponent = PooledComponents[ComponentIndex].Get();
		if (CandidateComponent != nullptr && !CandidateComponent->IsActive())
		{
			ReusableComponent = CandidateComponent;
			break;
		}
	}

	if (ReusableComponent == nullptr)
	{
		ReusableComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			NiagaraSystem,
			Location,
			Rotation,
			FVector(1.0f),
			false,
			false);
		if (ReusableComponent == nullptr)
		{
			return nullptr;
		}

		ReusableComponent->SetAutoDestroy(false);
		PooledComponents.Add(ReusableComponent);
	}

	ReusableComponent->DeactivateImmediate();
	ReusableComponent->SetAsset(NiagaraSystem);
	ReusableComponent->SetWorldLocationAndRotation(Location, Rotation);
	ReusableComponent->SetVisibility(true, true);
	return ReusableComponent;
}

void UMABSAbilityComponent::PlayPresentationCueLocally(const FMABSPresentationCueEvent& CueEvent)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (CueEvent.VFX != nullptr)
	{
		if (UNiagaraComponent* const CueComponent = AcquireReusableNiagaraComponent(
			CueEvent.VFX,
			CueEvent.Location,
			CueEvent.Rotation,
			ReusableCueVFXPool))
		{
			CueComponent->Activate(true);
		}
	}

	if (CueEvent.SFX != nullptr)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, CueEvent.SFX, CueEvent.Location, CueEvent.Rotation);
	}

	if (CueEvent.CameraShakeClass != nullptr)
	{
		UGameplayStatics::PlayWorldCameraShake(
			this,
			CueEvent.CameraShakeClass,
			CueEvent.Location,
			CueEvent.CameraShakeInnerRadius,
			FMath::Max(CueEvent.CameraShakeInnerRadius, CueEvent.CameraShakeOuterRadius),
			CueEvent.CameraShakeFalloff);
	}

	EmitDebugEvent(
		MABSAbilityComponentEventNames::PresentationCueRealized,
		CueEvent.AbilityTag,
		CueEvent.AbilityHandle,
		CueEvent.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Realized %s cue locally with policy %s at %s. Assets: %s."),
			*GetPresentationCuePhaseLabel(CueEvent.Phase),
			*GetPresentationVisibilityPolicyLabel(CueEvent.VisibilityPolicy),
			*FormatVectorForDebug(CueEvent.Location),
			*DescribeCueEventAssets(CueEvent)));
}

void UMABSAbilityComponent::SpawnTracerPresentationLocally(const FMABSTracerCueEvent& TracerEvent)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const FRotator TracerRotation = (TracerEvent.TraceEnd - TracerEvent.TraceStart).Rotation();
	if (TracerEvent.VFX != nullptr)
	{
		if (UNiagaraComponent* const TracerComponent = AcquireReusableNiagaraComponent(
			TracerEvent.VFX,
			TracerEvent.TraceStart,
			TracerRotation,
			ReusableTracerVFXPool))
		{
			TracerComponent->SetVectorParameter(NiagaraTracerTraceStartParameter, TracerEvent.TraceStart);
			TracerComponent->SetVectorParameter(NiagaraTracerTraceEndParameter, TracerEvent.TraceEnd);
			TracerComponent->SetVectorParameter(NiagaraTracerImpactPointParameter, TracerEvent.TraceEnd);
			TracerComponent->Activate(true);
		}
	}

	if (TracerEvent.SFX != nullptr)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, TracerEvent.SFX, TracerEvent.TraceStart, TracerRotation);
	}

	EmitDebugEvent(
		MABSAbilityComponentEventNames::TracerCueRealized,
		TracerEvent.AbilityTag,
		TracerEvent.AbilityHandle,
		TracerEvent.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Realized tracer cue locally with policy %s from %s to %s. Assets: %s."),
			*GetPresentationVisibilityPolicyLabel(TracerEvent.VisibilityPolicy),
			*FormatVectorForDebug(TracerEvent.TraceStart),
			*FormatVectorForDebug(TracerEvent.TraceEnd),
			*DescribeTracerCueAssets(TracerEvent)));
}

bool UMABSAbilityComponent::ValidateTargetActorForAbility(
	const UMABSAbilityDefinition* AbilityDefinition,
	const AActor* SourceActor,
	const AActor* CandidateActor,
	const bool bRequireValidActorTarget,
	const bool bAllowSelfTarget,
	FString& OutRejectionReason) const
{
	if (AbilityDefinition == nullptr)
	{
		OutRejectionReason = TEXT("the ability definition was invalid.");
		return false;
	}

	if (CandidateActor == nullptr)
	{
		OutRejectionReason = TEXT("the trace did not hit an actor.");
		return false;
	}

	if (CandidateActor == SourceActor && !bAllowSelfTarget)
	{
		OutRejectionReason = TEXT("self actor hits are not valid targets for this ability.");
		return false;
	}

	if (!bRequireValidActorTarget)
	{
		return true;
	}

	const bool bNeedsDamageValidation =
		AbilityDefinition->InstantEffectType == EMABSInstantEffectType::Damage
		|| (HasPeriodicGameplayEffect(AbilityDefinition)
			&& AbilityDefinition->PeriodicEffect.EffectType == EMABSPeriodicEffectType::DOT);
	const bool bNeedsHealValidation =
		AbilityDefinition->InstantEffectType == EMABSInstantEffectType::Heal
		|| (HasPeriodicGameplayEffect(AbilityDefinition)
			&& AbilityDefinition->PeriodicEffect.EffectType == EMABSPeriodicEffectType::HOT);

	if (!bNeedsDamageValidation && !bNeedsHealValidation)
	{
		OutRejectionReason = TEXT("the ability does not author a supported target validation effect.");
		return false;
	}

	if (bNeedsDamageValidation && !(CandidateActor->CanBeDamaged() || CandidateActor->IsA<APawn>()))
	{
		OutRejectionReason = TEXT("the actor is not damageable and is not a pawn.");
		return false;
	}

	if (bNeedsHealValidation && !CandidateActor->GetClass()->ImplementsInterface(UMABSInstantEffectReceiver::StaticClass()))
	{
		OutRejectionReason = TEXT("the actor does not implement IMABSInstantEffectReceiver.");
		return false;
	}

	return true;
}

void UMABSAbilityComponent::DrawTargetTraceDebug(
	const UMABSAbilityDefinition& AbilityDefinition,
	const FMABSTargetTraceDebugInfo& DebugInfo) const
{
	if (!AbilityDefinition.bDrawTargetTraceDebug || !DebugInfo.bHasTraceData)
	{
		return;
	}

	UWorld* const World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const FColor TraceColor = DebugInfo.bAcceptedTarget
		? FColor::Green
		: (DebugInfo.bHit ? FColor::Red : FColor::Yellow);

	DrawDebugLine(
		World,
		DebugInfo.TraceStart,
		DebugInfo.TraceEnd,
		TraceColor,
		false,
		AbilityDefinition.TargetTraceDebugDuration,
		0,
		1.5f);

	if (DebugInfo.TraceMode == EMABSTargetTraceMode::Sphere && DebugInfo.TraceRadius > 0.0f)
	{
		DrawDebugSphere(
			World,
			DebugInfo.TraceStart,
			DebugInfo.TraceRadius,
			16,
			FColor::Cyan,
			false,
			AbilityDefinition.TargetTraceDebugDuration);

		DrawDebugSphere(
			World,
			DebugInfo.TraceEnd,
			DebugInfo.TraceRadius,
			16,
			FColor::Cyan,
			false,
			AbilityDefinition.TargetTraceDebugDuration);
	}

	if (DebugInfo.bHit)
	{
		DrawDebugPoint(
			World,
			DebugInfo.HitLocation,
			18.0f,
			TraceColor,
			false,
			AbilityDefinition.TargetTraceDebugDuration);

		DrawDebugString(
			World,
			DebugInfo.HitLocation + FVector(0.0f, 0.0f, 24.0f),
			DebugInfo.ResultMessage,
			nullptr,
			TraceColor,
			AbilityDefinition.TargetTraceDebugDuration,
			false);
	}
}

bool UMABSAbilityComponent::CanMutateAbilityState() const
{
	const AActor* Owner = GetOwner();
	return Owner != nullptr && Owner->HasAuthority();
}

FMABSAbilityHandle UMABSAbilityComponent::MakeNextAbilityHandle()
{
	return FMABSAbilityHandle(NextAbilityHandleValue++);
}

int32 UMABSAbilityComponent::MakeNextPeriodicEffectRuntimeId()
{
	return NextPeriodicEffectRuntimeIdValue++;
}
