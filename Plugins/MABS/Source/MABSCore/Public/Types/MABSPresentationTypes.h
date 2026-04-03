// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "MABSAbilityTypes.h"
#include "MABSPresentationTypes.generated.h"

class UNiagaraSystem;
class USoundBase;

UENUM(BlueprintType)
enum class EMABSPresentationCueVisibilityPolicy : uint8
{
	RelevantClients UMETA(DisplayName="Relevant Clients"),
	OwnerOnly UMETA(DisplayName="Owner Only"),
	LocalOnly UMETA(DisplayName="Local Only")
};

UENUM(BlueprintType)
enum class EMABSPresentationCuePhase : uint8
{
	Startup UMETA(DisplayName="Startup"),
	Delivery UMETA(DisplayName="Delivery"),
	Tracer UMETA(DisplayName="Tracer"),
	ProjectileTravel UMETA(DisplayName="Projectile Travel"),
	Impact UMETA(DisplayName="Impact")
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSPresentationCameraShakeData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	TSubclassOf<UCameraShakeBase> CameraShakeClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation", meta=(ClampMin="0.0", EditCondition="CameraShakeClass != nullptr", EditConditionHides))
	float InnerRadius = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation", meta=(ClampMin="0.0", EditCondition="CameraShakeClass != nullptr", EditConditionHides))
	float OuterRadius = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation", meta=(ClampMin="0.0", EditCondition="CameraShakeClass != nullptr", EditConditionHides))
	float Falloff = 1.0f;

	bool HasCameraShake() const
	{
		return CameraShakeClass != nullptr;
	}
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSPresentationCueData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<UNiagaraSystem> VFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<USoundBase> SFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	EMABSPresentationCueVisibilityPolicy VisibilityPolicy = EMABSPresentationCueVisibilityPolicy::RelevantClients;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	FMABSPresentationCameraShakeData CameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	FName SocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	FRotator RotationOffset = FRotator::ZeroRotator;

	bool HasAnyPresentation() const
	{
		return VFX != nullptr || SFX != nullptr || CameraShake.HasCameraShake();
	}
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSHitTraceTracerPresentationData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<UNiagaraSystem> TracerVFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<USoundBase> TracerSFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	EMABSPresentationCueVisibilityPolicy VisibilityPolicy = EMABSPresentationCueVisibilityPolicy::RelevantClients;

	bool HasAnyPresentation() const
	{
		return TracerVFX != nullptr || TracerSFX != nullptr;
	}
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSProjectileTravelPresentationData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<UNiagaraSystem> TravelVFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<USoundBase> TravelSFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	EMABSPresentationCueVisibilityPolicy VisibilityPolicy = EMABSPresentationCueVisibilityPolicy::RelevantClients;

	bool HasAnyPresentation() const
	{
		return TravelVFX != nullptr || TravelSFX != nullptr;
	}
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSPresentationCueEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	FGameplayTag AbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	FMABSAbilityHandle AbilityHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	EMABSAbilityRuntimeState RuntimeState = EMABSAbilityRuntimeState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	EMABSPresentationCuePhase Phase = EMABSPresentationCuePhase::Startup;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	EMABSPresentationCueVisibilityPolicy VisibilityPolicy = EMABSPresentationCueVisibilityPolicy::RelevantClients;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<UNiagaraSystem> VFX = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<USoundBase> SFX = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	TSubclassOf<UCameraShakeBase> CameraShakeClass = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	float CameraShakeInnerRadius = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	float CameraShakeOuterRadius = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	float CameraShakeFalloff = 1.0f;

	bool HasAnyPresentation() const
	{
		return VFX != nullptr || SFX != nullptr || CameraShakeClass != nullptr;
	}
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSTracerCueEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	FGameplayTag AbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	FMABSAbilityHandle AbilityHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	EMABSAbilityRuntimeState RuntimeState = EMABSAbilityRuntimeState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	EMABSPresentationCueVisibilityPolicy VisibilityPolicy = EMABSPresentationCueVisibilityPolicy::RelevantClients;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<UNiagaraSystem> VFX = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<USoundBase> SFX = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	FVector TraceStart = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	FVector TraceEnd = FVector::ZeroVector;

	bool HasAnyPresentation() const
	{
		return VFX != nullptr || SFX != nullptr;
	}
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSProjectileTravelCueEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	FGameplayTag AbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	FMABSAbilityHandle AbilityHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	EMABSAbilityRuntimeState RuntimeState = EMABSAbilityRuntimeState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	EMABSPresentationCueVisibilityPolicy VisibilityPolicy = EMABSPresentationCueVisibilityPolicy::RelevantClients;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<UNiagaraSystem> VFX = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<USoundBase> SFX = nullptr;

	bool HasAnyPresentation() const
	{
		return VFX != nullptr || SFX != nullptr;
	}
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSPresentationStartupData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	FMABSPresentationCueData Cue;

	bool HasAnyPresentation() const
	{
		return Cue.HasAnyPresentation();
	}
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSPresentationDeliveryData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	FMABSPresentationCueData Cue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	FMABSHitTraceTracerPresentationData HitTraceTracer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	FMABSProjectileTravelPresentationData ProjectileTravel;

	bool HasAnyPresentation() const
	{
		return Cue.HasAnyPresentation()
			|| HitTraceTracer.HasAnyPresentation()
			|| ProjectileTravel.HasAnyPresentation();
	}
};

USTRUCT(BlueprintType)
struct MABSCORE_API FMABSPresentationImpactData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	FMABSPresentationCueData Cue;

	bool HasAnyPresentation() const
	{
		return Cue.HasAnyPresentation();
	}
};
