#include "Components/MABSAbilityRuntime_Internal.h"

using namespace MABSAbilityRuntimeInternal;

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
