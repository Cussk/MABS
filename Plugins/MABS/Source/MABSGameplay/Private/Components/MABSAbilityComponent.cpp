// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/MABSAbilityComponent.h"
#include "Data/MABSAbilityDefinition.h"
#include "Debug/MABSAbilitySystemLogs.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
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

void UMABSAbilityComponent::ServerTryActivateAbilityByTag_Implementation(const FGameplayTag AbilityTag)
{
	HandleTryActivateAbility(AbilityTag, true);
}

void UMABSAbilityComponent::ClientReceiveAbilityDebugEvent_Implementation(const FMABSAbilityDebugEvent& DebugEvent)
{
	RecordDebugEvent(DebugEvent);
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
	FString TargetDebugMessage;
	AActor* const TargetActor = ResolveAbilityTarget(AbilitySpec, TargetDebugMessage);
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

AActor* UMABSAbilityComponent::ResolveAbilityTarget(const FMABSAbilitySpec& AbilitySpec, FString& OutDebugMessage) const
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

			FVector TraceStart = Owner->GetActorLocation();
			FRotator TraceRotation = Owner->GetActorRotation();
			Owner->GetActorEyesViewPoint(TraceStart, TraceRotation);

			const FVector TraceEnd = TraceStart + (TraceRotation.Vector() * AbilityDefinition->TargetTraceDistance);
			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MABSAbilityTargetTrace), false);
			QueryParams.AddIgnoredActor(Owner);

			FHitResult HitResult;
			const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
			AActor* const HitActor = bHit ? HitResult.GetActor() : nullptr;
			if (HitActor == nullptr)
			{
				OutDebugMessage = FString::Printf(
					TEXT("Failed to resolve an actor target using target type '%s' within %.0f units."),
					*GetTargetTypeLabel(AbilityDefinition->TargetType),
					AbilityDefinition->TargetTraceDistance);
				return nullptr;
			}

			OutDebugMessage = FString::Printf(
				TEXT("Resolved target '%s' using target type '%s'."),
				*GetNameSafe(HitActor),
				*GetTargetTypeLabel(AbilityDefinition->TargetType));
			return HitActor;
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

bool UMABSAbilityComponent::CanMutateAbilityState() const
{
	const AActor* Owner = GetOwner();
	return Owner != nullptr && Owner->HasAuthority();
}

FMABSAbilityHandle UMABSAbilityComponent::MakeNextAbilityHandle()
{
	return FMABSAbilityHandle(NextAbilityHandleValue++);
}
