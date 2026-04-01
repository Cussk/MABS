// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/MABSAbilityComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Data/MABSAbilityDefinition.h"
#include "Debug/MABSAbilitySystemLogs.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/MABSInstantEffectReceiver.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

namespace MABSAbilityComponentEventNames
{
	static const FName AbilityBlocked(TEXT("AbilityBlocked"));
	static const FName AbilityGranted(TEXT("AbilityGranted"));
	static const FName AbilityGrantRejected(TEXT("AbilityGrantRejected"));
	static const FName AbilityUnblocked(TEXT("AbilityUnblocked"));
	static const FName CommitSucceeded(TEXT("CommitSucceeded"));
	static const FName EffectApplied(TEXT("EffectApplied"));
	static const FName EffectApplicationFailed(TEXT("EffectApplicationFailed"));
	static const FName RequestAccepted(TEXT("RequestAccepted"));
	static const FName RequestRejected(TEXT("RequestRejected"));
	static const FName RequestSentToServer(TEXT("RequestSentToServer"));
	static const FName RequestStarted(TEXT("RequestStarted"));
	static const FName TargetTraceHit(TEXT("TargetTraceHit"));
	static const FName TargetTraceRejected(TEXT("TargetTraceRejected"));
	static const FName TargetTraceStarted(TEXT("TargetTraceStarted"));
	static const FName TargetResolved(TEXT("TargetResolved"));
	static const FName TargetResolutionFailed(TEXT("TargetResolutionFailed"));
}

namespace
{
	FString DescribeActivationResult(const EMABSAbilityActivationResult ActivationResult)
	{
		switch (ActivationResult)
		{
		case EMABSAbilityActivationResult::InvalidAbility:
			return TEXT("Activation rejected because the requested ability was invalid.");

		case EMABSAbilityActivationResult::NotGranted:
			return TEXT("Activation rejected because the ability has not been granted.");

		case EMABSAbilityActivationResult::AlreadyActive:
			return TEXT("Activation rejected because the ability is already active.");

		case EMABSAbilityActivationResult::Blocked:
			return TEXT("Activation rejected because the ability is currently blocked.");

		case EMABSAbilityActivationResult::AuthorityRejected:
			return TEXT("Activation rejected because the request must be handled by authority.");

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

	FString GetHitActorLabel(const AActor* Actor)
	{
		return Actor != nullptr ? GetNameSafe(Actor) : TEXT("World");
	}

	FCollisionObjectQueryParams MakeTargetTraceObjectQueryParams(const UMABSAbilityDefinition& AbilityDefinition)
	{
		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Destructible);

		if (!AbilityDefinition.bIgnoreNonTargetWorldHits)
		{
			ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
		}

		return ObjectQueryParams;
	}
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
		return HandleTryActivateAbility(AbilityTag, false);
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

TArray<FMABSAbilityDebugEvent> UMABSAbilityComponent::GetRecentDebugEvents() const
{
	return RecentDebugEvents;
}

FMABSTargetTraceDebugInfo UMABSAbilityComponent::GetLatestTargetTraceDebugInfo() const
{
	return LatestTargetTraceDebugInfo;
}

void UMABSAbilityComponent::ServerTryActivateAbilityByTag_Implementation(const FGameplayTag AbilityTag)
{
	HandleTryActivateAbility(AbilityTag, true);
}

void UMABSAbilityComponent::ClientReceiveAbilityDebugEvent_Implementation(const FMABSAbilityDebugEvent& DebugEvent)
{
	RecordDebugEvent(DebugEvent);
}

void UMABSAbilityComponent::ClientReceiveTargetTraceDebugInfo_Implementation(const FMABSTargetTraceDebugInfo& DebugInfo)
{
	RecordLatestTargetTraceDebugInfo(DebugInfo);
}

EMABSAbilityActivationResult UMABSAbilityComponent::HandleTryActivateAbility(const FGameplayTag& AbilityTag, const bool bNotifyOwningClient)
{
	const FMABSAbilityHandle InvalidHandle;

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

	const EMABSAbilityActivationResult ActivationResult = CanActivateAbility(*AbilitySpec);
	AbilitySpec->LastActivationResult = ActivationResult;

	if (ActivationResult != EMABSAbilityActivationResult::Success)
	{
		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::RequestRejected,
			AbilitySpec->AbilityTag,
			AbilitySpec->Handle,
			AbilitySpec->RuntimeState,
			ActivationResult,
			DescribeActivationResult(ActivationResult));
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DebugEvent);
		}

