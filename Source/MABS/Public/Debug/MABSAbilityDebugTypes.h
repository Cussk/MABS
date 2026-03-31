// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Types/MABSAbilityTypes.h"
#include "MABSAbilityDebugTypes.generated.h"

USTRUCT(BlueprintType)
struct MABS_API FMABSAbilityDebugEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FName EventName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FGameplayTag AbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FMABSAbilityHandle AbilityHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	EMABSAbilityRuntimeState RuntimeState = EMABSAbilityRuntimeState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	EMABSAbilityActivationResult ActivationResult = EMABSAbilityActivationResult::InvalidRequest;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FString OwnerName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float WorldTimeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FString Message;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMABSAbilityDebugEventSignature, FMABSAbilityDebugEvent, DebugEvent);
