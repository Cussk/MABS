// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Types/MABSAbilityTypes.h"
#include "MABSAbilityDefinition.generated.h"

UCLASS(BlueprintType)
class MABSCORE_API UMABSAbilityDefinition : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(Categories="Ability"))
	FGameplayTag AbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	EMABSAbilityActivationPolicy ActivationPolicy = EMABSAbilityActivationPolicy::OnInputTriggered;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	EMABSTargetType TargetType = EMABSTargetType::Self;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	EMABSInstantEffectType InstantEffectType = EMABSInstantEffectType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0"))
	float EffectMagnitude = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0"))
	float TargetTraceDistance = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	EMABSTargetTraceMode ActorTargetTraceMode = EMABSTargetTraceMode::Sphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0"))
	float TargetTraceRadius = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	bool bRequireValidActorTarget = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	bool bIgnoreNonTargetWorldHits = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Debug")
	bool bDrawTargetTraceDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Debug", meta=(ClampMin="0.0"))
	float TargetTraceDebugDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0"))
	float CooldownSeconds = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	FGameplayTag CooldownGroupTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0"))
	float ResourceCost = 0.0f;
};