		return ActivationResult;
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

	return CommitAbility(*AbilitySpec, bNotifyOwningClient);
}

EMABSAbilityActivationResult UMABSAbilityComponent::CanActivateAbility(const FMABSAbilitySpec& AbilitySpec) const
{
	if (AbilitySpec.AbilityDefinition == nullptr || !AbilitySpec.AbilityTag.IsValid())
	{
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (AbilitySpec.AbilityDefinition->AbilityTag != AbilitySpec.AbilityTag)
	{
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (AbilitySpec.AbilityDefinition->TargetType != EMABSTargetType::Self
		&& AbilitySpec.AbilityDefinition->TargetType != EMABSTargetType::Actor)
	{
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (AbilitySpec.AbilityDefinition->InstantEffectType == EMABSInstantEffectType::None
		|| AbilitySpec.AbilityDefinition->EffectMagnitude <= 0.0f)
	{
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (AbilitySpec.AbilityDefinition->TargetType == EMABSTargetType::Actor
		&& AbilitySpec.AbilityDefinition->TargetTraceDistance <= 0.0f)
	{
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (AbilitySpec.AbilityDefinition->TargetType == EMABSTargetType::Actor
		&& AbilitySpec.AbilityDefinition->ActorTargetTraceMode == EMABSTargetTraceMode::Sphere
		&& AbilitySpec.AbilityDefinition->TargetTraceRadius <= 0.0f)
	{
		return EMABSAbilityActivationResult::InvalidAbility;
	}

	if (AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Active)
	{
		return EMABSAbilityActivationResult::AlreadyActive;
	}

	if (AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Blocked)
	{
		return EMABSAbilityActivationResult::Blocked;
	}

	return EMABSAbilityActivationResult::Success;
}

EMABSAbilityActivationResult UMABSAbilityComponent::CommitAbility(FMABSAbilitySpec& AbilitySpec, const bool bNotifyOwningClient)
{
	ClearLatestTargetTraceDebugInfo(bNotifyOwningClient);

	FString TargetDebugMessage;
	AActor* const TargetActor = ResolveAbilityTarget(AbilitySpec, TargetDebugMessage, bNotifyOwningClient);
	if (TargetActor == nullptr)
	{
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::TargetResolutionFailed;

		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::TargetResolutionFailed,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EMABSAbilityActivationResult::TargetResolutionFailed,
			TargetDebugMessage);
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
		TargetDebugMessage);
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(TargetResolvedEvent);
	}

	AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Active;

	FString EffectDebugMessage;
	const EMABSAbilityActivationResult EffectResult = ApplyInstantEffect(AbilitySpec, TargetActor, EffectDebugMessage);
	if (EffectResult != EMABSAbilityActivationResult::Success)
	{
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		AbilitySpec.LastActivationResult = EffectResult;

		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::EffectApplicationFailed,
			AbilitySpec.AbilityTag,
			AbilitySpec.Handle,
			AbilitySpec.RuntimeState,
			EffectResult,
			EffectDebugMessage);
		if (bNotifyOwningClient)
		{
			EmitDebugEventToOwningClient(DebugEvent);
		}

		return EffectResult;
	}

	AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::Success;

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

	const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::CommitSucceeded,
		AbilitySpec.AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		TEXT("Ability activation committed successfully."));
	if (bNotifyOwningClient)
	{
		EmitDebugEventToOwningClient(DebugEvent);
	}

	if (UWorld* World = GetWorld())
	{
		FTimerDelegate ResetDelegate = FTimerDelegate::CreateUObject(this, &UMABSAbilityComponent::ResetAbilityToIdle, AbilitySpec.Handle);
		World->GetTimerManager().SetTimerForNextTick(ResetDelegate);
	}
	else
	{
		ResetAbilityToIdle(AbilitySpec.Handle);
	}

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
			UWorld* const World = GetWorld();
			if (World == nullptr)
			{
				OutDebugMessage = TEXT("Failed to resolve an actor target because the world was unavailable.");
				return nullptr;
			}

			FVector TraceStart = FVector::ZeroVector;
			FRotator TraceRotation = FRotator::ZeroRotator;
			FString ViewPointDescription;
			if (!GetTargetTraceViewPoint(TraceStart, TraceRotation, ViewPointDescription))
			{
				OutDebugMessage = TEXT("Failed to resolve an actor target because the trace viewpoint was unavailable.");
				return nullptr;
			}

			FMABSTargetTraceDebugInfo TraceDebugInfo;
			TraceDebugInfo.bHasTraceData = true;
			TraceDebugInfo.AbilityTag = AbilitySpec.AbilityTag;
			TraceDebugInfo.AbilityHandle = AbilitySpec.Handle;
			TraceDebugInfo.TraceMode = AbilityDefinition->ActorTargetTraceMode;
			FVector GameplayTraceStart = TraceStart;
			FRotator UnusedTraceRotation = TraceRotation;
			Owner->GetActorEyesViewPoint(GameplayTraceStart, UnusedTraceRotation);

			TraceDebugInfo.TraceStart = GameplayTraceStart;
			TraceDebugInfo.TraceEnd = GameplayTraceStart + (TraceRotation.Vector() * AbilityDefinition->TargetTraceDistance);
			TraceDebugInfo.TraceRadius = AbilityDefinition->ActorTargetTraceMode == EMABSTargetTraceMode::Sphere
				? AbilityDefinition->TargetTraceRadius
				: 0.0f;
			TraceDebugInfo.WorldTimeSeconds = World->GetTimeSeconds();
			TraceDebugInfo.ViewPointDescription = FString::Printf(
				TEXT("%sAim + OwnerEyesStart"),
				*ViewPointDescription);

			const FString TraceStartedMessage = FString::Printf(
				TEXT("Started %s target trace from %s to %s using viewpoint '%s' with distance %.0f and radius %.0f."),
				*GetTargetTraceModeLabel(TraceDebugInfo.TraceMode),
				*FormatVectorForDebug(TraceDebugInfo.TraceStart),
				*FormatVectorForDebug(TraceDebugInfo.TraceEnd),
				*TraceDebugInfo.ViewPointDescription,
				AbilityDefinition->TargetTraceDistance,
				TraceDebugInfo.TraceRadius);
			const FMABSAbilityDebugEvent TraceStartedEvent = EmitDebugEvent(
				MABSAbilityComponentEventNames::TargetTraceStarted,
				AbilitySpec.AbilityTag,
				AbilitySpec.Handle,
				AbilitySpec.RuntimeState,
				EMABSAbilityActivationResult::Success,
				TraceStartedMessage);
			if (bNotifyOwningClient)
			{
				EmitDebugEventToOwningClient(TraceStartedEvent);
			}

			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MABSAbilityTargetTrace), false);
			QueryParams.AddIgnoredActor(Owner);
			const FCollisionObjectQueryParams ObjectQueryParams = MakeTargetTraceObjectQueryParams(*AbilityDefinition);

			TArray<FHitResult> HitResults;
			if (AbilityDefinition->ActorTargetTraceMode == EMABSTargetTraceMode::Sphere
				&& AbilityDefinition->TargetTraceRadius > 0.0f)
			{
				const FCollisionShape TraceShape = FCollisionShape::MakeSphere(AbilityDefinition->TargetTraceRadius);
				World->SweepMultiByObjectType(
					HitResults,
					TraceDebugInfo.TraceStart,
					TraceDebugInfo.TraceEnd,
					TraceRotation.Quaternion(),
					ObjectQueryParams,
					TraceShape,
					QueryParams);
			}
			else
			{
				World->LineTraceMultiByObjectType(
					HitResults,
					TraceDebugInfo.TraceStart,
					TraceDebugInfo.TraceEnd,
					ObjectQueryParams,
					QueryParams);
			}

			if (HitResults.IsEmpty())
			{
				TraceDebugInfo.ResultMessage = FString::Printf(
					TEXT("Target trace found no blocking hit within %.0f units."),
					AbilityDefinition->TargetTraceDistance);
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
				TraceDebugInfo.bHit = true;
				TraceDebugInfo.HitLocation = HitResult.ImpactPoint;
				TraceDebugInfo.HitDistance = FVector::Distance(TraceDebugInfo.TraceStart, HitResult.ImpactPoint);
				TraceDebugInfo.HitActorName = GetHitActorLabel(HitActor);
				TraceDebugInfo.HitComponentName = GetNameSafe(HitResult.GetComponent());
				TraceDebugInfo.WorldTimeSeconds = World->GetTimeSeconds();

				if (HitActor == nullptr)
				{
					TraceDebugInfo.bAcceptedTarget = false;
					TraceDebugInfo.ResultMessage = AbilityDefinition->bIgnoreNonTargetWorldHits
						? FString::Printf(
							TEXT("Rejected world hit on component '%s' at %.0f units and continued searching for a valid actor target."),
							*TraceDebugInfo.HitComponentName,
							TraceDebugInfo.HitDistance)
						: FString::Printf(
							TEXT("Rejected world hit on component '%s' at %.0f units because non-target world hits are not ignored."),
							*TraceDebugInfo.HitComponentName,
							TraceDebugInfo.HitDistance);

					const FMABSAbilityDebugEvent TraceRejectedEvent = EmitDebugEvent(
						MABSAbilityComponentEventNames::TargetTraceRejected,
						AbilitySpec.AbilityTag,
						AbilitySpec.Handle,
						AbilitySpec.RuntimeState,
						EMABSAbilityActivationResult::TargetResolutionFailed,
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
					if (AbilityDefinition->bIgnoreNonTargetWorldHits)
					{
						continue;
					}

					DrawTargetTraceDebug(*AbilityDefinition, TraceDebugInfo);
					OutDebugMessage = TraceDebugInfo.ResultMessage;
					return nullptr;
				}

				FString RejectionReason;
				if (!ValidateResolvedTargetActor(AbilitySpec, HitActor, RejectionReason))
				{
					TraceDebugInfo.bAcceptedTarget = false;
					TraceDebugInfo.ResultMessage = FString::Printf(
						TEXT("Rejected actor target '%s' on component '%s' at %.0f units: %s"),
						*TraceDebugInfo.HitActorName,
						*TraceDebugInfo.HitComponentName,
						TraceDebugInfo.HitDistance,
						*RejectionReason);

					const FMABSAbilityDebugEvent TraceRejectedEvent = EmitDebugEvent(
						MABSAbilityComponentEventNames::TargetTraceRejected,
						AbilitySpec.AbilityTag,
						AbilitySpec.Handle,
						AbilitySpec.RuntimeState,
						EMABSAbilityActivationResult::TargetResolutionFailed,
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
					TEXT("Accepted actor target '%s' on component '%s' at %.0f units using %s trace."),
					*TraceDebugInfo.HitActorName,
					*TraceDebugInfo.HitComponentName,
					TraceDebugInfo.HitDistance,
					*GetTargetTraceModeLabel(TraceDebugInfo.TraceMode));

				const FMABSAbilityDebugEvent TraceHitEvent = EmitDebugEvent(
					MABSAbilityComponentEventNames::TargetTraceHit,
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
				DrawTargetTraceDebug(*AbilityDefinition, TraceDebugInfo);
				OutDebugMessage = FString::Printf(
					TEXT("Resolved target '%s' using target type '%s'. %s"),
					*GetNameSafe(HitActor),
					*GetTargetTypeLabel(AbilityDefinition->TargetType),
					*TraceDebugInfo.ResultMessage);
				return HitActor;
			}

			DrawTargetTraceDebug(*AbilityDefinition, TraceDebugInfo);
			OutDebugMessage = TraceDebugInfo.ResultMessage.IsEmpty()
				? FString::Printf(
					TEXT("Failed to resolve an actor target using target type '%s' within %.0f units."),
					*GetTargetTypeLabel(AbilityDefinition->TargetType),
					AbilityDefinition->TargetTraceDistance)
				: TraceDebugInfo.ResultMessage;
			return nullptr;
		}

	default:
		OutDebugMessage = FString::Printf(
			TEXT("Failed to resolve a target because target type '%s' is not supported in Phase 2."),
			*GetTargetTypeLabel(AbilityDefinition->TargetType));
		return nullptr;
	}
}

