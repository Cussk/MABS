// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "Types/MABSAbilityTypes.h"
#include "MABSDeliveryHandlerTypes.generated.h"

class AActor;
class APawn;
class AController;
class UMABSAbilityComponent;
class UMABSAbilityDefinition;

USTRUCT(BlueprintType)
struct MABSGAMEPLAY_API FMABSDeliveryExecutionContext
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Delivery")
	TObjectPtr<UMABSAbilityComponent> AbilityComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Delivery")
	TObjectPtr<UMABSAbilityDefinition> AbilityDefinition = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Delivery")
	FMABSAbilitySpec AbilitySpec;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Delivery")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Delivery")
	TObjectPtr<APawn> InstigatorPawn = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Delivery")
	TObjectPtr<AController> InstigatorController = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Delivery")
	EMABSDeliveryMode DeliveryMode = EMABSDeliveryMode::Direct;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Delivery")
	FTransform SourceTransform = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Delivery")
	bool bNotifyOwningClient = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Delivery")
	bool bHasViewPoint = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Delivery")
	FVector ViewPointLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Delivery")
	FRotator ViewPointRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Delivery")
	FString ViewPointDescription;
};

USTRUCT(BlueprintType)
struct MABSGAMEPLAY_API FMABSDeliveryExecutionResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery")
	bool bDeliverySucceeded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery")
	EMABSAbilityActivationResult FailureResult = EMABSAbilityActivationResult::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery")
	FName FailureEventName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery")
	bool bApplyStandardEffects = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery")
	bool bTriggerStandardImpactPresentation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery")
	TObjectPtr<AActor> PrimaryTargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery")
	TArray<TObjectPtr<AActor>> ResolvedTargetActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery")
	bool bHasImpact = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery")
	FHitResult PrimaryHitResult;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery")
	FVector ImpactLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery")
	FVector ImpactNormal = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery")
	FString DebugMessage;

	void AddResolvedTarget(AActor* TargetActor)
	{
		if (TargetActor == nullptr || ResolvedTargetActors.Contains(TargetActor))
		{
			return;
		}

		ResolvedTargetActors.Add(TargetActor);
		if (PrimaryTargetActor == nullptr)
		{
			PrimaryTargetActor = TargetActor;
		}
	}

	void SetPrimaryImpactFromHitResult(const FHitResult& HitResult)
	{
		bHasImpact = true;
		PrimaryHitResult = HitResult;
		ImpactLocation = !HitResult.ImpactPoint.IsNearlyZero() ? HitResult.ImpactPoint : HitResult.Location;
		ImpactNormal = !HitResult.ImpactNormal.IsNearlyZero() ? HitResult.ImpactNormal : FVector::UpVector;

		if (PrimaryTargetActor == nullptr)
		{
			PrimaryTargetActor = HitResult.GetActor();
		}
	}
};
