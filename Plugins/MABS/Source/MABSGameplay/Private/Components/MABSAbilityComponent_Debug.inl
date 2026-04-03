TArray<FMABSAbilityDebugEvent> UMABSAbilityComponent::GetRecentDebugEvents() const
{
	return RecentDebugEvents;
}


TArray<FMABSGrantedAbilityDebugSummary> UMABSAbilityComponent::GetGrantedAbilityDebugSummaries() const
{
	TArray<FMABSGrantedAbilityDebugSummary> Summaries;
	Summaries.Reserve(GrantedAbilities.Num());

	const float CurrentWorldTime = GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : 0.0f;
	for (const FMABSAbilitySpec& AbilitySpec : GrantedAbilities)
	{
		FMABSGrantedAbilityDebugSummary Summary;
		Summary.AbilityHandle = AbilitySpec.Handle;
		Summary.AbilityTag = AbilitySpec.AbilityTag;
		Summary.DisplayName = GetAbilityDebugDisplayName(AbilitySpec.AbilityDefinition);
		Summary.RuntimeState = AbilitySpec.RuntimeState;
		Summary.LastActivationResult = AbilitySpec.LastActivationResult;
		Summary.bIsBlocked = AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Blocked;

		if (AbilitySpec.AbilityDefinition != nullptr)
		{
			Summary.DeliveryMode = AbilitySpec.AbilityDefinition->DeliveryMode;
			Summary.InstantEffectType = AbilitySpec.AbilityDefinition->InstantEffectType;
			Summary.CooldownGroupTag = AbilitySpec.AbilityDefinition->CooldownGroupTag;
			Summary.ResourceCost = AbilitySpec.AbilityDefinition->ResourceCost;
			Summary.bCanBufferComboInput = AbilitySpec.AbilityDefinition->Combo.bBufferComboInput;
			Summary.NextComboAbilityTag = AbilitySpec.AbilityDefinition->Combo.NextComboAbilityTag;
		}

		Summary.CooldownRemainingSeconds = FMath::Max(0.0f, AbilitySpec.CooldownEndTime - CurrentWorldTime);
		if (Summary.CooldownGroupTag.IsValid())
		{
			Summary.CooldownGroupRemainingSeconds = GetCooldownGroupRemainingInternal(Summary.CooldownGroupTag);
		}

		if (AbilitySpec.ActivationStartTime > 0.0f)
		{
			Summary.ActivationAgeSeconds = FMath::Max(0.0f, CurrentWorldTime - AbilitySpec.ActivationStartTime);
		}

		if (AbilitySpec.ScheduledDeliveryTime > 0.0f)
		{
			Summary.DeliveryRemainingSeconds = FMath::Max(0.0f, AbilitySpec.ScheduledDeliveryTime - CurrentWorldTime);
		}

		if (AbilitySpec.RecoveryEndTime > 0.0f)
		{
			Summary.RecoveryRemainingSeconds = FMath::Max(0.0f, AbilitySpec.RecoveryEndTime - CurrentWorldTime);
		}

		if (AbilitySpec.ComboWindowEndTime > AbilitySpec.ComboWindowStartTime)
		{
			if (CurrentWorldTime < AbilitySpec.ComboWindowStartTime)
			{
				Summary.ComboWindowOpensInSeconds = FMath::Max(0.0f, AbilitySpec.ComboWindowStartTime - CurrentWorldTime);
			}
			else if (CurrentWorldTime < AbilitySpec.ComboWindowEndTime)
			{
				Summary.bComboWindowOpen = true;
				Summary.ComboWindowRemainingSeconds = FMath::Max(0.0f, AbilitySpec.ComboWindowEndTime - CurrentWorldTime);
			}
		}

		Summary.QueuedComboAbilityTag = AbilitySpec.QueuedComboAbilityTag;
		Summaries.Add(MoveTemp(Summary));
	}

	return Summaries;
}


