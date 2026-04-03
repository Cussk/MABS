// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MABSAbilitySet.generated.h"

class UMABSAbilityDefinition;

UCLASS(BlueprintType)
class MABSCORE_API UMABSAbilitySet : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities")
	TArray<TObjectPtr<UMABSAbilityDefinition>> AbilityDefinitions;
};
