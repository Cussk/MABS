// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Types/MABSDeliveryHandlerTypes.h"
#include "UObject/Object.h"
#include "MABSDeliveryHandler.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class MABSGAMEPLAY_API UMABSDeliveryHandler : public UObject
{
	GENERATED_BODY()

public:
	virtual FMABSDeliveryExecutionResult ExecuteDelivery(const FMABSDeliveryExecutionContext& Context) const;

protected:
	UFUNCTION(BlueprintNativeEvent, Category="MABS|Delivery")
	void ModifyDeliveryResult(const FMABSDeliveryExecutionContext& Context, UPARAM(ref) FMABSDeliveryExecutionResult& Result) const;

	virtual void ModifyDeliveryResult_Implementation(
		const FMABSDeliveryExecutionContext& Context,
		FMABSDeliveryExecutionResult& Result) const;

	void ApplyResultModifier(const FMABSDeliveryExecutionContext& Context, FMABSDeliveryExecutionResult& Result) const;
};
