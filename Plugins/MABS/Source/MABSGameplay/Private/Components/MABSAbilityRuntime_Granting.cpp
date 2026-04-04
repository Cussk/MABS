#include "Components/MABSAbilityComponent.h"
#include "Components/MABSAbilityRuntime_Common.h"
#include "Components/MABSAbilityRuntime_EventNames.h"

#include "Data/MABSAbilityDefinition.h"
#include "Data/MABSAbilitySet.h"

using namespace MABSAbilityRuntimeInternal;

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


int32 UMABSAbilityComponent::GrantAbilitySet(UMABSAbilitySet* AbilitySet)
{
	const FMABSAbilityHandle InvalidHandle;

	if (AbilitySet == nullptr)
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::AbilitySetGrantFailed,
			FGameplayTag(),
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::InvalidAbility,
			TEXT("GrantAbilitySet rejected because the ability set asset was null."));
		return 0;
	}

	if (!CanMutateAbilityState())
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::AbilitySetGrantFailed,
			FGameplayTag(),
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::AuthorityRejected,
			FString::Printf(
				TEXT("GrantAbilitySet must run on the authoritative owner for set '%s'."),
				*GetAbilitySetLabel(AbilitySet)));
		return 0;
	}

	int32 GrantedCount = 0;
	int32 SkippedCount = 0;
	int32 RejectedCount = 0;

	for (UMABSAbilityDefinition* const AbilityDefinition : AbilitySet->AbilityDefinitions)
	{
		if (AbilityDefinition == nullptr)
		{
			++SkippedCount;
			EmitDebugEvent(
				MABSAbilityComponentEventNames::AbilitySetGrantSkipped,
				FGameplayTag(),
				InvalidHandle,
				EMABSAbilityRuntimeState::None,
				EMABSAbilityActivationResult::None,
				FString::Printf(
					TEXT("GrantAbilitySet skipped a null ability entry in set '%s'."),
					*GetAbilitySetLabel(AbilitySet)));
			continue;
		}

		if (GrantAbility(AbilityDefinition).IsValid())
		{
			++GrantedCount;
			continue;
		}

		++RejectedCount;
	}

	const FName SummaryEventName = GrantedCount > 0
		? MABSAbilityComponentEventNames::AbilitySetGranted
		: (RejectedCount > 0
			? MABSAbilityComponentEventNames::AbilitySetGrantFailed
			: MABSAbilityComponentEventNames::AbilitySetGrantSkipped);
	const EMABSAbilityActivationResult SummaryResult = GrantedCount > 0
		? EMABSAbilityActivationResult::Success
		: (RejectedCount > 0
			? EMABSAbilityActivationResult::InvalidAbility
			: EMABSAbilityActivationResult::None);

	EmitDebugEvent(
		SummaryEventName,
		FGameplayTag(),
		InvalidHandle,
		EMABSAbilityRuntimeState::None,
		SummaryResult,
		FString::Printf(
			TEXT("Processed ability set '%s': %d granted, %d skipped, %d rejected, %d total entries."),
			*GetAbilitySetLabel(AbilitySet),
			GrantedCount,
			SkippedCount,
			RejectedCount,
			AbilitySet->AbilityDefinitions.Num()));

	return GrantedCount;
}


int32 UMABSAbilityComponent::GrantAbilitySets(const TArray<UMABSAbilitySet*>& AbilitySets)
{
	const FMABSAbilityHandle InvalidHandle;

	if (!CanMutateAbilityState())
	{
		EmitDebugEvent(
			MABSAbilityComponentEventNames::AbilitySetGrantFailed,
			FGameplayTag(),
			InvalidHandle,
			EMABSAbilityRuntimeState::None,
			EMABSAbilityActivationResult::AuthorityRejected,
			TEXT("GrantAbilitySets must run on the authoritative owner."));
		return 0;
	}

	int32 TotalGrantedCount = 0;
	for (UMABSAbilitySet* const AbilitySet : AbilitySets)
	{
		if (AbilitySet == nullptr)
		{
			EmitDebugEvent(
				MABSAbilityComponentEventNames::AbilitySetGrantSkipped,
				FGameplayTag(),
				InvalidHandle,
				EMABSAbilityRuntimeState::None,
				EMABSAbilityActivationResult::None,
				TEXT("GrantAbilitySets skipped a null ability set entry."));
			continue;
		}

		TotalGrantedCount += GrantAbilitySet(AbilitySet);
	}

	return TotalGrantedCount;
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
