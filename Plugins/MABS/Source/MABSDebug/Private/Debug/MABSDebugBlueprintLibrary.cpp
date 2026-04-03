// Copyright Epic Games, Inc. All Rights Reserved.

#include "Debug/MABSDebugBlueprintLibrary.h"
#include "Data/MABSAbilityDefinition.h"

namespace
{
	template <typename TEnum>
	FString GetEnumValueName(const TEnum Value)
	{
		if (const UEnum* const Enum = StaticEnum<TEnum>())
		{
			return Enum->GetNameStringByValue(static_cast<int64>(Value));
		}

		return TEXT("Unknown");
	}

	FString TrimDebugString(const FString& Text, const int32 MaxCharacters)
	{
		if (Text.Len() <= MaxCharacters)
		{
			return Text;
		}

		return Text.Left(FMath::Max(0, MaxCharacters - 3)) + TEXT("...");
	}

	FString GetAbilityLabelFromSummary(const FText& DisplayName, const FGameplayTag& AbilityTag)
	{
		if (!DisplayName.IsEmpty())
		{
			return DisplayName.ToString();
		}

		return AbilityTag.IsValid() ? AbilityTag.ToString() : TEXT("UnknownAbility");
	}

	FString GetAbilityLabelFromDefinition(const UMABSAbilityDefinition* const AbilityDefinition, const FGameplayTag& AbilityTag)
	{
		if (AbilityDefinition != nullptr && !AbilityDefinition->DisplayName.IsEmpty())
		{
			return AbilityDefinition->DisplayName.ToString();
		}

		return AbilityTag.IsValid() ? AbilityTag.ToString() : TEXT("UnknownAbility");
	}

	FString FormatDurationText(const float Seconds, const TCHAR* const ReadyLabel)
	{
		return Seconds > 0.0f
			? FString::Printf(TEXT("%.2fs"), Seconds)
			: ReadyLabel;
	}
}

FString UMABSDebugBlueprintLibrary::FormatAbilityDebugEvent(const FMABSAbilityDebugEvent& DebugEvent)
{
	return FString::Printf(
		TEXT("[%.2fs][%s/%s] Owner=%s AbilityTag=%s Handle=%d State=%s Result=%s Message=%s"),
		DebugEvent.WorldTimeSeconds,
		*GetDebugEventCategoryLabel(DebugEvent.Category),
		*DebugEvent.EventName.ToString(),
		*DebugEvent.OwnerName,
		*DebugEvent.AbilityTag.ToString(),
		DebugEvent.AbilityHandle.Value,
		*GetEnumValueName(DebugEvent.RuntimeState),
		*GetEnumValueName(DebugEvent.ActivationResult),
		*DebugEvent.Message);
}

FString UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(const FMABSAbilityDebugEvent& DebugEvent)
{
	const FString AbilityLabel = DebugEvent.AbilityTag.IsValid() ? DebugEvent.AbilityTag.ToString() : TEXT("NoAbility");
	return FString::Printf(
		TEXT("%.2fs %s/%s %s | %s"),
		DebugEvent.WorldTimeSeconds,
		*GetDebugEventCategoryLabel(DebugEvent.Category),
		*DebugEvent.EventName.ToString(),
		*TrimDebugString(AbilityLabel, 28),
		*TrimDebugString(DebugEvent.Message, 92));
}

FString UMABSDebugBlueprintLibrary::FormatTargetTraceDebugInfo(const FMABSTargetTraceDebugInfo& DebugInfo)
{
	if (!DebugInfo.bHasTraceData)
	{
		return TEXT("No recent target or delivery trace.");
	}

	const FString TraceLabel = DebugInfo.TraceLabel.IsEmpty() ? TEXT("Trace") : DebugInfo.TraceLabel;
	return FString::Printf(
		TEXT("%s %s | Ability=%s Hit=%s Accepted=%s Actor=%s Dist=%.0f | %s"),
		*GetEnumValueName(DebugInfo.DeliveryMode),
		*GetEnumValueName(DebugInfo.TraceMode),
		*DebugInfo.AbilityTag.ToString(),
		DebugInfo.bHit ? TEXT("true") : TEXT("false"),
		DebugInfo.bAcceptedTarget ? TEXT("true") : TEXT("false"),
		*TrimDebugString(DebugInfo.HitActorName, 24),
		DebugInfo.HitDistance,
		*TrimDebugString(TraceLabel + TEXT(": ") + DebugInfo.ResultMessage, 96));
}

