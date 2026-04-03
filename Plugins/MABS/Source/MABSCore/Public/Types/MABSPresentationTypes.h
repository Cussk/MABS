// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "MABSPresentationTypes.generated.h"

class UParticleSystem;
class USoundBase;

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
	TObjectPtr<UParticleSystem> VFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<USoundBase> SFX = nullptr;

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
	TObjectPtr<UParticleSystem> TracerVFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<USoundBase> TracerSFX = nullptr;

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
	TObjectPtr<UParticleSystem> TravelVFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	TObjectPtr<USoundBase> TravelSFX = nullptr;

	bool HasAnyPresentation() const
	{
		return TravelVFX != nullptr || TravelSFX != nullptr;
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
