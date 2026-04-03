#include "Components/MABSAbilityRuntime_Internal.h"

using namespace MABSAbilityRuntimeInternal;

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


void UMABSAbilityComponent::ServerTryActivateAbilityByTag_Implementation(const FGameplayTag AbilityTag)
{
	HandleTryActivateAbility(AbilityTag, true, AbilityTag);
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