EMABSAbilityActivationResult UMABSAbilityComponent::ApplyInstantEffect(
	const FMABSAbilitySpec& AbilitySpec,
	AActor* TargetActor,
	FString& OutDebugMessage) const
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	AActor* const Owner = GetOwner();
	if (AbilityDefinition == nullptr || Owner == nullptr || TargetActor == nullptr)
	{
		OutDebugMessage = TEXT("Failed to apply the instant effect because the ability definition, owner, or target was invalid.");
		return EMABSAbilityActivationResult::EffectApplicationFailed;
	}

	switch (AbilityDefinition->InstantEffectType)
	{
	case EMABSInstantEffectType::Damage:
		{
			const float AppliedDamage = UGameplayStatics::ApplyDamage(
				TargetActor,
				AbilityDefinition->EffectMagnitude,
				Owner->GetInstigatorController(),
				Owner,
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
				Owner,
				AbilitySpec.AbilityDefinition);
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
			TEXT("Failed to apply the instant effect because effect type '%s' is not supported in Phase 2."),
			*GetInstantEffectTypeLabel(AbilityDefinition->InstantEffectType));
		return EMABSAbilityActivationResult::EffectApplicationFailed;
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

void UMABSAbilityComponent::ResetAbilityToIdle(const FMABSAbilityHandle AbilityHandle)
{
	FMABSAbilitySpec* AbilitySpec = FindGrantedAbilitySpecByHandle(AbilityHandle);
	if (AbilitySpec == nullptr)
	{
		return;
	}

	if (AbilitySpec->RuntimeState == EMABSAbilityRuntimeState::Active)
	{
		AbilitySpec->RuntimeState = EMABSAbilityRuntimeState::Idle;
	}
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

bool UMABSAbilityComponent::ValidateResolvedTargetActor(
	const FMABSAbilitySpec& AbilitySpec,
	const AActor* CandidateActor,
	FString& OutRejectionReason) const
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	const AActor* const Owner = GetOwner();
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

	if (CandidateActor == Owner)
	{
		OutRejectionReason = TEXT("self actor hits are not valid for actor targeting.");
		return false;
	}

	if (!AbilityDefinition->bRequireValidActorTarget)
	{
		return true;
	}

	switch (AbilityDefinition->InstantEffectType)
	{
	case EMABSInstantEffectType::Damage:
		if (CandidateActor->CanBeDamaged() || CandidateActor->IsA<APawn>())
		{
			return true;
		}

		OutRejectionReason = TEXT("the actor is not damageable and is not a pawn.");
		return false;

	case EMABSInstantEffectType::Heal:
		if (CandidateActor->GetClass()->ImplementsInterface(UMABSInstantEffectReceiver::StaticClass()))
		{
			return true;
		}

		OutRejectionReason = TEXT("the actor does not implement IMABSInstantEffectReceiver.");
		return false;

	default:
		OutRejectionReason = TEXT("the authored instant effect type does not support actor validation.");
		return false;
	}
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
