// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/MABSAbilityComponent.h"
#include "Components/MABSAbilityRuntime_Internal.h"

UMABSAbilityComponent::UMABSAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}
