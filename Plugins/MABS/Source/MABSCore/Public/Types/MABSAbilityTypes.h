// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MABSAbilityTypes.generated.h"

class UMABSAbilityDefinition;
class AActor;

UENUM(BlueprintType)
enum class EMABSAbilityActivationResult : uint8
{
	None UMETA(DisplayName="None"),
	Success UMETA(DisplayName="Success"),
	ComboQueued UMETA(DisplayName="Combo Queued"),
	RequestSentToServer UMETA(DisplayName="Request Sent To Server"),
	InvalidAbility UMETA(DisplayName="Invalid Ability"),
	NotGranted UMETA(DisplayName="Not Granted"),
	AlreadyActive UMETA(DisplayName="Already Active"),
	Blocked UMETA(DisplayName="Blocked"),
	OnCooldown UMETA(DisplayName="On Cooldown"),
	InsufficientResources UMETA(DisplayName="Insufficient Resources"),
	ComboRejected UMETA(DisplayName="Combo Rejected"),
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
	Startup UMETA(DisplayName="Startup"),
	Active UMETA(DisplayName="Active"),
	Recovery UMETA(DisplayName="Recovery"),
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

UENUM(BlueprintType)
enum class EMABSAoEShape : uint8
{
	Sphere UMETA(DisplayName="Sphere"),
	Box UMETA(DisplayName="Box"),
	Capsule UMETA(DisplayName="Capsule")
};

UENUM(BlueprintType)
enum class EMABSPeriodicEffectType : uint8
{
	DOT UMETA(DisplayName="DOT"),
	HOT UMETA(DisplayName="HOT")
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSComboData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo", meta=(Categories="Ability"))
	FGameplayTag NextComboAbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo", meta=(ClampMin="0.0"))
	float ComboWindowStart = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo", meta=(ClampMin="0.0"))
	float ComboWindowEnd = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo")
	bool bBufferComboInput = true;

	bool IsEnabled() const
	{
		return NextComboAbilityTag.IsValid() && ComboWindowEnd > ComboWindowStart;
	}
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSAoEData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AoE")
	bool bEnabled = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AoE", meta=(EditCondition="bEnabled", EditConditionHides))
	EMABSAoEShape Shape = EMABSAoEShape::Sphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AoE", meta=(ClampMin="0.0", EditCondition="bEnabled && Shape == EMABSAoEShape::Sphere", EditConditionHides))
	float Radius = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AoE", meta=(EditCondition="bEnabled && Shape == EMABSAoEShape::Box", EditConditionHides))
	FVector BoxExtent = FVector(150.0f, 150.0f, 150.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AoE", meta=(ClampMin="0.0", EditCondition="bEnabled && Shape == EMABSAoEShape::Capsule", EditConditionHides))
	float CapsuleRadius = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AoE", meta=(ClampMin="0.0", EditCondition="bEnabled && Shape == EMABSAoEShape::Capsule", EditConditionHides))
	float CapsuleHalfHeight = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AoE", meta=(EditCondition="bEnabled", EditConditionHides))
	FVector Offset = FVector::ZeroVector;

	bool IsValid() const
	{
		if (!bEnabled)
		{
			return false;
		}

		switch (Shape)
		{
		case EMABSAoEShape::Sphere:
			return Radius > 0.0f;

		case EMABSAoEShape::Box:
			return BoxExtent.GetMax() > 0.0f;

		case EMABSAoEShape::Capsule:
			return CapsuleRadius > 0.0f && CapsuleHalfHeight > 0.0f;

		default:
			return false;
		}
	}
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSPeriodicEffectData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Periodic")
	bool bEnabled = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Periodic", meta=(EditCondition="bEnabled", EditConditionHides))
	EMABSPeriodicEffectType EffectType = EMABSPeriodicEffectType::DOT;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Periodic", meta=(ClampMin="0.0", EditCondition="bEnabled", EditConditionHides))
	float Duration = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Periodic", meta=(ClampMin="0.01", EditCondition="bEnabled", EditConditionHides))
	float TickInterval = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Periodic", meta=(ClampMin="0.0", EditCondition="bEnabled", EditConditionHides))
	float TickMagnitude = 5.0f;

	bool IsValid() const
	{
		return bEnabled && Duration > 0.0f && TickInterval > 0.0f && TickMagnitude > 0.0f;
	}
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	float ActivationStartTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	float ScheduledDeliveryTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	float RecoveryEndTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	float ComboWindowStartTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	float ComboWindowEndTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	FGameplayTag QueuedComboAbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	FGameplayTag ComboInputRoutingTag;
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

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSActivePeriodicEffect
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Periodic")
	int32 RuntimeId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Periodic")
	FGameplayTag AbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Periodic")
	FMABSAbilityHandle AbilityHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Periodic")
	TObjectPtr<UMABSAbilityDefinition> AbilityDefinition = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Periodic")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Periodic")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Periodic")
	EMABSPeriodicEffectType EffectType = EMABSPeriodicEffectType::DOT;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Periodic")
	float TickMagnitude = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Periodic")
	float TickInterval = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Periodic")
	float AppliedWorldTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Periodic")
	float ExpirationWorldTime = 0.0f;
};