TArray<FMABSCooldownGroupDebugSummary> UMABSAbilityComponent::GetCooldownGroupDebugSummaries() const
{
	TArray<FMABSCooldownGroupDebugSummary> Summaries;

	const float CurrentWorldTime = GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : 0.0f;
	for (const FMABSCooldownGroupState& CooldownGroupState : CooldownGroupStates)
	{
		const float RemainingSeconds = FMath::Max(0.0f, CooldownGroupState.CooldownEndTime - CurrentWorldTime);
		if (RemainingSeconds <= 0.0f)
		{
			continue;
		}

		FMABSCooldownGroupDebugSummary Summary;
		Summary.CooldownGroupTag = CooldownGroupState.CooldownGroupTag;
		Summary.RemainingSeconds = RemainingSeconds;
		Summaries.Add(Summary);
	}

	Summaries.Sort([](const FMABSCooldownGroupDebugSummary& Left, const FMABSCooldownGroupDebugSummary& Right)
	{
		return Left.RemainingSeconds > Right.RemainingSeconds;
	});

	return Summaries;
}


FMABSComboDebugSummary UMABSAbilityComponent::GetComboDebugSummary() const
{
	FMABSComboDebugSummary Summary;

	const float CurrentWorldTime = GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : 0.0f;
	const FMABSAbilitySpec* BestSpec = nullptr;
	int32 BestPriority = INDEX_NONE;

	for (const FMABSAbilitySpec& AbilitySpec : GrantedAbilities)
	{
		const bool bHasComboData = AbilitySpec.AbilityDefinition != nullptr && AbilitySpec.AbilityDefinition->Combo.IsEnabled();
		const bool bHasQueuedFollowup = AbilitySpec.QueuedComboAbilityTag.IsValid();
		const bool bHasWindow = AbilitySpec.ComboWindowEndTime > AbilitySpec.ComboWindowStartTime;
		const bool bWindowOpen = bHasWindow
			&& CurrentWorldTime >= AbilitySpec.ComboWindowStartTime
			&& CurrentWorldTime < AbilitySpec.ComboWindowEndTime;
		const bool bWindowPending = bHasWindow && CurrentWorldTime < AbilitySpec.ComboWindowStartTime;

		if (!bHasComboData && !bHasQueuedFollowup && !bHasWindow)
		{
			continue;
		}

		int32 Priority = 1;
		if (bWindowPending)
		{
			Priority = 2;
		}
		if (bHasQueuedFollowup)
		{
			Priority = 3;
		}
		if (bWindowOpen)
		{
			Priority = 4;
		}

		if (Priority > BestPriority)
		{
			BestPriority = Priority;
			BestSpec = &AbilitySpec;
		}
	}

	if (BestSpec == nullptr)
	{
		return Summary;
	}

	Summary.bHasActiveComboState = true;
	Summary.SourceAbilityTag = BestSpec->AbilityTag;
	Summary.SourceAbilityDisplayName = GetAbilityDebugDisplayName(BestSpec->AbilityDefinition);
	Summary.QueuedComboAbilityTag = BestSpec->QueuedComboAbilityTag;

	if (BestSpec->AbilityDefinition != nullptr)
	{
		Summary.NextComboAbilityTag = BestSpec->AbilityDefinition->Combo.NextComboAbilityTag;
		Summary.bBufferComboInput = BestSpec->AbilityDefinition->Combo.bBufferComboInput;
	}

	if (BestSpec->ComboWindowEndTime > BestSpec->ComboWindowStartTime)
	{
		if (CurrentWorldTime < BestSpec->ComboWindowStartTime)
		{
			Summary.ComboWindowOpensInSeconds = FMath::Max(0.0f, BestSpec->ComboWindowStartTime - CurrentWorldTime);
		}
		else if (CurrentWorldTime < BestSpec->ComboWindowEndTime)
		{
			Summary.bComboWindowOpen = true;
			Summary.ComboWindowRemainingSeconds = FMath::Max(0.0f, BestSpec->ComboWindowEndTime - CurrentWorldTime);
		}
	}

	return Summary;
}


FMABSTargetTraceDebugInfo UMABSAbilityComponent::GetLatestTargetTraceDebugInfo() const
{
	return LatestTargetTraceDebugInfo;
}


TArray<FMABSActivePeriodicEffect> UMABSAbilityComponent::GetActivePeriodicEffects() const
{
	return ActivePeriodicEffects;
}


