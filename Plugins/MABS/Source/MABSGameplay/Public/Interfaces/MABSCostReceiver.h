// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/MABSAbilityDefinition.h"
#include "UObject/Interface.h"
#include "MABSCostReceiver.generated.h"

UINTERFACE(BlueprintType)
class MABSGAMEPLAY_API UMABSCostReceiver : public UInterface
{
	GENERATED_BODY()
};

class MABSGAMEPLAY_API IMABSCostReceiver
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="MABS|Costs")
	bool CanAffordMABSCost(float Cost, UMABSAbilityDefinition* SourceAbility);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="MABS|Costs")
	bool SpendMABSCost(float Cost, UMABSAbilityDefinition* SourceAbility);
};
