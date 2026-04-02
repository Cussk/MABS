// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/MABSAbilityComponent.h"
#include "Actors/MABSProjectileBase.h"
#include "Components/PrimitiveComponent.h"
#include "Data/MABSAbilityDefinition.h"
#include "Debug/MABSAbilitySystemLogs.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/MABSCostReceiver.h"
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
	static const FName CooldownRejected(TEXT("CooldownRejected"));
	static const FName CooldownStarted(TEXT("CooldownStarted"));
	static const FName DeliveryFailed(TEXT("DeliveryFailed"));
	static const FName DeliveryStarted(TEXT("DeliveryStarted"));
	static const FName EffectApplied(TEXT("EffectApplied"));
	static const FName EffectApplicationFailed(TEXT("EffectApplicationFailed"));
	static const FName CostRejected(TEXT("CostRejected"));
	static const FName CostSpent(TEXT("CostSpent"));
	static const FName CostValidated(TEXT("CostValidated"));
	static const FName HitTraceHit(TEXT("HitTraceHit"));
	static const FName HitTraceRejected(TEXT("HitTraceRejected"));
	static const FName MeleeHit(TEXT("MeleeHit"));
	static const FName MeleeRejected(TEXT("MeleeRejected"));
	static const FName ProjectileImpact(TEXT("ProjectileImpact"));
	static const FName ProjectileImpactRejected(TEXT("ProjectileImpactRejected"));
	static const FName ProjectileSpawned(TEXT("ProjectileSpawned"));
	static const FName ProjectileSpawnFailed(TEXT("ProjectileSpawnFailed"));
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

	FName GetActivationFailureEventName(const EMABSAbilityActivationResult ActivationResult)
	{
		switch (ActivationResult)
		{
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

	return CommitAbility(*AbilitySpec, bNotifyOwningClient);
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

	if (AbilityDefinition->InstantEffectType == EMABSInstantEffectType::None
		|| AbilityDefinition->EffectMagnitude <= 0.0f)
	{
		OutDebugMessage = TEXT("Activation rejected because the authored instant effect is invalid.");
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

	if (AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Active)
	{
		return EMABSAbilityActivationResult::AlreadyActive;
	}

	if (AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Blocked)
	{
		return EMABSAbilityActivationResult::Blocked;
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

EMABSAbilityActivationResult UMABSAbilityComponent::CommitAbility(FMABSAbilitySpec& AbilitySpec, const bool bNotifyOwningClient)
{
	ClearLatestTargetTraceDebugInfo(bNotifyOwningClient);

	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	if (AbilityDefinition == nullptr)
	{
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::InvalidAbility;

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
			AActor* const TargetActor = ExecuteDirectDelivery(AbilitySpec, DeliveryDebugMessage, bNotifyOwningClient);
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

			return CompleteResolvedTargetAbility(AbilitySpec, TargetActor, DeliveryMode, bNotifyOwningClient);
		}

	case EMABSDeliveryMode::HitTrace:
		{
			FString DeliveryDebugMessage;
			AActor* const TargetActor = ExecuteHitTraceDelivery(AbilitySpec, DeliveryDebugMessage, bNotifyOwningClient);
			if (TargetActor == nullptr)
			{
				AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
				AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::DeliveryFailed;

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

			return CompleteResolvedTargetAbility(AbilitySpec, TargetActor, DeliveryMode, bNotifyOwningClient);
		}

	case EMABSDeliveryMode::Melee:
		{
			FString DeliveryDebugMessage;
			AActor* const TargetActor = ExecuteMeleeDelivery(AbilitySpec, DeliveryDebugMessage, bNotifyOwningClient);
			if (TargetActor == nullptr)
			{
				AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
				AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::DeliveryFailed;

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

			return CompleteResolvedTargetAbility(AbilitySpec, TargetActor, DeliveryMode, bNotifyOwningClient);
		}

	case EMABSDeliveryMode::Projectile:
		return ExecuteProjectileDelivery(AbilitySpec, bNotifyOwningClient);

	default:
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::InvalidAbility;

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

AActor* UMABSAbilityComponent::ExecuteDirectDelivery(
	const FMABSAbilitySpec& AbilitySpec,
	FString& OutDebugMessage,
	const bool bNotifyOwningClient)
{
	return ResolveAbilityTarget(AbilitySpec, OutDebugMessage, bNotifyOwningClient);
}

AActor* UMABSAbilityComponent::ExecuteHitTraceDelivery(
	const FMABSAbilitySpec& AbilitySpec,
	FString& OutDebugMessage,
	const bool bNotifyOwningClient)
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	AActor* const Owner = GetOwner();
	if (AbilityDefinition == nullptr || Owner == nullptr)
	{
		OutDebugMessage = TEXT("HitTrace delivery failed because the ability definition or owning actor was invalid.");
		return nullptr;
	}

	FVector TraceStart = FVector::ZeroVector;
	FRotator TraceRotation = FRotator::ZeroRotator;
	FString ViewPointDescription;
	if (!GetTargetTraceViewPoint(TraceStart, TraceRotation, ViewPointDescription))
	{
		OutDebugMessage = TEXT("HitTrace delivery failed because the trace viewpoint was unavailable.");
		return nullptr;
	}

	FVector GameplayTraceStart = TraceStart;
	FRotator UnusedTraceRotation = TraceRotation;
	Owner->GetActorEyesViewPoint(GameplayTraceStart, UnusedTraceRotation);
	const FVector TraceEnd = GameplayTraceStart + (TraceRotation.Vector() * AbilityDefinition->HitTraceDistance);
	return ResolveActorTargetFromTrace(
		AbilitySpec,
		GameplayTraceStart,
		TraceEnd,
		EMABSDeliveryMode::HitTrace,
		GetTraceModeForRadius(AbilityDefinition->HitTraceRadius),
		AbilityDefinition->HitTraceRadius,
		false,
		TEXT("HitTrace delivery"),
		FString::Printf(TEXT("%sAim + OwnerEyesStart"), *ViewPointDescription),
		NAME_None,
		MABSAbilityComponentEventNames::HitTraceHit,
		MABSAbilityComponentEventNames::HitTraceRejected,
		OutDebugMessage,
		bNotifyOwningClient);
}

AActor* UMABSAbilityComponent::ExecuteMeleeDelivery(
	const FMABSAbilitySpec& AbilitySpec,
	FString& OutDebugMessage,
	const bool bNotifyOwningClient)
{
	const UMABSAbilityDefinition* const AbilityDefinition = AbilitySpec.AbilityDefinition;
	AActor* const Owner = GetOwner();
	if (AbilityDefinition == nullptr || Owner == nullptr)
	{
		OutDebugMessage = TEXT("Melee delivery failed because the ability definition or owning actor was invalid.");
		return nullptr;
	}

	const FVector ForwardVector = Owner->GetActorForwardVector();
	const FVector TraceStart = Owner->GetActorLocation() + (ForwardVector * AbilityDefinition->MeleeForwardOffset);
	const FVector TraceEnd = TraceStart + (ForwardVector * AbilityDefinition->MeleeRange);
	return ResolveActorTargetFromTrace(
		AbilitySpec,
		TraceStart,
		TraceEnd,
		EMABSDeliveryMode::Melee,
		EMABSTargetTraceMode::Sphere,
		AbilityDefinition->MeleeRadius,
		false,
		TEXT("Melee delivery"),
		TEXT("OwnerForwardFromActorLocation"),
		NAME_None,
		MABSAbilityComponentEventNames::MeleeHit,
		MABSAbilityComponentEventNames::MeleeRejected,
		OutDebugMessage,
		bNotifyOwningClient);
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
	if (!GetProjectileSpawnTransform(*AbilityDefinition, SpawnTransform, SpawnTransformMessage))
	{
		AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Idle;
		AbilitySpec.LastActivationResult = EMABSAbilityActivationResult::DeliveryFailed;

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

	return FinalizeAbilityCommit(AbilitySpec, EMABSDeliveryMode::Projectile, bNotifyOwningClient);
}

EMABSAbilityActivationResult UMABSAbilityComponent::CompleteResolvedTargetAbility(
	FMABSAbilitySpec& AbilitySpec,
	AActor* TargetActor,
	const EMABSDeliveryMode DeliveryMode,
	const bool bNotifyOwningClient)
{
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
	const bool bNotifyOwningClient)
{
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
		if (!ValidateTargetActorForAbility(AbilityDefinition, Owner, HitActor, bRequireValidActorTarget, RejectionReason))
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

	FString ImpactDebugMessage;
	FString RejectionReason;
	if (!ValidateTargetActorForAbility(AbilityDefinition, SourceActor, HitActor, true, RejectionReason))
	{
		ImpactDebugMessage = HitActor == nullptr
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

	const EMABSAbilityActivationResult EffectResult = ApplyInstantEffectFromSource(
		AbilityDefinition,
		SourceActor,
		HitActor,
		ImpactDebugMessage);
	if (EffectResult != EMABSAbilityActivationResult::Success)
	{
		const FMABSAbilityDebugEvent DebugEvent = EmitDebugEvent(
			MABSAbilityComponentEventNames::ProjectileImpactRejected,
			Projectile.GetSourceAbilityTag(),
			Projectile.GetSourceAbilityHandle(),
			EMABSAbilityRuntimeState::Idle,
			EffectResult,
			ImpactDebugMessage);
		EmitDebugEventToOwningClient(DebugEvent);
		return EffectResult;
	}

	const FMABSAbilityDebugEvent EffectAppliedEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::EffectApplied,
		Projectile.GetSourceAbilityTag(),
		Projectile.GetSourceAbilityHandle(),
		EMABSAbilityRuntimeState::Idle,
		EMABSAbilityActivationResult::Success,
		ImpactDebugMessage);
	EmitDebugEventToOwningClient(EffectAppliedEvent);

	const FMABSAbilityDebugEvent ProjectileImpactEvent = EmitDebugEvent(
		MABSAbilityComponentEventNames::ProjectileImpact,
		Projectile.GetSourceAbilityTag(),
		Projectile.GetSourceAbilityHandle(),
		EMABSAbilityRuntimeState::Idle,
		EMABSAbilityActivationResult::Success,
		FString::Printf(
			TEXT("Projectile impacted '%s' at %s and applied its effect."),
			*GetNameSafe(HitActor),
			*FormatVectorForDebug(HitResult.ImpactPoint)));
	EmitDebugEventToOwningClient(ProjectileImpactEvent);

	return EMABSAbilityActivationResult::Success;
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

	if (AbilitySpec->RuntimeState == EMABSAbilityRuntimeState::Active)
	{
		AbilitySpec->RuntimeState = EMABSAbilityRuntimeState::Idle;
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

bool UMABSAbilityComponent::GetProjectileSpawnTransform(
	const UMABSAbilityDefinition& AbilityDefinition,
	FTransform& OutSpawnTransform,
	FString& OutDebugMessage) const
{
	const AActor* const Owner = GetOwner();
	if (Owner == nullptr)
	{
		OutDebugMessage = TEXT("Projectile delivery failed because the owning actor was invalid.");
		return false;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	FString ViewPointDescription;
	if (!GetTargetTraceViewPoint(ViewLocation, ViewRotation, ViewPointDescription))
	{
		OutDebugMessage = TEXT("Projectile delivery failed because the projectile spawn viewpoint was unavailable.");
		return false;
	}

	FVector SpawnOrigin = ViewLocation;
	FRotator UnusedSpawnRotation = ViewRotation;
	Owner->GetActorEyesViewPoint(SpawnOrigin, UnusedSpawnRotation);
	const FVector SpawnLocation = SpawnOrigin + ViewRotation.RotateVector(AbilityDefinition.ProjectileSpawnOffset);
	OutSpawnTransform = FTransform(ViewRotation, SpawnLocation);
	OutDebugMessage = FString::Printf(
		TEXT("Prepared projectile spawn at %s using viewpoint '%s'."),
		*FormatVectorForDebug(SpawnLocation),
		*ViewPointDescription);
	return true;
}

bool UMABSAbilityComponent::ValidateTargetActorForAbility(
	const UMABSAbilityDefinition* AbilityDefinition,
	const AActor* SourceActor,
	const AActor* CandidateActor,
	const bool bRequireValidActorTarget,
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

	if (CandidateActor == SourceActor)
	{
		OutRejectionReason = TEXT("self actor hits are not valid targets for this ability.");
		return false;
	}

	if (!bRequireValidActorTarget)
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
