#include "Delivery/MABSHitTraceDeliveryHandler.h"

#include "Components/MABSAbilityComponent.h"
#include "Components/MABSAbilityRuntime_EventNames.h"
#include "Data/MABSAbilityDefinition.h"

FMABSDeliveryExecutionResult UMABSHitTraceDeliveryHandler::ExecuteDelivery(const FMABSDeliveryExecutionContext& Context) const
{
	FMABSDeliveryExecutionResult Result;
	UMABSAbilityComponent* const AbilityComponent = Context.AbilityComponent;
	if (AbilityComponent == nullptr)
	{
		Result.FailureResult = EMABSAbilityActivationResult::DeliveryFailed;
		Result.FailureEventName = MABSAbilityComponentEventNames::DeliveryFailed;
		Result.DebugMessage = TEXT("HitTrace delivery failed because the ability component was invalid.");
		return Result;
	}

	FString DeliveryDebugMessage;
	const FMABSResolvedAbilityTarget ResolvedTarget = AbilityComponent->ExecuteHitTraceDelivery(
		Context.AbilitySpec,
		DeliveryDebugMessage,
		Context.bNotifyOwningClient);
	if (ResolvedTarget.TargetActor == nullptr)
	{
		Result.FailureResult = EMABSAbilityActivationResult::DeliveryFailed;
		Result.FailureEventName = MABSAbilityComponentEventNames::DeliveryFailed;
		Result.DebugMessage = DeliveryDebugMessage;
		return Result;
	}

	Result.DebugMessage = DeliveryDebugMessage;
	Result.PrimaryTargetActor = ResolvedTarget.TargetActor;
	if (ResolvedTarget.bHasImpactHitResult)
	{
		Result.SetPrimaryImpactFromHitResult(ResolvedTarget.ImpactHitResult);
	}

	FMABSAbilitySpec HandlerSpec = Context.AbilitySpec;
	HandlerSpec.RuntimeState = EMABSAbilityRuntimeState::Active;

	if (Context.AbilityDefinition != nullptr && Context.AbilityDefinition->AoE.IsValid())
	{
		TArray<AActor*> TargetActors;
		FString TargetResolutionMessage;
		if (!AbilityComponent->ResolveAoETargets(
			HandlerSpec,
			ResolvedTarget,
			TargetActors,
			TargetResolutionMessage,
			Context.bNotifyOwningClient))
		{
			Result.FailureResult = EMABSAbilityActivationResult::DeliveryFailed;
			Result.FailureEventName = MABSAbilityComponentEventNames::DeliveryFailed;
			Result.DebugMessage = TargetResolutionMessage;
			return Result;
		}

		for (AActor* const TargetActor : TargetActors)
		{
			Result.AddResolvedTarget(TargetActor);
		}
	}
	else
	{
		Result.AddResolvedTarget(ResolvedTarget.TargetActor);
	}

	Result.bDeliverySucceeded = true;
	ApplyResultModifier(Context, Result);
	return Result;
}
