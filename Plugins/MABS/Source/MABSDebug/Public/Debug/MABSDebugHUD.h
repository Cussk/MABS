// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	bool bOverlayEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug", meta=(ClampMin="1"))
	int32 MaxDisplayedEvents = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug", meta=(ClampMin="1"))
	int32 MaxDisplayedAbilities = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	FVector2D OverlayPosition = FVector2D(24.0f, 24.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	FVector2D OverlaySize = FVector2D(900.0f, 340.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	FLinearColor OverlayBackgroundColor = FLinearColor(0.03f, 0.04f, 0.05f, 0.72f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MABS|Debug")
	FLinearColor OverlayTitleColor = FLinearColor(0.9f, 0.92f, 0.95f, 1.0f);

private:

	UMABSAbilityComponent* ResolveAbilityComponent() const;
};
