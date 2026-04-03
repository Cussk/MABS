// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Debug/MABSAbilityDebugTypes.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "Types/MABSAbilityTypes.h"
#include "Types/MABSPresentationTypes.h"
#include "MABSAbilityComponent.generated.h"

class AMABSProjectileBase;
class AActor;
class UAnimMontage;
class UCameraShakeBase;
class UNiagaraSystem;
class USceneComponent;
class USkeletalMeshComponent;
class USoundBase;
class UMABSAbilityDefinition;
struct FHitResult;

struct FMABSAbilityExecutionContext
{
	FTimerHandle DeliveryTimerHandle;
	FTimerHandle RecoveryTimerHandle;
	bool bNotifyOwningClient = false;
};

struct FMABSResolvedAbilityTarget
{
	AActor* TargetActor = nullptr;
	FHitResult ImpactHitResult;
	bool bHasImpactHitResult = false;
};

USTRUCT()
struct FMABSPresentationCueRuntimeData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> VFX = nullptr;

	UPROPERTY()
	TObjectPtr<USoundBase> SFX = nullptr;

	UPROPERTY()
	TSubclassOf<UCameraShakeBase> CameraShakeClass = nullptr;

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	float CameraShakeInnerRadius = 0.0f;

	UPROPERTY()
	float CameraShakeOuterRadius = 0.0f;

	UPROPERTY()
	float CameraShakeFalloff = 1.0f;
};

USTRUCT()
struct FMABSTracerPresentationRuntimeData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> VFX = nullptr;

	UPROPERTY()
	TObjectPtr<USoundBase> SFX = nullptr;

	UPROPERTY()
	FVector TraceStart = FVector::ZeroVector;

	UPROPERTY()
	FVector TraceEnd = FVector::ZeroVector;
};

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

	UFUNCTION(BlueprintPure, Category="MABS|Abilities")
	float GetCooldownRemainingByTag(FGameplayTag AbilityTag) const;

	UFUNCTION(BlueprintPure, Category="MABS|Abilities")
	bool IsAbilityOnCooldown(FGameplayTag AbilityTag) const;

	UFUNCTION(BlueprintPure, Category="MABS|Abilities")
	float GetCooldownGroupRemaining(FGameplayTag CooldownGroupTag) const;

	UFUNCTION(BlueprintPure, Category="MABS|Abilities")
	bool IsCooldownGroupActive(FGameplayTag CooldownGroupTag) const;

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	TArray<FMABSAbilityDebugEvent> GetRecentDebugEvents() const;

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	FMABSTargetTraceDebugInfo GetLatestTargetTraceDebugInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="MABS|Debug")
	void SetDebugReplicationEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	bool IsDebugReplicationEnabled() const;

protected:

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="MABS|Abilities", Transient, Replicated)
	TArray<FMABSAbilitySpec> GrantedAbilities;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="MABS|Abilities", Transient, Replicated)
	TArray<FMABSCooldownGroupState> CooldownGroupStates;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="MABS|Debug", Transient)
	TArray<FMABSAbilityDebugEvent> RecentDebugEvents;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="MABS|Debug", Transient)
	FMABSTargetTraceDebugInfo LatestTargetTraceDebugInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MABS|Debug", meta=(ClampMin="1"))
	int32 MaxStoredDebugEvents = 32;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MABS|Debug", Replicated)
	bool bReplicateDebugDataToOwningClient = true;

