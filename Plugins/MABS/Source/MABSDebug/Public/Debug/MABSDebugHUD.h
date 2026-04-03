// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Debug/MABSAbilityDebugTypes.h"
#include "GameFramework/HUD.h"
#include "MABSDebugHUD.generated.h"

class UMABSAbilityComponent;

UCLASS(Blueprintable)
class MABSDEBUG_API AMABSDebugHUD : public AHUD
{
	GENERATED_BODY()

public:

	AMABSDebugHUD();

	virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable, Category="MABS|Debug")
	void SetOverlayEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="MABS|Debug")
	void ToggleOverlayEnabled();

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	bool IsOverlayEnabled() const;

	UFUNCTION(BlueprintCallable, Category="MABS|Debug")
	void SetCategoryEnabled(EMABSDebugEventCategory Category, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="MABS|Debug")
	void ToggleCategoryEnabled(EMABSDebugEventCategory Category);

	UFUNCTION(BlueprintPure, Category="MABS|Debug")
	bool IsCategoryEnabled(EMABSDebugEventCategory Category) const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	bool bOverlayEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug", meta=(ClampMin="1"))
	int32 MaxDisplayedEvents = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug", meta=(ClampMin="1"))
	int32 MaxDisplayedAbilities = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug", meta=(ClampMin="1"))
	int32 MaxDisplayedCooldownGroups = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug", meta=(ClampMin="1"))
	int32 MaxDisplayedPeriodicEffects = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	FVector2D OverlayPosition = FVector2D(24.0f, 24.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	FVector2D OverlaySize = FVector2D(1320.0f, 760.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	FLinearColor OverlayBackgroundColor = FLinearColor(0.03f, 0.04f, 0.05f, 0.84f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	FLinearColor OverlayTitleColor = FLinearColor(0.9f, 0.92f, 0.95f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	FLinearColor SectionBackgroundColor = FLinearColor(0.06f, 0.08f, 0.1f, 0.88f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	bool bShowGeneralCategory = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	bool bShowActivationCategory = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	bool bShowTargetingCategory = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	bool bShowDeliveryCategory = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	bool bShowCostCooldownCategory = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	bool bShowComboCategory = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	bool bShowPeriodicCategory = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	bool bShowPresentationCategory = true;

private:

	bool IsCategoryFlagEnabled(EMABSDebugEventCategory Category) const;
	bool IsConsoleCategoryEnabled(EMABSDebugEventCategory Category) const;
	void SetCategoryFlagEnabled(EMABSDebugEventCategory Category, bool bEnabled);
	void SetConsoleCategoryEnabled(EMABSDebugEventCategory Category, bool bEnabled) const;

	UMABSAbilityComponent* ResolveAbilityComponent() const;
};
