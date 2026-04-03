// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Types/MABSAbilityTypes.h"
#include "MABSAbilityDebugTypes.generated.h"

UENUM(BlueprintType)
enum class EMABSDebugEventCategory : uint8
{
	General UMETA(DisplayName="General"),
	Activation UMETA(DisplayName="Activation"),
	Targeting UMETA(DisplayName="Targeting"),
	Delivery UMETA(DisplayName="Delivery"),
	CostCooldown UMETA(DisplayName="Cost / Cooldown"),
	Combo UMETA(DisplayName="Combo"),
	Periodic UMETA(DisplayName="Periodic"),
	Presentation UMETA(DisplayName="Presentation / Cues")
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSAbilityDebugEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FName EventName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	EMABSDebugEventCategory Category = EMABSDebugEventCategory::General;

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
struct MABSCORE_API FMABSGrantedAbilityDebugSummary
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FMABSAbilityHandle AbilityHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FGameplayTag AbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FText DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	EMABSAbilityRuntimeState RuntimeState = EMABSAbilityRuntimeState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	EMABSAbilityActivationResult LastActivationResult = EMABSAbilityActivationResult::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	EMABSDeliveryMode DeliveryMode = EMABSDeliveryMode::Direct;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	EMABSInstantEffectType InstantEffectType = EMABSInstantEffectType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float CooldownRemainingSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FGameplayTag CooldownGroupTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float CooldownGroupRemainingSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float ResourceCost = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float ActivationAgeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float DeliveryRemainingSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float RecoveryRemainingSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	bool bIsBlocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	bool bComboWindowOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float ComboWindowOpensInSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float ComboWindowRemainingSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	bool bCanBufferComboInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FGameplayTag NextComboAbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FGameplayTag QueuedComboAbilityTag;
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSCooldownGroupDebugSummary
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FGameplayTag CooldownGroupTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float RemainingSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSPeriodicEffectDebugSummary
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	int32 RuntimeId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FGameplayTag AbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FText AbilityDisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	EMABSPeriodicEffectType EffectType = EMABSPeriodicEffectType::DOT;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FString SourceActorName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FString TargetActorName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float TickMagnitude = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float TickInterval = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float TimeUntilNextTickSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float TimeRemainingSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSComboDebugSummary
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	bool bHasActiveComboState = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FGameplayTag SourceAbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FText SourceAbilityDisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FGameplayTag NextComboAbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	bool bBufferComboInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	bool bComboWindowOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float ComboWindowOpensInSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	float ComboWindowRemainingSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FGameplayTag QueuedComboAbilityTag;
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
