// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Debug/MABSDebugHUD.h"
#include "MABSDemoHUD.generated.h"

class AMABSCharacter;
class UMABSAbilityComponent;
class UMABSDemoDisplayConfig;

UCLASS(Blueprintable)
class MABS_API AMABSDemoHUD : public AMABSDebugHUD
{
	GENERATED_BODY()

public:

	AMABSDemoHUD();

	virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable, Category="MABS|Demo")
	void SetDemoOverlayEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="MABS|Demo")
	void ToggleDemoOverlayEnabled();

	UFUNCTION(BlueprintPure, Category="MABS|Demo")
	bool IsDemoOverlayEnabled() const;

	UFUNCTION(BlueprintCallable, Category="MABS|Demo")
	void SetHelpPanelEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="MABS|Demo")
	void ToggleHelpPanelEnabled();

	UFUNCTION(BlueprintPure, Category="MABS|Demo")
	bool IsHelpPanelEnabled() const;

	UFUNCTION(BlueprintCallable, Category="MABS|Demo")
	void SetValidationPanelEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="MABS|Demo")
	void ToggleValidationPanelEnabled();

	UFUNCTION(BlueprintPure, Category="MABS|Demo")
	bool IsValidationPanelEnabled() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Demo")
	bool bDemoOverlayEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Demo")
	bool bHelpPanelEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Demo")
	bool bValidationPanelEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MABS|Demo")
	TObjectPtr<UMABSDemoDisplayConfig> DemoDisplayConfig = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Demo")
	FVector2D DemoOverlayMargin = FVector2D(24.0f, 24.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Demo")
	FVector2D DemoOverlaySize = FVector2D(940.0f, 520.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Demo")
	FLinearColor DemoBackgroundColor = FLinearColor(0.06f, 0.08f, 0.09f, 0.86f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Demo")
	FLinearColor DemoTitleColor = FLinearColor(0.95f, 0.96f, 0.93f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Demo")
	FLinearColor DemoPanelColor = FLinearColor(0.10f, 0.13f, 0.15f, 0.92f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Demo")
	FLinearColor DemoMutedTextColor = FLinearColor(0.78f, 0.80f, 0.78f, 1.0f);

private:

	void DrawDemoOverlay(UMABSAbilityComponent& AbilityComponent);

	UMABSAbilityComponent* ResolveDemoAbilityComponent() const;

	AMABSCharacter* ResolveExampleCharacter() const;
};
