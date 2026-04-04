#include "Components/MABSAbilityComponent.h"
#include "Components/MABSAbilityRuntime_Common.h"
#include "Components/MABSAbilityRuntime_EventNames.h"

#include "Data/MABSAbilityDefinition.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "Interfaces/MABSCostReceiver.h"
#include "Interfaces/MABSInstantEffectReceiver.h"
#include "Kismet/GameplayStatics.h"

using namespace MABSAbilityRuntimeInternal;

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
