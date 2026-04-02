// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MABSAbilityTypes.generated.h"

class UMABSAbilityDefinition;

UENUM(BlueprintType)
enum class EMABSAbilityActivationResult : uint8
{
	None UMETA(DisplayName="None"),
	Success UMETA(DisplayName="Success"),
	RequestSentToServer UMETA(DisplayName="Request Sent To Server"),
	InvalidAbility UMETA(DisplayName="Invalid Ability"),
	NotGranted UMETA(DisplayName="Not Granted"),
	AlreadyActive UMETA(DisplayName="Already Active"),
	Blocked UMETA(DisplayName="Blocked"),
	OnCooldown UMETA(DisplayName="On Cooldown"),
	InsufficientResources UMETA(DisplayName="Insufficient Resources"),
	AuthorityRejected UMETA(DisplayName="Authority Rejected"),
	DeliveryFailed UMETA(DisplayName="Delivery Failed"),
	TargetResolutionFailed UMETA(DisplayName="Target Resolution Failed"),
	EffectApplicationFailed UMETA(DisplayName="Effect Application Failed")
};

UENUM(BlueprintType)
enum class EMABSAbilityRuntimeState : uint8
{
	None UMETA(DisplayName="None"),
	Idle UMETA(DisplayName="Idle"),
	Active UMETA(DisplayName="Active"),
	Blocked UMETA(DisplayName="Blocked")
};

UENUM(BlueprintType)
enum class EMABSTargetType : uint8
{
	None UMETA(DisplayName="None"),
	Self UMETA(DisplayName="Self"),
	Actor UMETA(DisplayName="Actor"),
	Location UMETA(DisplayName="Location")
};

UENUM(BlueprintType)
enum class EMABSTargetTraceMode : uint8
{
	Line UMETA(DisplayName="Line"),
	Sphere UMETA(DisplayName="Sphere")
};

UENUM(BlueprintType)
enum class EMABSAbilityActivationPolicy : uint8
{
	OnInputTriggered UMETA(DisplayName="On Input Triggered"),
	Passive UMETA(DisplayName="Passive")
};

UENUM(BlueprintType)
enum class EMABSDeliveryMode : uint8
{
	Direct UMETA(DisplayName="Direct"),
	HitTrace UMETA(DisplayName="HitTrace"),
	Melee UMETA(DisplayName="Melee"),
	Projectile UMETA(DisplayName="Projectile")
};

UENUM(BlueprintType)
enum class EMABSInstantEffectType : uint8
{
	None UMETA(DisplayName="None"),
	Damage UMETA(DisplayName="Damage"),
	Heal UMETA(DisplayName="Heal")
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSAbilityHandle
{
	GENERATED_BODY()

	FMABSAbilityHandle() = default;

	explicit FMABSAbilityHandle(const int32 InValue)
		: Value(InValue)
	{
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	int32 Value = INDEX_NONE;

	bool IsValid() const
	{
		return Value != INDEX_NONE;
	}

	bool operator==(const FMABSAbilityHandle& Other) const
	{
		return Value == Other.Value;
	}

	bool operator!=(const FMABSAbilityHandle& Other) const
	{
		return !(*this == Other);
	}
};

FORCEINLINE uint32 GetTypeHash(const FMABSAbilityHandle& Handle)
{
	return GetTypeHash(Handle.Value);
}

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSAbilitySpec
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	FMABSAbilityHandle Handle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UMABSAbilityDefinition> AbilityDefinition = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	FGameplayTag AbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	EMABSAbilityRuntimeState RuntimeState = EMABSAbilityRuntimeState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	EMABSAbilityActivationResult LastActivationResult = EMABSAbilityActivationResult::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	float CooldownEndTime = 0.0f;
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSCooldownGroupState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	FGameplayTag CooldownGroupTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	float CooldownEndTime = 0.0f;
};