FString UMABSDebugBlueprintLibrary::FormatGrantedAbilityDebugSummary(const FMABSGrantedAbilityDebugSummary& AbilitySummary)
{
	FString Line = FString::Printf(
		TEXT("%s | State=%s Result=%s CD=%s Delivery=%s"),
		*GetAbilityLabelFromSummary(AbilitySummary.DisplayName, AbilitySummary.AbilityTag),
		*GetEnumValueName(AbilitySummary.RuntimeState),
		*GetEnumValueName(AbilitySummary.LastActivationResult),
		*FormatDurationText(AbilitySummary.CooldownRemainingSeconds, TEXT("Ready")),
		*GetEnumValueName(AbilitySummary.DeliveryMode));

	if (AbilitySummary.CooldownGroupTag.IsValid())
	{
		Line += FString::Printf(
			TEXT(" Group=%s(%.2fs)"),
			*AbilitySummary.CooldownGroupTag.ToString(),
			AbilitySummary.CooldownGroupRemainingSeconds);
	}

	if (AbilitySummary.DeliveryRemainingSeconds > 0.0f)
	{
		Line += FString::Printf(TEXT(" DeliveryIn=%.2fs"), AbilitySummary.DeliveryRemainingSeconds);
	}

	if (AbilitySummary.RecoveryRemainingSeconds > 0.0f)
	{
		Line += FString::Printf(TEXT(" Recovery=%.2fs"), AbilitySummary.RecoveryRemainingSeconds);
	}

	if (AbilitySummary.bComboWindowOpen)
	{
		Line += FString::Printf(TEXT(" ComboOpen=%.2fs"), AbilitySummary.ComboWindowRemainingSeconds);
	}
	else if (AbilitySummary.ComboWindowOpensInSeconds > 0.0f)
	{
		Line += FString::Printf(TEXT(" ComboIn=%.2fs"), AbilitySummary.ComboWindowOpensInSeconds);
	}

	if (AbilitySummary.QueuedComboAbilityTag.IsValid())
	{
		Line += TEXT(" Queued=") + AbilitySummary.QueuedComboAbilityTag.ToString();
	}

	if (AbilitySummary.bIsBlocked)
	{
		Line += TEXT(" BLOCKED");
	}

	return TrimDebugString(Line, 168);
}

FString UMABSDebugBlueprintLibrary::FormatCooldownGroupDebugSummary(const FMABSCooldownGroupDebugSummary& CooldownSummary)
{
	return FString::Printf(
		TEXT("%s | Remaining=%.2fs"),
		*CooldownSummary.CooldownGroupTag.ToString(),
		CooldownSummary.RemainingSeconds);
}

FString UMABSDebugBlueprintLibrary::FormatPeriodicEffectDebugSummary(const FMABSPeriodicEffectDebugSummary& EffectSummary)
{
	return FString::Printf(
		TEXT("%s %s -> %s | Mag=%.2f Tick=%.2fs Next=%.2fs Rem=%.2fs"),
		*GetEnumValueName(EffectSummary.EffectType),
		*GetAbilityLabelFromSummary(EffectSummary.AbilityDisplayName, EffectSummary.AbilityTag),
		*TrimDebugString(EffectSummary.TargetActorName, 24),
		EffectSummary.TickMagnitude,
		EffectSummary.TickInterval,
		EffectSummary.TimeUntilNextTickSeconds,
		EffectSummary.TimeRemainingSeconds);
}

FString UMABSDebugBlueprintLibrary::FormatComboDebugSummary(const FMABSComboDebugSummary& ComboSummary)
{
	if (!ComboSummary.bHasActiveComboState)
	{
		return TEXT("No active combo window or queued combo follow-up.");
	}

	FString Line = FString::Printf(
		TEXT("%s | Next=%s Buffer=%s"),
		*GetAbilityLabelFromSummary(ComboSummary.SourceAbilityDisplayName, ComboSummary.SourceAbilityTag),
		ComboSummary.NextComboAbilityTag.IsValid() ? *ComboSummary.NextComboAbilityTag.ToString() : TEXT("None"),
		ComboSummary.bBufferComboInput ? TEXT("true") : TEXT("false"));

	if (ComboSummary.bComboWindowOpen)
	{
		Line += FString::Printf(TEXT(" WindowOpen=%.2fs"), ComboSummary.ComboWindowRemainingSeconds);
	}
	else if (ComboSummary.ComboWindowOpensInSeconds > 0.0f)
	{
		Line += FString::Printf(TEXT(" WindowIn=%.2fs"), ComboSummary.ComboWindowOpensInSeconds);
	}

	if (ComboSummary.QueuedComboAbilityTag.IsValid())
	{
		Line += TEXT(" Queued=") + ComboSummary.QueuedComboAbilityTag.ToString();
	}

	return TrimDebugString(Line, 168);
}

