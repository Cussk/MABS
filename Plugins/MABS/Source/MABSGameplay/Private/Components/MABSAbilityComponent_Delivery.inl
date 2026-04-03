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
