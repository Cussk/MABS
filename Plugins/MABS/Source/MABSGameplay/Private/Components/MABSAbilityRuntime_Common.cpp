// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/MABSAbilityRuntime_Common.h"

#include "Data/MABSAbilityDefinition.h"
#include "Data/MABSAbilitySet.h"
#include "GameFramework/Actor.h"

namespace MABSAbilityRuntimeInternal
{
	FString DescribeActivationResult(const EMABSAbilityActivationResult ActivationResult)
	{
		switch (ActivationResult)
		{
		case EMABSAbilityActivationResult::InvalidAbility:
			return TEXT("Activation rejected because the requested ability was invalid.");

		case EMABSAbilityActivationResult::ComboRejected:
			return TEXT("Activation rejected because the requested combo follow-up was not valid.");

		case EMABSAbilityActivationResult::NotGranted:
			return TEXT("Activation rejected because the ability has not been granted.");

		case EMABSAbilityActivationResult::AlreadyActive:
			return TEXT("Activation rejected because the ability is already active.");

		case EMABSAbilityActivationResult::ComboQueued:
			return TEXT("Activation was accepted and queued as a combo follow-up.");

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

	FString GetRuntimeStateLabel(const EMABSAbilityRuntimeState RuntimeState)
	{
		switch (RuntimeState)
		{
		case EMABSAbilityRuntimeState::Startup:
			return TEXT("Startup");

		case EMABSAbilityRuntimeState::Active:
			return TEXT("Active");

		case EMABSAbilityRuntimeState::Recovery:
			return TEXT("Recovery");

		case EMABSAbilityRuntimeState::Blocked:
			return TEXT("Blocked");

		case EMABSAbilityRuntimeState::Idle:
			return TEXT("Idle");

		default:
			return TEXT("None");
		}
	}

	FString GetPresentationCuePhaseLabel(const EMABSPresentationCuePhase CuePhase)
	{
		switch (CuePhase)
		{
		case EMABSPresentationCuePhase::Delivery:
			return TEXT("Delivery");

		case EMABSPresentationCuePhase::Tracer:
			return TEXT("Tracer");

		case EMABSPresentationCuePhase::ProjectileTravel:
			return TEXT("ProjectileTravel");

		case EMABSPresentationCuePhase::Impact:
			return TEXT("Impact");

		default:
			return TEXT("Startup");
		}
	}

	FString GetPresentationVisibilityPolicyLabel(const EMABSPresentationCueVisibilityPolicy VisibilityPolicy)
	{
		switch (VisibilityPolicy)
		{
		case EMABSPresentationCueVisibilityPolicy::OwnerOnly:
			return TEXT("OwnerOnly");

		case EMABSPresentationCueVisibilityPolicy::LocalOnly:
			return TEXT("LocalOnly");

		default:
			return TEXT("RelevantClients");
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

	FString GetPeriodicEffectTypeLabel(const EMABSPeriodicEffectType EffectType)
	{
		switch (EffectType)
		{
		case EMABSPeriodicEffectType::HOT:
			return TEXT("HOT");

		default:
			return TEXT("DOT");
		}
	}

	FString GetAoEShapeLabel(const EMABSAoEShape Shape)
	{
		switch (Shape)
		{
		case EMABSAoEShape::Box:
			return TEXT("Box");

		case EMABSAoEShape::Capsule:
			return TEXT("Capsule");

		default:
			return TEXT("Sphere");
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

	FText GetAbilityDebugDisplayName(const UMABSAbilityDefinition* AbilityDefinition)
	{
		if (AbilityDefinition == nullptr)
		{
			return FText::FromString(TEXT("InvalidAbility"));
		}

		return AbilityDefinition->DisplayName.IsEmpty()
			? FText::FromString(GetNameSafe(AbilityDefinition))
			: AbilityDefinition->DisplayName;
	}

	FString GetAbilitySetLabel(const UMABSAbilitySet* AbilitySet)
	{
		return AbilitySet != nullptr ? GetNameSafe(AbilitySet) : TEXT("InvalidAbilitySet");
	}

	FString FormatVectorForDebug(const FVector& Vector)
	{
		return FString::Printf(TEXT("(X=%.0f Y=%.0f Z=%.0f)"), Vector.X, Vector.Y, Vector.Z);
	}

	FString DescribeActorList(const TArray<AActor*>& Actors)
	{
		TArray<FString> ActorLabels;
		ActorLabels.Reserve(Actors.Num());
		for (AActor* const Actor : Actors)
		{
			ActorLabels.Add(GetNameSafe(Actor));
		}

		return ActorLabels.IsEmpty() ? TEXT("None") : FString::Join(ActorLabels, TEXT(", "));
	}

	FString GetHitActorLabel(const AActor* Actor)
	{
		return Actor != nullptr ? GetNameSafe(Actor) : TEXT("World");
	}
}
