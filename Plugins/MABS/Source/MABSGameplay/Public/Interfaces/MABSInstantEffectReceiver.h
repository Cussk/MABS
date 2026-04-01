// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/MABSAbilityDefinition.h"
#include "UObject/Interface.h"
#include "MABSInstantEffectReceiver.generated.h"

class AActor;

UINTERFACE(BlueprintType)
class MABSGAMEPLAY_API UMABSInstantEffectReceiver : public UInterface
{
	GENERATED_BODY()
};

class MABSGAMEPLAY_API IMABSInstantEffectReceiver
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="MABS|Effects")
	bool ApplyMABSHeal(float HealAmount, AActor* SourceActor, UMABSAbilityDefinition* SourceAbility);
};
