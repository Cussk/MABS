// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Types/MABSAbilityTypes.h"
#include "Types/MABSPresentationTypes.h"

class AActor;
class UMABSAbilityDefinition;
class UMABSAbilitySet;

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
	FString GetAbilityLabel(const UMABSAbilityDefinition* AbilityDefinition);
	FText GetAbilityDebugDisplayName(const UMABSAbilityDefinition* AbilityDefinition);
	FString GetAbilitySetLabel(const UMABSAbilitySet* AbilitySet);
	FString FormatVectorForDebug(const FVector& Vector);
	FString DescribeActorList(const TArray<AActor*>& Actors);
	FString GetHitActorLabel(const AActor* Actor);
}
