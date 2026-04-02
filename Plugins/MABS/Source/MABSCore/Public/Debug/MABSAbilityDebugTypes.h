// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Types/MABSAbilityTypes.h"
#include "MABSAbilityDebugTypes.generated.h"

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSAbilityDebugEvent
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
	EMABSAbilityActivationResult ActivationResult = EMABSAbilityActivationResult::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FString OwnerName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float WorldTimeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FString Message;
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSTargetTraceDebugInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	bool bHasTraceData = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	bool bHit = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	bool bAcceptedTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FGameplayTag AbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FMABSAbilityHandle AbilityHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	EMABSDeliveryMode DeliveryMode = EMABSDeliveryMode::Direct;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	EMABSTargetTraceMode TraceMode = EMABSTargetTraceMode::Line;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FVector TraceStart = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FVector TraceEnd = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float TraceRadius = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float HitDistance = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float WorldTimeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FString TraceLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FString ViewPointDescription;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FString HitActorName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FString HitComponentName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FString ResultMessage;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMABSAbilityDebugEventSignature, FMABSAbilityDebugEvent, DebugEvent);
