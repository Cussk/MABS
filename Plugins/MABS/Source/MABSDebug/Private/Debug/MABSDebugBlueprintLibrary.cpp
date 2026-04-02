// Copyright Epic Games, Inc. All Rights Reserved.

#include "Debug/MABSDebugBlueprintLibrary.h"
#include "Data/MABSAbilityDefinition.h"

FString UMABSDebugBlueprintLibrary::FormatAbilityDebugEvent(const FMABSAbilityDebugEvent& DebugEvent)
{
	const UEnum* RuntimeStateEnum = StaticEnum<EMABSAbilityRuntimeState>();
	const UEnum* ResultEnum = StaticEnum<EMABSAbilityActivationResult>();
	const FString RuntimeStateName = RuntimeStateEnum != nullptr
		? RuntimeStateEnum->GetNameStringByValue(static_cast<int64>(DebugEvent.RuntimeState))
		: TEXT("Unknown");
	const FString ResultName = ResultEnum != nullptr
		? ResultEnum->GetNameStringByValue(static_cast<int64>(DebugEvent.ActivationResult))
		: TEXT("Unknown");

	return FString::Printf(
		TEXT("[%s] Owner=%s AbilityTag=%s Handle=%d State=%s Result=%s Message=%s"),
		*DebugEvent.EventName.ToString(),
		*DebugEvent.OwnerName,
		*DebugEvent.AbilityTag.ToString(),
		DebugEvent.AbilityHandle.Value,
		*RuntimeStateName,
		*ResultName,
		*DebugEvent.Message);
}

FString UMABSDebugBlueprintLibrary::FormatTargetTraceDebugInfo(const FMABSTargetTraceDebugInfo& DebugInfo)
{
	if (!DebugInfo.bHasTraceData)
	{
		return TEXT("No recent actor-target trace.");
	}

	const UEnum* TraceModeEnum = StaticEnum<EMABSTargetTraceMode>();
	const FString TraceModeName = TraceModeEnum != nullptr
		? TraceModeEnum->GetNameStringByValue(static_cast<int64>(DebugInfo.TraceMode))
		: TEXT("Unknown");

	return FString::Printf(
		TEXT("[%s Trace] AbilityTag=%s Hit=%s Accepted=%s Actor=%s Component=%s Distance=%.0f Message=%s"),
		*TraceModeName,
		*DebugInfo.AbilityTag.ToString(),
		DebugInfo.bHit ? TEXT("true") : TEXT("false"),
		DebugInfo.bAcceptedTarget ? TEXT("true") : TEXT("false"),
		*DebugInfo.HitActorName,
		*DebugInfo.HitComponentName,
		DebugInfo.HitDistance,
		*DebugInfo.ResultMessage);
}

FString UMABSDebugBlueprintLibrary::FormatAbilitySpecRuntimeSummary(const FMABSAbilitySpec& AbilitySpec, const float CurrentWorldTimeSeconds)
{
	const UEnum* RuntimeStateEnum = StaticEnum<EMABSAbilityRuntimeState>();
	const UEnum* ResultEnum = StaticEnum<EMABSAbilityActivationResult>();
	const FString RuntimeStateName = RuntimeStateEnum != nullptr
		? RuntimeStateEnum->GetNameStringByValue(static_cast<int64>(AbilitySpec.RuntimeState))
		: TEXT("Unknown");
	const FString ResultName = ResultEnum != nullptr
		? ResultEnum->GetNameStringByValue(static_cast<int64>(AbilitySpec.LastActivationResult))
		: TEXT("Unknown");
	const float CooldownRemaining = FMath::Max(0.0f, AbilitySpec.CooldownEndTime - CurrentWorldTimeSeconds);
	const FString CooldownText = CooldownRemaining > 0.0f
		? FString::Printf(TEXT("%.2fs"), CooldownRemaining)
		: TEXT("Ready");

	FString CooldownGroupText;
	if (AbilitySpec.AbilityDefinition != nullptr && AbilitySpec.AbilityDefinition->CooldownGroupTag.IsValid())
	{
		CooldownGroupText = TEXT(" Group=") + AbilitySpec.AbilityDefinition->CooldownGroupTag.ToString();
	}

	return FString::Printf(
		TEXT("[Ability] Tag=%s State=%s Result=%s Cooldown=%s%s"),
		*AbilitySpec.AbilityTag.ToString(),
		*RuntimeStateName,
		*ResultName,
		*CooldownText,
		*CooldownGroupText);
}

FLinearColor UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(const FMABSAbilityDebugEvent& DebugEvent)
{
	switch (DebugEvent.ActivationResult)
	{
	case EMABSAbilityActivationResult::Success:
	case EMABSAbilityActivationResult::RequestSentToServer:
		return FLinearColor(0.2f, 0.85f, 0.35f, 1.0f);

	case EMABSAbilityActivationResult::None:
		return FLinearColor(0.95f, 0.85f, 0.25f, 1.0f);

	default:
		return FLinearColor(0.95f, 0.3f, 0.25f, 1.0f);
	}
}

FLinearColor UMABSDebugBlueprintLibrary::GetTargetTraceDebugColor(const FMABSTargetTraceDebugInfo& DebugInfo)
{
	if (!DebugInfo.bHasTraceData)
	{
		return FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);
	}

	if (DebugInfo.bAcceptedTarget)
	{
		return FLinearColor(0.2f, 0.85f, 0.35f, 1.0f);
	}

	if (DebugInfo.bHit)
	{
		return FLinearColor(0.95f, 0.3f, 0.25f, 1.0f);
	}

	return FLinearColor(0.95f, 0.85f, 0.25f, 1.0f);
}

FLinearColor UMABSDebugBlueprintLibrary::GetAbilitySpecRuntimeColor(const FMABSAbilitySpec& AbilitySpec, const float CurrentWorldTimeSeconds)
{
	const float CooldownRemaining = FMath::Max(0.0f, AbilitySpec.CooldownEndTime - CurrentWorldTimeSeconds);
	if (CooldownRemaining > 0.0f)
	{
		return FLinearColor(0.95f, 0.85f, 0.25f, 1.0f);
	}

	switch (AbilitySpec.LastActivationResult)
	{
	case EMABSAbilityActivationResult::Success:
		return FLinearColor(0.2f, 0.85f, 0.35f, 1.0f);

	case EMABSAbilityActivationResult::OnCooldown:
	case EMABSAbilityActivationResult::InsufficientResources:
	case EMABSAbilityActivationResult::EffectApplicationFailed:
	case EMABSAbilityActivationResult::TargetResolutionFailed:
		return FLinearColor(0.95f, 0.3f, 0.25f, 1.0f);

	default:
		return FLinearColor(0.8f, 0.82f, 0.88f, 1.0f);
	}
}