FString UMABSDebugBlueprintLibrary::FormatAbilitySpecRuntimeSummary(const FMABSAbilitySpec& AbilitySpec, const float CurrentWorldTimeSeconds)
{
	const float CooldownRemaining = FMath::Max(0.0f, AbilitySpec.CooldownEndTime - CurrentWorldTimeSeconds);
	const float DeliveryRemaining = AbilitySpec.ScheduledDeliveryTime > 0.0f
		? FMath::Max(0.0f, AbilitySpec.ScheduledDeliveryTime - CurrentWorldTimeSeconds)
		: 0.0f;
	const float RecoveryRemaining = AbilitySpec.RecoveryEndTime > 0.0f
		? FMath::Max(0.0f, AbilitySpec.RecoveryEndTime - CurrentWorldTimeSeconds)
		: 0.0f;

	FString TimingText;
	if (AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Startup && AbilitySpec.ActivationStartTime > 0.0f)
	{
		TimingText = FString::Printf(
			TEXT(" Startup=%.2fs DeliveryIn=%.2fs"),
			CurrentWorldTimeSeconds - AbilitySpec.ActivationStartTime,
			DeliveryRemaining);
	}
	else if (AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Recovery)
	{
		TimingText = FString::Printf(TEXT(" Recovery=%.2fs"), RecoveryRemaining);
	}
	else if (AbilitySpec.RuntimeState == EMABSAbilityRuntimeState::Active && AbilitySpec.ActivationStartTime > 0.0f)
	{
		TimingText = FString::Printf(TEXT(" ActiveFor=%.2fs"), CurrentWorldTimeSeconds - AbilitySpec.ActivationStartTime);
	}

	FString ComboText;
	if (AbilitySpec.ComboWindowEndTime > AbilitySpec.ComboWindowStartTime)
	{
		if (CurrentWorldTimeSeconds < AbilitySpec.ComboWindowStartTime)
		{
			ComboText += FString::Printf(
				TEXT(" ComboIn=%.2fs"),
				FMath::Max(0.0f, AbilitySpec.ComboWindowStartTime - CurrentWorldTimeSeconds));
		}
		else
		{
			ComboText += FString::Printf(
				TEXT(" ComboOpen=%.2fs"),
				FMath::Max(0.0f, AbilitySpec.ComboWindowEndTime - CurrentWorldTimeSeconds));
		}
	}

	if (AbilitySpec.QueuedComboAbilityTag.IsValid())
	{
		ComboText += TEXT(" QueuedCombo=") + AbilitySpec.QueuedComboAbilityTag.ToString();
	}

	return FString::Printf(
		TEXT("[Ability] %s | State=%s Result=%s Cooldown=%s%s%s"),
		*GetAbilityLabelFromDefinition(AbilitySpec.AbilityDefinition, AbilitySpec.AbilityTag),
		*GetEnumValueName(AbilitySpec.RuntimeState),
		*GetEnumValueName(AbilitySpec.LastActivationResult),
		*FormatDurationText(CooldownRemaining, TEXT("Ready")),
		*TimingText,
		*ComboText);
}

FLinearColor UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(const FMABSAbilityDebugEvent& DebugEvent)
{
	switch (DebugEvent.ActivationResult)
	{
	case EMABSAbilityActivationResult::Success:
	case EMABSAbilityActivationResult::RequestSentToServer:
		return FLinearColor(0.2f, 0.85f, 0.35f, 1.0f);

	case EMABSAbilityActivationResult::ComboQueued:
		return FLinearColor(0.25f, 0.65f, 0.95f, 1.0f);

	case EMABSAbilityActivationResult::None:
		return GetDebugEventCategoryColor(DebugEvent.Category);

	default:
		return FLinearColor(0.95f, 0.3f, 0.25f, 1.0f);
	}
}

