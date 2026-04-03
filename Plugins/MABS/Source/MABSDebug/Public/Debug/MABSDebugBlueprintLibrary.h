// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Debug/MABSAbilityDebugTypes.h"
#include "MABSDebugBlueprintLibrary.generated.h"

UCLASS()
class MABSDEBUG_API UMABSDebugBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FString FormatAbilityDebugEvent(const FMABSAbilityDebugEvent& DebugEvent);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FString FormatCompactAbilityDebugEvent(const FMABSAbilityDebugEvent& DebugEvent);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FString FormatTargetTraceDebugInfo(const FMABSTargetTraceDebugInfo& DebugInfo);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FString FormatGrantedAbilityDebugSummary(const FMABSGrantedAbilityDebugSummary& AbilitySummary);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FString FormatCooldownGroupDebugSummary(const FMABSCooldownGroupDebugSummary& CooldownSummary);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FString FormatPeriodicEffectDebugSummary(const FMABSPeriodicEffectDebugSummary& EffectSummary);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FString FormatComboDebugSummary(const FMABSComboDebugSummary& ComboSummary);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FString FormatAbilitySpecRuntimeSummary(const FMABSAbilitySpec& AbilitySpec, float CurrentWorldTimeSeconds);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FLinearColor GetAbilityDebugEventColor(const FMABSAbilityDebugEvent& DebugEvent);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FLinearColor GetDebugEventCategoryColor(EMABSDebugEventCategory Category);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FLinearColor GetTargetTraceDebugColor(const FMABSTargetTraceDebugInfo& DebugInfo);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FLinearColor GetAbilitySpecRuntimeColor(const FMABSAbilitySpec& AbilitySpec, float CurrentWorldTimeSeconds);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FLinearColor GetGrantedAbilityDebugSummaryColor(const FMABSGrantedAbilityDebugSummary& AbilitySummary);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FString GetDebugEventCategoryLabel(EMABSDebugEventCategory Category);
};
