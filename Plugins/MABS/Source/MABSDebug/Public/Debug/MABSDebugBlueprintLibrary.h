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
	static FString FormatTargetTraceDebugInfo(const FMABSTargetTraceDebugInfo& DebugInfo);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FLinearColor GetAbilityDebugEventColor(const FMABSAbilityDebugEvent& DebugEvent);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	static FLinearColor GetTargetTraceDebugColor(const FMABSTargetTraceDebugInfo& DebugInfo);
};
