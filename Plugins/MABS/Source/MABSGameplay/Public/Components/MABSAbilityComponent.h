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
class MABSGAMEPLAY_API UMABSAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UMABSAbilityComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category="MABS|Debug")
	FMABSAbilityDebugEventSignature OnAbilityDebugEvent;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="MABS|Abilities")
	FMABSAbilityHandle GrantAbility(UMABSAbilityDefinition* AbilityDefinition);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="MABS|Abilities")
	bool SetAbilityBlockedByTag(FGameplayTag AbilityTag, bool bBlocked);

	UFUNCTION(BlueprintCallable, Category="MABS|Abilities")
	EMABSAbilityActivationResult TryActivateAbilityByTag(FGameplayTag AbilityTag);

	UFUNCTION(BlueprintPure, Category="MABS|Abilities")
	TArray<FMABSAbilitySpec> GetGrantedAbilities() const;

	UFUNCTION(BlueprintPure, Category="MABS|Abilities")
	bool FindGrantedAbilitySpecByTag(FGameplayTag AbilityTag, FMABSAbilitySpec& OutAbilitySpec) const;

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	TArray<FMABSAbilityDebugEvent> GetRecentDebugEvents() const;

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	FMABSTargetTraceDebugInfo GetLatestTargetTraceDebugInfo() const;

protected:

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="MABS|Abilities", Transient, Replicated)
	TArray<FMABSAbilitySpec> GrantedAbilities;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="MABS|Debug", Transient)
	TArray<FMABSAbilityDebugEvent> RecentDebugEvents;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="MABS|Debug", Transient)
	FMABSTargetTraceDebugInfo LatestTargetTraceDebugInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MABS|Debug", meta=(ClampMin="1"))
	int32 MaxStoredDebugEvents = 32;

private:

	UFUNCTION(Server, Reliable)
	void ServerTryActivateAbilityByTag(FGameplayTag AbilityTag);

	UFUNCTION(Client, Reliable)
	void ClientReceiveAbilityDebugEvent(const FMABSAbilityDebugEvent& DebugEvent);

	UFUNCTION(Client, Reliable)
	void ClientReceiveTargetTraceDebugInfo(const FMABSTargetTraceDebugInfo& DebugInfo);

	EMABSAbilityActivationResult HandleTryActivateAbility(const FGameplayTag& AbilityTag, bool bNotifyOwningClient);

	EMABSAbilityActivationResult CanActivateAbility(const FMABSAbilitySpec& AbilitySpec) const;

	EMABSAbilityActivationResult CommitAbility(FMABSAbilitySpec& AbilitySpec, bool bNotifyOwningClient);

	AActor* ResolveAbilityTarget(const FMABSAbilitySpec& AbilitySpec, FString& OutDebugMessage, bool bNotifyOwningClient);

	EMABSAbilityActivationResult ApplyInstantEffect(
		const FMABSAbilitySpec& AbilitySpec,
		AActor* TargetActor,
		FString& OutDebugMessage) const;

	FMABSAbilitySpec* FindGrantedAbilitySpecByTagMutable(const FGameplayTag& AbilityTag);

	const FMABSAbilitySpec* FindGrantedAbilitySpecByTagInternal(const FGameplayTag& AbilityTag) const;

	FMABSAbilitySpec* FindGrantedAbilitySpecByHandle(const FMABSAbilityHandle& AbilityHandle);

	void ResetAbilityToIdle(FMABSAbilityHandle AbilityHandle);

	FMABSAbilityDebugEvent MakeDebugEvent(
		FName EventName,
		const FGameplayTag& AbilityTag,
		const FMABSAbilityHandle& AbilityHandle,
		EMABSAbilityRuntimeState RuntimeState,
		EMABSAbilityActivationResult ActivationResult,
		const FString& Message) const;

	void RecordDebugEvent(const FMABSAbilityDebugEvent& DebugEvent);

	FMABSAbilityDebugEvent EmitDebugEvent(
		FName EventName,
		const FGameplayTag& AbilityTag,
		const FMABSAbilityHandle& AbilityHandle,
		EMABSAbilityRuntimeState RuntimeState,
		EMABSAbilityActivationResult ActivationResult,
		const FString& Message);

	void EmitDebugEventToOwningClient(const FMABSAbilityDebugEvent& DebugEvent);

	void RecordLatestTargetTraceDebugInfo(const FMABSTargetTraceDebugInfo& DebugInfo);

	void EmitLatestTargetTraceDebugInfoToOwningClient(const FMABSTargetTraceDebugInfo& DebugInfo);

	void ClearLatestTargetTraceDebugInfo(bool bNotifyOwningClient);

	bool GetTargetTraceViewPoint(FVector& OutTraceStart, FRotator& OutTraceRotation, FString& OutViewPointDescription) const;

	bool ValidateResolvedTargetActor(const FMABSAbilitySpec& AbilitySpec, const AActor* CandidateActor, FString& OutRejectionReason) const;

	void DrawTargetTraceDebug(const UMABSAbilityDefinition& AbilityDefinition, const FMABSTargetTraceDebugInfo& DebugInfo) const;

	bool CanMutateAbilityState() const;

	FMABSAbilityHandle MakeNextAbilityHandle();

	int32 NextAbilityHandleValue = 1;
};
