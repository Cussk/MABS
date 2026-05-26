// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MABSDemoDisplayConfig.generated.h"

USTRUCT(BlueprintType)
struct FMABSDemoAbilityDisplayEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Demo")
	FGameplayTag AbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Demo")
	FText AbilityLabel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Demo")
	FText InputLabel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Demo", meta=(MultiLine="true"))
	FText FeatureSummary;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Demo")
	FLinearColor AccentColor = FLinearColor(0.82f, 0.85f, 0.9f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Demo")
	int32 SortOrder = 0;
};

USTRUCT(BlueprintType)
struct FMABSDemoTextPanelEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Demo")
	FText Title;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Demo", meta=(MultiLine="true"))
	FText Body;
};

UCLASS(BlueprintType)
class MABS_API UMABSDemoDisplayConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Demo")
	FText OverlayTitle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Demo")
	FText OverlaySubtitle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Demo")
	TArray<FMABSDemoAbilityDisplayEntry> AbilityBarEntries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Demo")
	TArray<FMABSDemoTextPanelEntry> HelpEntries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Demo")
	TArray<FMABSDemoTextPanelEntry> ValidationEntries;
};
