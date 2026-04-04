#include "Delivery/MABSDeliveryHandler.h"

FMABSDeliveryExecutionResult UMABSDeliveryHandler::ExecuteDelivery(const FMABSDeliveryExecutionContext& Context) const
{
	FMABSDeliveryExecutionResult Result;
	Result.FailureResult = EMABSAbilityActivationResult::DeliveryFailed;
	Result.DebugMessage = TEXT("Delivery execution failed because the handler does not implement ExecuteDelivery.");
	ApplyResultModifier(Context, Result);
	return Result;
}


void UMABSDeliveryHandler::ModifyDeliveryResult_Implementation(
	const FMABSDeliveryExecutionContext& Context,
	FMABSDeliveryExecutionResult& Result) const
{
}


void UMABSDeliveryHandler::ApplyResultModifier(
	const FMABSDeliveryExecutionContext& Context,
	FMABSDeliveryExecutionResult& Result) const
{
	ModifyDeliveryResult(Context, Result);
}