TArray<FMABSPeriodicEffectDebugSummary> UMABSAbilityComponent::GetPeriodicEffectDebugSummaries() const
{
	TArray<FMABSPeriodicEffectDebugSummary> Summaries;
	Summaries.Reserve(ActivePeriodicEffects.Num());

	const UWorld* const World = GetWorld();
	const float CurrentWorldTime = World != nullptr ? World->GetTimeSeconds() : 0.0f;
	for (const FMABSActivePeriodicEffect& ActiveEffect : ActivePeriodicEffects)
	{
		FMABSPeriodicEffectDebugSummary Summary;
		Summary.RuntimeId = ActiveEffect.RuntimeId;
		Summary.AbilityTag = ActiveEffect.AbilityTag;
		Summary.AbilityDisplayName = GetAbilityDebugDisplayName(ActiveEffect.AbilityDefinition);
		Summary.EffectType = ActiveEffect.EffectType;
		Summary.SourceActorName = GetNameSafe(ActiveEffect.SourceActor);
		Summary.TargetActorName = GetNameSafe(ActiveEffect.TargetActor);
		Summary.TickMagnitude = ActiveEffect.TickMagnitude;
		Summary.TickInterval = ActiveEffect.TickInterval;
		Summary.TimeRemainingSeconds = FMath::Max(0.0f, ActiveEffect.ExpirationWorldTime - CurrentWorldTime);

		bool bUsedAuthoritativeTimer = false;
		if (World != nullptr)
		{
			if (const FMABSPeriodicEffectRuntime* const Runtime = ActivePeriodicEffectRuntimes.Find(ActiveEffect.RuntimeId))
			{
				const float TickTimerRemaining = World->GetTimerManager().GetTimerRemaining(Runtime->TickTimerHandle);
				if (TickTimerRemaining >= 0.0f)
				{
					Summary.TimeUntilNextTickSeconds = FMath::Min(TickTimerRemaining, Summary.TimeRemainingSeconds);
					bUsedAuthoritativeTimer = true;
				}
			}
		}

		if (!bUsedAuthoritativeTimer && ActiveEffect.TickInterval > 0.0f)
		{
			const float ElapsedSinceApplication = FMath::Max(0.0f, CurrentWorldTime - ActiveEffect.AppliedWorldTime);
			const int32 CompletedTickCount = FMath::FloorToInt(ElapsedSinceApplication / ActiveEffect.TickInterval);
			const float NextTickWorldTime = ActiveEffect.AppliedWorldTime + ((CompletedTickCount + 1) * ActiveEffect.TickInterval);
			Summary.TimeUntilNextTickSeconds = FMath::Clamp(
				NextTickWorldTime - CurrentWorldTime,
				0.0f,
				Summary.TimeRemainingSeconds);
		}

		Summaries.Add(MoveTemp(Summary));
	}

	Summaries.Sort([](const FMABSPeriodicEffectDebugSummary& Left, const FMABSPeriodicEffectDebugSummary& Right)
	{
		return Left.RuntimeId < Right.RuntimeId;
	});

	return Summaries;
}


void UMABSAbilityComponent::ClientReceiveAbilityDebugEvent_Implementation(const FMABSAbilityDebugEvent& DebugEvent)
{
	RecordDebugEvent(DebugEvent);
}


void UMABSAbilityComponent::ClientReceiveTargetTraceDebugInfo_Implementation(const FMABSTargetTraceDebugInfo& DebugInfo)
{
	RecordLatestTargetTraceDebugInfo(DebugInfo);
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
	DebugEvent.Category = GetDebugEventCategory(EventName);
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
	const UEnum* CategoryEnum = StaticEnum<EMABSDebugEventCategory>();
	const FString CategoryName = CategoryEnum != nullptr
		? CategoryEnum->GetNameStringByValue(static_cast<int64>(DebugEvent.Category))
		: TEXT("General");
	const FString RuntimeStateName = RuntimeStateEnum != nullptr
		? RuntimeStateEnum->GetNameStringByValue(static_cast<int64>(DebugEvent.RuntimeState))
		: TEXT("Unknown");
	const FString ResultName = ResultEnum != nullptr
		? ResultEnum->GetNameStringByValue(static_cast<int64>(DebugEvent.ActivationResult))
		: TEXT("Unknown");

	UE_LOG(
		LogMABSAbilitySystem,
		Log,
		TEXT("[%s/%s] Owner=%s AbilityTag=%s Handle=%d State=%s Result=%s Message=%s"),
		*CategoryName,
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