private:

	friend class AMABSProjectileBase;

	UFUNCTION(Server, Reliable)
	void ServerTryActivateAbilityByTag(FGameplayTag AbilityTag);

	UFUNCTION(Client, Reliable)
	void ClientReceiveAbilityDebugEvent(const FMABSAbilityDebugEvent& DebugEvent);

	UFUNCTION(Client, Reliable)
	void ClientReceiveTargetTraceDebugInfo(const FMABSTargetTraceDebugInfo& DebugInfo);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayActivationMontage(UAnimMontage* Montage, float PlayRate);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayPresentationCue(const FMABSPresentationCueRuntimeData& CueData);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnTracerPresentation(const FMABSTracerPresentationRuntimeData& TracerData);

	EMABSAbilityActivationResult HandleTryActivateAbility(const FGameplayTag& AbilityTag, bool bNotifyOwningClient);

	EMABSAbilityActivationResult CanActivateAbility(const FMABSAbilitySpec& AbilitySpec, FString& OutDebugMessage) const;

	EMABSAbilityActivationResult BeginAbilityStartup(FMABSAbilitySpec& AbilitySpec, bool bNotifyOwningClient);

	void TriggerScheduledAbilityDelivery(FMABSAbilityHandle AbilityHandle);

	void BeginAbilityRecovery(FMABSAbilitySpec& AbilitySpec, bool bNotifyOwningClient);

	void CompleteAbilityRecovery(FMABSAbilityHandle AbilityHandle);

	void ClearAbilityExecutionContext(FMABSAbilityHandle AbilityHandle, bool bResetRuntimeTimes);

	void ClearAbilityRuntimeTimes(FMABSAbilitySpec& AbilitySpec);

	EMABSAbilityActivationResult CommitAbility(FMABSAbilitySpec& AbilitySpec, bool bNotifyOwningClient);

	FMABSResolvedAbilityTarget ExecuteDirectDelivery(const FMABSAbilitySpec& AbilitySpec, FString& OutDebugMessage, bool bNotifyOwningClient);

	FMABSResolvedAbilityTarget ExecuteHitTraceDelivery(const FMABSAbilitySpec& AbilitySpec, FString& OutDebugMessage, bool bNotifyOwningClient);

	FMABSResolvedAbilityTarget ExecuteMeleeDelivery(const FMABSAbilitySpec& AbilitySpec, FString& OutDebugMessage, bool bNotifyOwningClient);

	EMABSAbilityActivationResult ExecuteProjectileDelivery(FMABSAbilitySpec& AbilitySpec, bool bNotifyOwningClient);

	EMABSAbilityActivationResult CompleteResolvedTargetAbility(
		FMABSAbilitySpec& AbilitySpec,
		const FMABSResolvedAbilityTarget& ResolvedTarget,
		EMABSDeliveryMode DeliveryMode,
		bool bNotifyOwningClient);

	EMABSAbilityActivationResult FinalizeAbilityCommit(
		FMABSAbilitySpec& AbilitySpec,
		EMABSDeliveryMode DeliveryMode,
		bool bNotifyOwningClient);

	AActor* ResolveAbilityTarget(const FMABSAbilitySpec& AbilitySpec, FString& OutDebugMessage, bool bNotifyOwningClient);

	AActor* ResolveActorTargetFromTrace(
		const FMABSAbilitySpec& AbilitySpec,
		const FVector& TraceStart,
		const FVector& TraceEnd,
		EMABSDeliveryMode DeliveryMode,
		EMABSTargetTraceMode TraceMode,
		float TraceRadius,
		bool bIgnoreNonTargetWorldHits,
		const FString& TraceLabel,
		const FString& ViewPointDescription,
		FName TraceStartedEventName,
		FName TraceHitEventName,
		FName TraceRejectedEventName,
		FString& OutDebugMessage,
		FHitResult* OutAcceptedHitResult,
		FVector* OutTraceEndPoint,
		bool bNotifyOwningClient);

	EMABSAbilityActivationResult ApplyInstantEffect(
		const FMABSAbilitySpec& AbilitySpec,
		AActor* TargetActor,
		FString& OutDebugMessage) const;

	EMABSAbilityActivationResult ApplyInstantEffectFromSource(
		const UMABSAbilityDefinition* AbilityDefinition,
		AActor* SourceActor,
		AActor* TargetActor,
		FString& OutDebugMessage) const;

	EMABSAbilityActivationResult HandleProjectileImpact(
		AMABSProjectileBase& Projectile,
		AActor* HitActor,
		const FHitResult& HitResult);

	FMABSAbilitySpec* FindGrantedAbilitySpecByTagMutable(const FGameplayTag& AbilityTag);

	const FMABSAbilitySpec* FindGrantedAbilitySpecByTagInternal(const FGameplayTag& AbilityTag) const;

	FMABSAbilitySpec* FindGrantedAbilitySpecByHandle(const FMABSAbilityHandle& AbilityHandle);

	FMABSCooldownGroupState* FindCooldownGroupStateMutable(const FGameplayTag& CooldownGroupTag);

	const FMABSCooldownGroupState* FindCooldownGroupStateInternal(const FGameplayTag& CooldownGroupTag) const;

	void ResetAbilityToIdle(FMABSAbilityHandle AbilityHandle);

	void PruneExpiredCooldownGroupStates();

	float GetCooldownRemainingForAbilitySpec(const FMABSAbilitySpec& AbilitySpec) const;

	float GetCooldownGroupRemainingInternal(const FGameplayTag& CooldownGroupTag) const;

	bool ShouldValidateAbilityCost(const FMABSAbilitySpec& AbilitySpec) const;

	EMABSAbilityActivationResult SpendAbilityCost(const FMABSAbilitySpec& AbilitySpec, FString& OutDebugMessage) const;

	void StartAbilityCooldowns(FMABSAbilitySpec& AbilitySpec, bool bNotifyOwningClient);

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

	bool GetProjectileSpawnTransform(
		const UMABSAbilityDefinition& AbilityDefinition,
		FTransform& OutSpawnTransform,
		FString& OutDebugMessage,
		bool bNotifyOwningClient);

	bool ResolvePresentationTransform(
		const UMABSAbilityDefinition& AbilityDefinition,
		EMABSDeliveryMode DeliveryMode,
		const FMABSPresentationCueData& CueData,
		FTransform& OutPresentationTransform,
		FString& OutOriginDescription,
		const FGameplayTag& AbilityTag,
		const FMABSAbilityHandle& AbilityHandle,
		EMABSAbilityRuntimeState RuntimeState,
		bool bNotifyOwningClient);

	void TriggerStartupPresentation(const FMABSAbilitySpec& AbilitySpec, bool bNotifyOwningClient);

	void TriggerDeliveryPresentation(const FMABSAbilitySpec& AbilitySpec, EMABSDeliveryMode DeliveryMode, bool bNotifyOwningClient);

	void TriggerTracerPresentation(
		const FMABSAbilitySpec& AbilitySpec,
		const FVector& TraceStart,
		const FVector& TraceEnd,
		bool bNotifyOwningClient);

	void TriggerImpactPresentation(
		const FMABSAbilitySpec& AbilitySpec,
		const FMABSResolvedAbilityTarget& ResolvedTarget,
		EMABSDeliveryMode DeliveryMode,
		bool bNotifyOwningClient);

	void TriggerProjectileImpactPresentation(
		const UMABSAbilityDefinition& AbilityDefinition,
		const FGameplayTag& AbilityTag,
		const FMABSAbilityHandle& AbilityHandle,
		const FHitResult& HitResult,
		AActor* HitActor);

	void PlayPresentationCueLocally(const FMABSPresentationCueRuntimeData& CueData) const;

	void SpawnTracerPresentationLocally(const FMABSTracerPresentationRuntimeData& TracerData) const;

	float GetEffectiveDeliveryDelay(const UMABSAbilityDefinition& AbilityDefinition) const;

	bool RequestAbilityMontagePlayback(const FMABSAbilitySpec& AbilitySpec, bool bNotifyOwningClient);

	bool PlayActivationMontageLocally(UAnimMontage* Montage, float PlayRate) const;

	USkeletalMeshComponent* ResolveAbilitySkeletalMeshComponent() const;

	void PrepareSkeletalMeshForAbilitySocketQueries(USkeletalMeshComponent& SkeletalMeshComponent) const;

	FVector ResolveMeleeTraceDirection() const;

	bool TryResolveSocketTransform(const FName& SocketName, FTransform& OutSocketTransform, FString& OutComponentName) const;

	bool ResolveDeliveryOriginTransform(
		const UMABSAbilityDefinition& AbilityDefinition,
		EMABSDeliveryMode DeliveryMode,
		FTransform& OutOriginTransform,
		FString& OutOriginDescription,
		bool bNotifyOwningClient);

	bool ValidateTargetActorForAbility(
		const UMABSAbilityDefinition* AbilityDefinition,
		const AActor* SourceActor,
		const AActor* CandidateActor,
		bool bRequireValidActorTarget,
		FString& OutRejectionReason) const;

	void DrawTargetTraceDebug(const UMABSAbilityDefinition& AbilityDefinition, const FMABSTargetTraceDebugInfo& DebugInfo) const;

	bool CanMutateAbilityState() const;

	FMABSAbilityHandle MakeNextAbilityHandle();

	int32 NextAbilityHandleValue = 1;

	TMap<FMABSAbilityHandle, FMABSAbilityExecutionContext> ActiveAbilityExecutionContexts;
};
