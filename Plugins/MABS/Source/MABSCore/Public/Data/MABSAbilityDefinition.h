// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Types/MABSAbilityTypes.h"
#include "MABSAbilityDefinition.generated.h"

class AActor;
class UAnimMontage;

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
	EMABSDeliveryMode DeliveryMode = EMABSDeliveryMode::Direct;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	EMABSTargetType TargetType = EMABSTargetType::Self;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	EMABSInstantEffectType InstantEffectType = EMABSInstantEffectType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0"))
	float EffectMagnitude = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Timing", meta=(ClampMin="0.0"))
	float StartupDuration = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Timing", meta=(ClampMin="0.0"))
	float DeliveryTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Timing", meta=(ClampMin="0.0"))
	float RecoveryDuration = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0"))
	float TargetTraceDistance = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	EMABSTargetTraceMode ActorTargetTraceMode = EMABSTargetTraceMode::Sphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0"))
	float TargetTraceRadius = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	bool bRequireValidActorTarget = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	bool bIgnoreNonTargetWorldHits = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets")
	FName DeliveryOriginSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets")
	FName HitTraceOriginSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0", EditCondition="DeliveryMode == EMABSDeliveryMode::HitTrace", EditConditionHides))
	float HitTraceDistance = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0", EditCondition="DeliveryMode == EMABSDeliveryMode::HitTrace", EditConditionHides))
	float HitTraceRadius = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets", meta=(EditCondition="DeliveryMode == EMABSDeliveryMode::HitTrace", EditConditionHides))
	FVector HitTraceOriginOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets")
	FName MeleeOriginSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0", EditCondition="DeliveryMode == EMABSDeliveryMode::Melee", EditConditionHides))
	float MeleeRange = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0", EditCondition="DeliveryMode == EMABSDeliveryMode::Melee", EditConditionHides))
	float MeleeRadius = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets", meta=(EditCondition="DeliveryMode == EMABSDeliveryMode::Melee", EditConditionHides))
	FVector MeleeOriginOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(EditCondition="DeliveryMode == EMABSDeliveryMode::Melee", EditConditionHides))
	float MeleeForwardOffset = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets")
	FName ProjectileSpawnSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(EditCondition="DeliveryMode == EMABSDeliveryMode::Projectile", EditConditionHides))
	TSubclassOf<AActor> ProjectileActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(EditCondition="DeliveryMode == EMABSDeliveryMode::Projectile", EditConditionHides))
	FVector ProjectileSpawnOffset = FVector(100.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> ActivationMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation", meta=(ClampMin="0.01", EditCondition="ActivationMontage != nullptr", EditConditionHides))
	float MontagePlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Debug")
	bool bDrawTargetTraceDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Debug", meta=(ClampMin="0.0"))
	float TargetTraceDebugDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0"))
	float CooldownSeconds = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	FGameplayTag CooldownGroupTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="0.0"))
	float ResourceCost = 0.0f;
};
