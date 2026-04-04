// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delivery/MABSDeliveryHandler.h"
#include "MABSHitTraceDeliveryHandler.generated.h"

UCLASS(BlueprintType, Blueprintable)
class MABSGAMEPLAY_API UMABSHitTraceDeliveryHandler : public UMABSDeliveryHandler
{
	GENERATED_BODY()

public:
	virtual FMABSDeliveryExecutionResult ExecuteDelivery(const FMABSDeliveryExecutionContext& Context) const override;
};
