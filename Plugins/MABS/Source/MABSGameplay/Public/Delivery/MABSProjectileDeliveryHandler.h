// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delivery/MABSDeliveryHandler.h"
#include "MABSProjectileDeliveryHandler.generated.h"

UCLASS(BlueprintType, Blueprintable)
class MABSGAMEPLAY_API UMABSProjectileDeliveryHandler : public UMABSDeliveryHandler
{
	GENERATED_BODY()

public:
	virtual FMABSDeliveryExecutionResult ExecuteDelivery(const FMABSDeliveryExecutionContext& Context) const override;
};
