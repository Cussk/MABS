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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0"))
	float CooldownSeconds = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0"))
	float ResourceCost = 0.0f;
};
