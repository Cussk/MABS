// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delivery/MABSHitTraceDeliveryHandler.h"
#include "MABSExampleKnockbackHitTraceDeliveryHandler.generated.h"

UCLASS(BlueprintType, Blueprintable)
class MABS_API UMABSExampleKnockbackHitTraceDeliveryHandler : public UMABSHitTraceDeliveryHandler
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Knockback", meta=(ClampMin="0.0"))
	float HorizontalImpulse = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Knockback", meta=(ClampMin="0.0"))
	float VerticalImpulse = 200.0f;

protected:
	virtual void ModifyDeliveryResult_Implementation(
		const FMABSDeliveryExecutionContext& Context,
		FMABSDeliveryExecutionResult& Result) const override;
};
