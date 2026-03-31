// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Debug/MABSAbilityDebugTypes.h"
#include "GameplayTagContainer.h"
#include "Types/MABSAbilityTypes.h"
#include "MABSAbilityComponent.generated.h"

class UMABSAbilityDefinition;

UCLASS(ClassGroup=(MABS), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class MABS_API UMABSAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UMABSAbilityComponent();

	UPROPERTY(BlueprintAssignable, Category="MABS|Debug")
	FMABSAbilityDebugEventSignature OnAbilityDebugEvent;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="MABS|Abilities")
	FMABSAbilityHandle GrantAbility(UMABSAbilityDefinition* AbilityDefinition);

	UFUNCTION(BlueprintCallable, Category="MABS|Abilities")
	EMABSAbilityActivationResult TryActivateAbilityByTag(FGameplayTag AbilityTag);

	UFUNCTION(BlueprintPure, Category="MABS|Abilities")
	TArray<FMABSAbilitySpec> GetGrantedAbilities() const;

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	TArray<FMABSAbilityDebugEvent> GetRecentDebugEvents() const;

protected:

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="MABS|Abilities", Transient)
	TArray<FMABSAbilitySpec> GrantedAbilities;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="MABS|Debug", Transient)
	TArray<FMABSAbilityDebugEvent> RecentDebugEvents;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MABS|Debug", meta=(ClampMin="1"))
	int32 MaxStoredDebugEvents = 32;

private:

	FMABSAbilitySpec* FindGrantedAbilityByTag(const FGameplayTag& AbilityTag);

	void EmitDebugEvent(
		FName EventName,
		const FGameplayTag& AbilityTag,
		const FMABSAbilityHandle& AbilityHandle,
		EMABSAbilityRuntimeState RuntimeState,
		EMABSAbilityActivationResult ActivationResult,
		const FString& Message);

	bool CanMutateAbilityState() const;

	FMABSAbilityHandle MakeNextAbilityHandle();

	int32 NextAbilityHandleValue = 1;
};
