#include "Delivery/MABSProjectileDeliveryHandler.h"

#include "Actors/MABSProjectileBase.h"
#include "Components/MABSAbilityComponent.h"
#include "Components/MABSAbilityRuntime_Common.h"
#include "Components/MABSAbilityRuntime_EventNames.h"
#include "Data/MABSAbilityDefinition.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

using namespace MABSAbilityRuntimeInternal;

FMABSDeliveryExecutionResult UMABSProjectileDeliveryHandler::ExecuteDelivery(const FMABSDeliveryExecutionContext& Context) const
{
	FMABSDeliveryExecutionResult Result;
	UMABSAbilityComponent* const AbilityComponent = Context.AbilityComponent;
	const UMABSAbilityDefinition* const AbilityDefinition = Context.AbilityDefinition;
	UWorld* const World = AbilityComponent != nullptr ? AbilityComponent->GetWorld() : nullptr;
	if (AbilityComponent == nullptr || AbilityDefinition == nullptr || World == nullptr)
	{
		Result.FailureResult = EMABSAbilityActivationResult::DeliveryFailed;
		Result.FailureEventName = MABSAbilityComponentEventNames::DeliveryFailed;
		Result.DebugMessage = TEXT("Projectile delivery failed because the world, ability component, or ability definition was unavailable.");
		return Result;
	}

	FTransform SpawnTransform = FTransform::Identity;
	FString SpawnTransformMessage;
	if (!AbilityComponent->GetProjectileSpawnTransform(
		*AbilityDefinition,
		SpawnTransform,
		SpawnTransformMessage,
		Context.bNotifyOwningClient))
	{
		const FMABSAbilityDebugEvent DebugEvent = AbilityComponent->EmitDebugEvent(
			MABSAbilityComponentEventNames::ProjectileSpawnFailed,
			Context.AbilitySpec.AbilityTag,
			Context.AbilitySpec.Handle,
			EMABSAbilityRuntimeState::Idle,
			EMABSAbilityActivationResult::DeliveryFailed,
			SpawnTransformMessage);
		if (Context.bNotifyOwningClient)
		{
			AbilityComponent->EmitDebugEventToOwningClient(DebugEvent);
		}

		Result.FailureResult = EMABSAbilityActivationResult::DeliveryFailed;
		Result.FailureEventName = MABSAbilityComponentEventNames::DeliveryFailed;
		Result.DebugMessage = SpawnTransformMessage;
		return Result;
	}

	AbilityComponent->TriggerDeliveryPresentation(Context.AbilitySpec, EMABSDeliveryMode::Projectile, Context.bNotifyOwningClient);

	AMABSProjectileBase* Projectile = World->SpawnActorDeferred<AMABSProjectileBase>(
		AbilityDefinition->ProjectileActorClass,
		SpawnTransform,
		AbilityComponent->GetOwner(),
		AbilityComponent->GetOwner() != nullptr ? AbilityComponent->GetOwner()->GetInstigator() : nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
	if (Projectile == nullptr)
	{
		const FString SpawnFailedMessage = FString::Printf(
			TEXT("Projectile delivery failed because class '%s' could not be spawned. %s"),
			*GetNameSafe(AbilityDefinition->ProjectileActorClass),
			*SpawnTransformMessage);

		const FMABSAbilityDebugEvent DebugEvent = AbilityComponent->EmitDebugEvent(
			MABSAbilityComponentEventNames::ProjectileSpawnFailed,
			Context.AbilitySpec.AbilityTag,
			Context.AbilitySpec.Handle,
			EMABSAbilityRuntimeState::Idle,
			EMABSAbilityActivationResult::DeliveryFailed,
			SpawnFailedMessage);
		if (Context.bNotifyOwningClient)
		{
			AbilityComponent->EmitDebugEventToOwningClient(DebugEvent);
		}

		Result.FailureResult = EMABSAbilityActivationResult::DeliveryFailed;
		Result.FailureEventName = MABSAbilityComponentEventNames::DeliveryFailed;
		Result.DebugMessage = SpawnFailedMessage;
		return Result;
	}

	Projectile->InitializeProjectile(
		AbilityComponent,
		AbilityComponent->GetOwner(),
		Context.AbilitySpec.AbilityDefinition,
		Context.AbilitySpec.AbilityTag,
		Context.AbilitySpec.Handle);
	UGameplayStatics::FinishSpawningActor(Projectile, SpawnTransform);

	Result.DebugMessage = FString::Printf(
		TEXT("Spawned projectile '%s' for ability '%s' at %s."),
		*GetNameSafe(Projectile),
		*GetAbilityLabel(AbilityDefinition),
		*FormatVectorForDebug(SpawnTransform.GetLocation()));

	const FMABSAbilityDebugEvent ProjectileSpawnedEvent = AbilityComponent->EmitDebugEvent(
		MABSAbilityComponentEventNames::ProjectileSpawned,
		Context.AbilitySpec.AbilityTag,
		Context.AbilitySpec.Handle,
		EMABSAbilityRuntimeState::Active,
		EMABSAbilityActivationResult::Success,
		Result.DebugMessage);
	if (Context.bNotifyOwningClient)
	{
		AbilityComponent->EmitDebugEventToOwningClient(ProjectileSpawnedEvent);
	}

	if (AbilityDefinition->DeliveryPresentation.ProjectileTravel.HasAnyPresentation())
	{
		const FMABSAbilityDebugEvent CueRoutedEvent = AbilityComponent->EmitDebugEvent(
			MABSAbilityComponentEventNames::PresentationCueRouted,
			Context.AbilitySpec.AbilityTag,
			Context.AbilitySpec.Handle,
			EMABSAbilityRuntimeState::Active,
			EMABSAbilityActivationResult::Success,
			FString::Printf(
				TEXT("Routed ProjectileTravel cue with policy %s through the replicated projectile '%s'. Assets: VFX=%s, SFX=%s."),
				*MABSAbilityRuntimeInternal::GetPresentationVisibilityPolicyLabel(AbilityDefinition->DeliveryPresentation.ProjectileTravel.VisibilityPolicy),
				*GetNameSafe(Projectile),
				*GetNameSafe(AbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelVFX.Get()),
				*GetNameSafe(AbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelSFX.Get())));
		if (Context.bNotifyOwningClient)
		{
			AbilityComponent->EmitDebugEventToOwningClient(CueRoutedEvent);
		}

		const FMABSAbilityDebugEvent TravelPresentationEvent = AbilityComponent->EmitDebugEvent(
			MABSAbilityComponentEventNames::ProjectileTravelPresentationTriggered,
			Context.AbilitySpec.AbilityTag,
			Context.AbilitySpec.Handle,
			EMABSAbilityRuntimeState::Active,
			EMABSAbilityActivationResult::Success,
			FString::Printf(
				TEXT("Hooked projectile travel cue for projectile '%s' with policy %s. Assets: VFX=%s, SFX=%s."),
				*GetNameSafe(Projectile),
				*MABSAbilityRuntimeInternal::GetPresentationVisibilityPolicyLabel(AbilityDefinition->DeliveryPresentation.ProjectileTravel.VisibilityPolicy),
				*GetNameSafe(AbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelVFX.Get()),
				*GetNameSafe(AbilityDefinition->DeliveryPresentation.ProjectileTravel.TravelSFX.Get())));
		if (Context.bNotifyOwningClient)
		{
			AbilityComponent->EmitDebugEventToOwningClient(TravelPresentationEvent);
		}
	}

	Result.bDeliverySucceeded = true;
	Result.bApplyStandardEffects = false;
	Result.bTriggerStandardImpactPresentation = false;
	return Result;
}
