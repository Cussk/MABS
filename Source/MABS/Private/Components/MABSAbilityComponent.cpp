// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/MABSAbilityComponent.h"
#include "Data/MABSAbilityDefinition.h"
#include "Debug/MABSAbilitySystemLogs.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace MABSAbilityComponentEventNames
{
	static const FName AbilityGrantFailed(TEXT("AbilityGrantFailed"));
	static const FName AbilityGranted(TEXT("AbilityGranted"));
	static const FName ActivationRejected(TEXT("ActivationRejected"));
	static const FName ActivationRequested(TEXT("ActivationRequested"));
}

UMABSAbilityComponent::UMABSAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

FMABSAbilityHandle UMABSAbilityComponent::GrantAbility(UMABSAbilityDefinition* AbilityDefinition)
{
	const FMABSAbilityHandle InvalidHandle;

	if (AbilityDefinition == nullptr)
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::AbilityGrantFailed,
			FGameplayTag(),
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::InvalidDefinition,
			TEXT("GrantAbility failed because the ability definition was null."));
		return InvalidHandle;
	}

	if (!CanMutateAbilityState())
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::AbilityGrantFailed,
			AbilityDefinition->AbilityTag,
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::RequiresAuthority,
			TEXT("GrantAbility must run on the authoritative owner."));
		return InvalidHandle;
	}

	FMABSAbilitySpec AbilitySpec;
	AbilitySpec.Handle = MakeNextAbilityHandle();
	AbilitySpec.AbilityDefinition = AbilityDefinition;
	AbilitySpec.RuntimeState = EMABSAbilityRuntimeState::Granted;
	GrantedAbilities.Add(AbilitySpec);

	EmitDebugEvent(
		MABSAbilityComponentEventNames::AbilityGranted,
		AbilityDefinition->AbilityTag,
		AbilitySpec.Handle,
		AbilitySpec.RuntimeState,
		EMABSAbilityActivationResult::Success,
		FString::Printf(TEXT("Granted ability definition '%s'."), *GetNameSafe(AbilityDefinition)));

	return AbilitySpec.Handle;
}

EMABSAbilityActivationResult UMABSAbilityComponent::TryActivateAbilityByTag(FGameplayTag AbilityTag)
{
	const FMABSAbilityHandle InvalidHandle;

	if (!AbilityTag.IsValid())
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::ActivationRejected,
			AbilityTag,
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::InvalidRequest,
			TEXT("TryActivateAbilityByTag requires a valid ability tag."));
		return EMABSAbilityActivationResult::InvalidRequest;
	}

	if (!CanMutateAbilityState())
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::ActivationRejected,
			AbilityTag,
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::RequiresAuthority,
			TEXT("Ability activation is server-authoritative in Phase 00."));
		return EMABSAbilityActivationResult::RequiresAuthority;
	}

	FMABSAbilitySpec* AbilitySpec = FindGrantedAbilityByTag(AbilityTag);
	if (AbilitySpec == nullptr || AbilitySpec->AbilityDefinition == nullptr)
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::ActivationRejected,
			AbilityTag,
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::AbilityNotGranted,
			TEXT("No granted ability matched the requested tag."));
		return EMABSAbilityActivationResult::AbilityNotGranted;
	}

	EmitDebugEvent(
		MABSAbilityComponentEventNames::ActivationRequested,
		AbilityTag,
		AbilitySpec->Handle,
		AbilitySpec->RuntimeState,
		EMABSAbilityActivationResult::NotImplemented,
		TEXT("Phase 00 records activation requests but does not execute ability logic yet."));
	return EMABSAbilityActivationResult::NotImplemented;
}

TArray<FMABSAbilitySpec> UMABSAbilityComponent::GetGrantedAbilities() const
{
	return GrantedAbilities;
}

TArray<FMABSAbilityDebugEvent> UMABSAbilityComponent::GetRecentDebugEvents() const
{
	return RecentDebugEvents;
}

FMABSAbilitySpec* UMABSAbilityComponent::FindGrantedAbilityByTag(const FGameplayTag& AbilityTag)
{
	return GrantedAbilities.FindByPredicate(
		[&AbilityTag](const FMABSAbilitySpec& AbilitySpec)
		{
			return AbilitySpec.AbilityDefinition != nullptr
				&& AbilitySpec.AbilityDefinition->AbilityTag == AbilityTag;
		});
}

void UMABSAbilityComponent::EmitDebugEvent(
	FName EventName,
	const FGameplayTag& AbilityTag,
	const FMABSAbilityHandle& AbilityHandle,
	EMABSAbilityRuntimeState RuntimeState,
	EMABSAbilityActivationResult ActivationResult,
	const FString& Message)
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

	RecentDebugEvents.Add(DebugEvent);
	if (RecentDebugEvents.Num() > MaxStoredDebugEvents)
	{
		const int32 NumberToRemove = RecentDebugEvents.Num() - MaxStoredDebugEvents;
		RecentDebugEvents.RemoveAt(0, NumberToRemove, EAllowShrinking::No);
	}

	const UEnum* RuntimeStateEnum = StaticEnum<EMABSAbilityRuntimeState>();
	const UEnum* ResultEnum = StaticEnum<EMABSAbilityActivationResult>();
	const FString RuntimeStateName = RuntimeStateEnum != nullptr
		? RuntimeStateEnum->GetNameStringByValue(static_cast<int64>(RuntimeState))
		: TEXT("Unknown");
	const FString ResultName = ResultEnum != nullptr
		? ResultEnum->GetNameStringByValue(static_cast<int64>(ActivationResult))
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

bool UMABSAbilityComponent::CanMutateAbilityState() const
{
	const AActor* Owner = GetOwner();
	return Owner != nullptr && Owner->HasAuthority();
}

FMABSAbilityHandle UMABSAbilityComponent::MakeNextAbilityHandle()
{
	return FMABSAbilityHandle(NextAbilityHandleValue++);
}
