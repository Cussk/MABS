// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MABSAbilityTypes.generated.h"

class UMABSAbilityDefinition;

UENUM(BlueprintType)
enum class EMABSAbilityActivationResult : uint8
{
	Success UMETA(DisplayName="Success"),
	InvalidRequest UMETA(DisplayName="Invalid Request"),
	InvalidDefinition UMETA(DisplayName="Invalid Definition"),
	AbilityNotGranted UMETA(DisplayName="Ability Not Granted"),
	RequiresAuthority UMETA(DisplayName="Requires Authority"),
	NotImplemented UMETA(DisplayName="Not Implemented")
};

UENUM(BlueprintType)
enum class EMABSAbilityRuntimeState : uint8
{
	None UMETA(DisplayName="None"),
	Granted UMETA(DisplayName="Granted"),
	ActivationRequested UMETA(DisplayName="Activation Requested"),
	Active UMETA(DisplayName="Active"),
	Ended UMETA(DisplayName="Ended")
};

UENUM(BlueprintType)
enum class EMABSTargetType : uint8
{
	None UMETA(DisplayName="None"),
	Self UMETA(DisplayName="Self"),
	Actor UMETA(DisplayName="Actor"),
	Location UMETA(DisplayName="Location")
};

USTRUCT(BlueprintType)
struct MABS_API FMABSAbilityHandle
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
struct MABS_API FMABSAbilitySpec
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	FMABSAbilityHandle Handle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UMABSAbilityDefinition> AbilityDefinition = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	EMABSAbilityRuntimeState RuntimeState = EMABSAbilityRuntimeState::Granted;
};
