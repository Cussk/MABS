// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/MABSAbilityComponent.h"
#include "Actors/MABSProjectileBase.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraShakeBase.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "Data/MABSAbilityDefinition.h"
#include "Data/MABSAbilitySet.h"
#include "Debug/MABSAbilitySystemLogs.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/MABSCostReceiver.h"
#include "Interfaces/MABSInstantEffectReceiver.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

namespace MABSAbilityComponentEventNames
{
	extern const FName AbilityBlocked;
	extern const FName AbilityGranted;
	extern const FName AbilityGrantRejected;
	extern const FName AbilitySetGranted;
	extern const FName AbilitySetGrantFailed;
	extern const FName AbilitySetGrantSkipped;
	extern const FName AbilityUnblocked;
	extern const FName CommitSucceeded;
	extern const FName CooldownRejected;
	extern const FName CooldownStarted;
	extern const FName ComboQueued;
	extern const FName ComboRejected;
	extern const FName ComboWindowEnded;
	extern const FName ComboWindowStarted;
	extern const FName DeliveryFailed;
	extern const FName DeliveryStarted;
	extern const FName EffectApplied;
	extern const FName EffectApplicationFailed;
	extern const FName AoEResolved;
	extern const FName AoETargetRejected;
	extern const FName CostRejected;
	extern const FName CostSpent;
	extern const FName CostValidated;
	extern const FName HitTraceHit;
	extern const FName HitTraceRejected;
	extern const FName MeleeHit;
	extern const FName MeleeRejected;
	extern const FName MontagePlayFailed;
	extern const FName MontagePlayRequested;
	extern const FName PresentationAssetMissing;
	extern const FName PresentationCuePolicyFallbackUsed;
	extern const FName PresentationCueRealized;
	extern const FName PresentationCueRouted;
	extern const FName PresentationCueSkipped;
	extern const FName PresentationSocketFallbackUsed;
	extern const FName ProjectileImpact;
	extern const FName ProjectileImpactRejected;
	extern const FName ProjectileTravelPresentationTriggered;
	extern const FName ProjectileSpawned;
	extern const FName ProjectileSpawnFailed;
	extern const FName PeriodicEffectApplied;
	extern const FName PeriodicEffectExpired;
	extern const FName PeriodicEffectRefreshed;
	extern const FName PeriodicEffectTick;
	extern const FName RecoveryCompleted;
	extern const FName RecoveryStarted;
	extern const FName RequestAccepted;
	extern const FName RequestRejected;
	extern const FName RequestSentToServer;
	extern const FName RequestStarted;
	extern const FName SocketFallbackUsed;
	extern const FName SocketResolved;
	extern const FName StartupStarted;
	extern const FName StartupPresentationTriggered;
	extern const FName TargetTraceHit;
	extern const FName TargetTraceRejected;
	extern const FName TargetTraceStarted;
	extern const FName TargetResolved;
	extern const FName TargetResolutionFailed;
	extern const FName DeliveryScheduled;
	extern const FName DeliveryTriggered;
	extern const FName DeliveryPresentationTriggered;
	extern const FName ImpactPresentationTriggered;
	extern const FName TracerCueRealized;
	extern const FName TracerCueRouted;
	extern const FName TracerCueSkipped;
	extern const FName TracerSpawned;
	extern const FName TracerSpawnFailed;
}

namespace MABSAbilityRuntimeInternal
{
	FString DescribeActivationResult(EMABSAbilityActivationResult ActivationResult);
	FString GetTargetTypeLabel(EMABSTargetType TargetType);
	FString GetDeliveryModeLabel(EMABSDeliveryMode DeliveryMode);
	FString GetRuntimeStateLabel(EMABSAbilityRuntimeState RuntimeState);
	FString GetPresentationCuePhaseLabel(EMABSPresentationCuePhase CuePhase);
	FString GetPresentationVisibilityPolicyLabel(EMABSPresentationCueVisibilityPolicy VisibilityPolicy);
	FString GetTargetTraceModeLabel(EMABSTargetTraceMode TraceMode);
	FString GetInstantEffectTypeLabel(EMABSInstantEffectType EffectType);
	FString GetPeriodicEffectTypeLabel(EMABSPeriodicEffectType EffectType);
	FString GetAoEShapeLabel(EMABSAoEShape Shape);
	EMABSDebugEventCategory GetDebugEventCategory(FName EventName);
	FString GetAbilityLabel(const UMABSAbilityDefinition* AbilityDefinition);
	FText GetAbilityDebugDisplayName(const UMABSAbilityDefinition* AbilityDefinition);
	FString GetAbilitySetLabel(const UMABSAbilitySet* AbilitySet);
	FString FormatVectorForDebug(const FVector& Vector);
	FString DescribeActorList(const TArray<AActor*>& Actors);
	FString GetHitActorLabel(const AActor* Actor);
	FString DescribeCueAssets(const FMABSPresentationCueData& CueData);
	FString DescribeCueEventAssets(const FMABSPresentationCueEvent& CueEvent);
	FString DescribeTracerAssets(const FMABSHitTraceTracerPresentationData& TracerData);
	FString DescribeTracerCueAssets(const FMABSTracerCueEvent& TracerEvent);
	FName GetActivationFailureEventName(EMABSAbilityActivationResult ActivationResult);
	EMABSTargetTraceMode GetTraceModeForRadius(float TraceRadius);
	FCollisionObjectQueryParams MakeTargetTraceObjectQueryParams(bool bIgnoreNonTargetWorldHits);

	extern const FName NiagaraTracerTraceStartParameter;
	extern const FName NiagaraTracerTraceEndParameter;
	extern const FName NiagaraTracerImpactPointParameter;
}