FLinearColor UMABSDebugBlueprintLibrary::GetDebugEventCategoryColor(const EMABSDebugEventCategory Category)
{
	switch (Category)
	{
	case EMABSDebugEventCategory::Activation:
		return FLinearColor(0.78f, 0.86f, 0.97f, 1.0f);

	case EMABSDebugEventCategory::Targeting:
		return FLinearColor(0.35f, 0.9f, 0.92f, 1.0f);

	case EMABSDebugEventCategory::Delivery:
		return FLinearColor(0.96f, 0.74f, 0.34f, 1.0f);

	case EMABSDebugEventCategory::CostCooldown:
		return FLinearColor(0.95f, 0.85f, 0.25f, 1.0f);

	case EMABSDebugEventCategory::Combo:
		return FLinearColor(0.38f, 0.75f, 0.98f, 1.0f);

	case EMABSDebugEventCategory::Periodic:
		return FLinearColor(0.45f, 0.9f, 0.48f, 1.0f);

	case EMABSDebugEventCategory::Presentation:
		return FLinearColor(0.93f, 0.55f, 0.77f, 1.0f);

	case EMABSDebugEventCategory::General:
	default:
		return FLinearColor(0.82f, 0.85f, 0.9f, 1.0f);
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
	FMABSGrantedAbilityDebugSummary Summary;
	Summary.RuntimeState = AbilitySpec.RuntimeState;
	Summary.LastActivationResult = AbilitySpec.LastActivationResult;
	Summary.CooldownRemainingSeconds = FMath::Max(0.0f, AbilitySpec.CooldownEndTime - CurrentWorldTimeSeconds);
	return GetGrantedAbilityDebugSummaryColor(Summary);
}

FLinearColor UMABSDebugBlueprintLibrary::GetGrantedAbilityDebugSummaryColor(const FMABSGrantedAbilityDebugSummary& AbilitySummary)
{
	switch (AbilitySummary.RuntimeState)
	{
	case EMABSAbilityRuntimeState::Startup:
		return FLinearColor(0.25f, 0.65f, 0.95f, 1.0f);

	case EMABSAbilityRuntimeState::Active:
		return FLinearColor(0.2f, 0.85f, 0.35f, 1.0f);

	case EMABSAbilityRuntimeState::Recovery:
		return FLinearColor(0.95f, 0.6f, 0.2f, 1.0f);

	case EMABSAbilityRuntimeState::Blocked:
		return FLinearColor(0.95f, 0.3f, 0.25f, 1.0f);

	default:
		break;
	}

	if (AbilitySummary.CooldownRemainingSeconds > 0.0f)
	{
		return FLinearColor(0.95f, 0.85f, 0.25f, 1.0f);
	}

	switch (AbilitySummary.LastActivationResult)
	{
	case EMABSAbilityActivationResult::Success:
		return FLinearColor(0.2f, 0.85f, 0.35f, 1.0f);

	case EMABSAbilityActivationResult::ComboQueued:
		return FLinearColor(0.25f, 0.65f, 0.95f, 1.0f);

	case EMABSAbilityActivationResult::OnCooldown:
	case EMABSAbilityActivationResult::ComboRejected:
	case EMABSAbilityActivationResult::InsufficientResources:
	case EMABSAbilityActivationResult::EffectApplicationFailed:
	case EMABSAbilityActivationResult::TargetResolutionFailed:
	case EMABSAbilityActivationResult::DeliveryFailed:
		return FLinearColor(0.95f, 0.3f, 0.25f, 1.0f);

	default:
		return FLinearColor(0.8f, 0.82f, 0.88f, 1.0f);
	}
}

FString UMABSDebugBlueprintLibrary::GetDebugEventCategoryLabel(const EMABSDebugEventCategory Category)
{
	switch (Category)
	{
	case EMABSDebugEventCategory::Activation:
		return TEXT("Activation");

	case EMABSDebugEventCategory::Targeting:
		return TEXT("Targeting");

	case EMABSDebugEventCategory::Delivery:
		return TEXT("Delivery");

	case EMABSDebugEventCategory::CostCooldown:
		return TEXT("CostCooldown");

	case EMABSDebugEventCategory::Combo:
		return TEXT("Combo");

	case EMABSDebugEventCategory::Periodic:
		return TEXT("Periodic");

	case EMABSDebugEventCategory::Presentation:
		return TEXT("Presentation");

	case EMABSDebugEventCategory::General:
	default:
		return TEXT("General");
	}
}
