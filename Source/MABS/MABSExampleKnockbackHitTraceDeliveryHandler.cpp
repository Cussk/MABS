// Copyright Epic Games, Inc. All Rights Reserved.

#include "MABSExampleKnockbackHitTraceDeliveryHandler.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"

void UMABSExampleKnockbackHitTraceDeliveryHandler::ModifyDeliveryResult_Implementation(
	const FMABSDeliveryExecutionContext& Context,
	FMABSDeliveryExecutionResult& Result) const
{
	Super::ModifyDeliveryResult_Implementation(Context, Result);

	if (!Result.bDeliverySucceeded)
	{
		return;
	}

	const AActor* const SourceActor = Context.SourceActor;
	for (AActor* const TargetActor : Result.ResolvedTargetActors)
	{
		if (TargetActor == nullptr || TargetActor == SourceActor)
		{
			continue;
		}

		FVector ImpulseDirection = !Result.ImpactNormal.IsNearlyZero()
			? -Result.ImpactNormal.GetSafeNormal()
			: FVector::ZeroVector;
		if (ImpulseDirection.IsNearlyZero() && SourceActor != nullptr)
		{
			ImpulseDirection = (TargetActor->GetActorLocation() - SourceActor->GetActorLocation()).GetSafeNormal();
		}
		if (ImpulseDirection.IsNearlyZero())
		{
			ImpulseDirection = FVector::ForwardVector;
		}

		const FVector KnockbackImpulse = (ImpulseDirection * HorizontalImpulse) + (FVector::UpVector * VerticalImpulse);
		if (ACharacter* const Character = Cast<ACharacter>(TargetActor))
		{
			Character->LaunchCharacter(KnockbackImpulse, true, true);
			continue;
		}

		if (UPrimitiveComponent* const RootPrimitive = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent()))
		{
			if (RootPrimitive->IsSimulatingPhysics())
			{
				RootPrimitive->AddImpulse(KnockbackImpulse, NAME_None, true);
			}
		}
	}
}
