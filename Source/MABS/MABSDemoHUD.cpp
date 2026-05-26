// Copyright Epic Games, Inc. All Rights Reserved.

#include "MABSDemoHUD.h"

#include "CanvasItem.h"
#include "Components/MABSAbilityComponent.h"
#include "Debug/MABSDebugBlueprintLibrary.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "MABSCharacter.h"
#include "MABSDemoDisplayConfig.h"

namespace
{
	static TAutoConsoleVariable<int32> CVarMABSDemoHUD(
		TEXT("mabs.DemoHUD"),
		1,
		TEXT("Enables the readable MABS sample demo HUD."));

	static TAutoConsoleVariable<int32> CVarMABSDemoHUDHelp(
		TEXT("mabs.DemoHUD.Help"),
		1,
		TEXT("Shows the MABS sample help panel."));

	static TAutoConsoleVariable<int32> CVarMABSDemoHUDValidation(
		TEXT("mabs.DemoHUD.Validation"),
		0,
		TEXT("Shows the MABS sample validation notes panel."));

	struct FMABSDemoHUDLine
	{
		FString Text;
		FLinearColor Color = FLinearColor::White;
	};

	struct FMABSDemoHUDSection
	{
		FString Title;
		FLinearColor AccentColor = FLinearColor::White;
		TArray<FMABSDemoHUDLine> Lines;
	};

	FString TrimLine(const FString& Text, const int32 MaxCharacters)
	{
		if (Text.Len() <= MaxCharacters)
		{
			return Text;
		}

		return Text.Left(FMath::Max(0, MaxCharacters - 3)) + TEXT("...");
	}

	void AddSectionLine(
		FMABSDemoHUDSection& Section,
		const FString& Text,
		const FLinearColor& Color)
	{
		Section.Lines.Add({TrimLine(Text, 120), Color});
	}

	float DrawSection(
		UCanvas* Canvas,
		UFont* Font,
		const FVector2D& Position,
		const float Width,
		const FLinearColor& BackgroundColor,
		const FLinearColor& TitleColor,
		const FMABSDemoHUDSection& Section)
	{
		if (Canvas == nullptr || Font == nullptr || Section.Lines.IsEmpty())
		{
			return 0.0f;
		}

		const float TitleHeight = 20.0f;
		const float LineHeight = 15.0f;
		const float HorizontalPadding = 10.0f;
		const float VerticalPadding = 8.0f;
		const float Height = TitleHeight + (Section.Lines.Num() * LineHeight) + (VerticalPadding * 2.0f);

		FCanvasTileItem BackgroundTile(Position, FVector2D(Width, Height), BackgroundColor);
		BackgroundTile.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(BackgroundTile);

		FCanvasTileItem AccentTile(Position, FVector2D(Width, 2.0f), Section.AccentColor);
		AccentTile.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(AccentTile);

		FCanvasTextItem TitleItem(
			FVector2D(Position.X + HorizontalPadding, Position.Y + 4.0f),
			FText::FromString(Section.Title),
			Font,
			TitleColor);
		TitleItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(TitleItem);

		float CurrentY = Position.Y + TitleHeight + VerticalPadding - 2.0f;
		for (const FMABSDemoHUDLine& Line : Section.Lines)
		{
			FCanvasTextItem LineItem(
				FVector2D(Position.X + HorizontalPadding, CurrentY),
				FText::FromString(Line.Text),
				Font,
				Line.Color);
			LineItem.EnableShadow(FLinearColor::Black);
			Canvas->DrawItem(LineItem);
			CurrentY += LineHeight;
		}

		return Height;
	}

	template <typename PredicateType>
	const FMABSAbilityDebugEvent* FindLatestEvent(
		const TArray<FMABSAbilityDebugEvent>& Events,
		PredicateType&& Predicate)
	{
		for (int32 EventIndex = Events.Num() - 1; EventIndex >= 0; --EventIndex)
		{
			if (Predicate(Events[EventIndex]))
			{
				return &Events[EventIndex];
			}
		}

		return nullptr;
	}

	const FMABSGrantedAbilityDebugSummary* FindAbilitySummary(
		const TArray<FMABSGrantedAbilityDebugSummary>& Summaries,
		const FGameplayTag& AbilityTag)
	{
		for (const FMABSGrantedAbilityDebugSummary& Summary : Summaries)
		{
			if (Summary.AbilityTag == AbilityTag)
			{
				return &Summary;
			}
		}

		return nullptr;
	}

	const FMABSDemoAbilityDisplayEntry* FindDisplayEntry(
		const UMABSDemoDisplayConfig* DemoDisplayConfig,
		const FGameplayTag& AbilityTag)
	{
		if (DemoDisplayConfig == nullptr)
		{
			return nullptr;
		}

		for (const FMABSDemoAbilityDisplayEntry& Entry : DemoDisplayConfig->AbilityBarEntries)
		{
			if (Entry.AbilityTag == AbilityTag)
			{
				return &Entry;
			}
		}

		return nullptr;
	}

	FString GetAbilityLabel(
		const FMABSGrantedAbilityDebugSummary* AbilitySummary,
		const FMABSDemoAbilityDisplayEntry* DisplayEntry)
	{
		if (DisplayEntry != nullptr && !DisplayEntry->AbilityLabel.IsEmpty())
		{
			return DisplayEntry->AbilityLabel.ToString();
		}

		if (AbilitySummary != nullptr && !AbilitySummary->DisplayName.IsEmpty())
		{
			return AbilitySummary->DisplayName.ToString();
		}

		return AbilitySummary != nullptr && AbilitySummary->AbilityTag.IsValid()
			? AbilitySummary->AbilityTag.ToString()
			: TEXT("UnknownAbility");
	}

	FString GetAbilityInputLabel(const FMABSDemoAbilityDisplayEntry* DisplayEntry, const int32 Index)
	{
		if (DisplayEntry != nullptr && !DisplayEntry->InputLabel.IsEmpty())
		{
			return DisplayEntry->InputLabel.ToString();
		}

		return FString::Printf(TEXT("%d"), Index + 1);
	}

	FString GetAbilityStateLabel(const FMABSGrantedAbilityDebugSummary& AbilitySummary)
	{
		if (AbilitySummary.bIsBlocked)
		{
			return TEXT("Blocked");
		}

		if (AbilitySummary.RuntimeState == EMABSAbilityRuntimeState::Startup)
		{
			return AbilitySummary.DeliveryRemainingSeconds > 0.0f
				? FString::Printf(TEXT("Startup %.1fs"), AbilitySummary.DeliveryRemainingSeconds)
				: TEXT("Startup");
		}

		if (AbilitySummary.RuntimeState == EMABSAbilityRuntimeState::Recovery)
		{
			return FString::Printf(TEXT("Recovery %.1fs"), AbilitySummary.RecoveryRemainingSeconds);
		}

		if (AbilitySummary.RuntimeState == EMABSAbilityRuntimeState::Active)
		{
			return TEXT("Active");
		}

		if (AbilitySummary.CooldownRemainingSeconds > 0.0f)
		{
			return FString::Printf(TEXT("Cooldown %.1fs"), AbilitySummary.CooldownRemainingSeconds);
		}

		return TEXT("Ready");
	}

	FLinearColor GetAbilityColor(
		const FMABSGrantedAbilityDebugSummary* AbilitySummary,
		const FMABSDemoAbilityDisplayEntry* DisplayEntry,
		const FLinearColor& FallbackColor)
	{
		if (AbilitySummary != nullptr)
		{
			if (AbilitySummary->RuntimeState != EMABSAbilityRuntimeState::Idle
				|| AbilitySummary->CooldownRemainingSeconds > 0.0f
				|| AbilitySummary->bIsBlocked)
			{
				return UMABSDebugBlueprintLibrary::GetGrantedAbilityDebugSummaryColor(*AbilitySummary);
			}
		}

		if (DisplayEntry != nullptr)
		{
			return DisplayEntry->AccentColor;
		}

		return FallbackColor;
	}

	FString GetValueLine(const TCHAR* Label, const float CurrentValue, const float MaxValue)
	{
		return FString::Printf(TEXT("%s %.0f / %.0f"), Label, CurrentValue, MaxValue);
	}
}

AMABSDemoHUD::AMABSDemoHUD()
{
	bOverlayEnabled = false;
}

void AMABSDemoHUD::DrawHUD()
{
	if (Canvas != nullptr
		&& IsValid(PlayerOwner)
		&& PlayerOwner->IsLocalPlayerController()
		&& IsDemoOverlayEnabled())
	{
		if (UMABSAbilityComponent* const AbilityComponent = ResolveDemoAbilityComponent())
		{
			DrawDemoOverlay(*AbilityComponent);
		}
	}

	Super::DrawHUD();
}

void AMABSDemoHUD::SetDemoOverlayEnabled(const bool bEnabled)
{
	bDemoOverlayEnabled = bEnabled;

	if (IConsoleVariable* const ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(TEXT("mabs.DemoHUD")))
	{
		ConsoleVariable->Set(bEnabled ? 1 : 0);
	}
}

void AMABSDemoHUD::ToggleDemoOverlayEnabled()
{
	SetDemoOverlayEnabled(!IsDemoOverlayEnabled());
}

bool AMABSDemoHUD::IsDemoOverlayEnabled() const
{
	return bDemoOverlayEnabled && CVarMABSDemoHUD.GetValueOnGameThread() != 0;
}

void AMABSDemoHUD::SetHelpPanelEnabled(const bool bEnabled)
{
	bHelpPanelEnabled = bEnabled;

	if (IConsoleVariable* const ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(TEXT("mabs.DemoHUD.Help")))
	{
		ConsoleVariable->Set(bEnabled ? 1 : 0);
	}
}

void AMABSDemoHUD::ToggleHelpPanelEnabled()
{
	SetHelpPanelEnabled(!IsHelpPanelEnabled());
}

bool AMABSDemoHUD::IsHelpPanelEnabled() const
{
	return bHelpPanelEnabled && CVarMABSDemoHUDHelp.GetValueOnGameThread() != 0;
}

void AMABSDemoHUD::SetValidationPanelEnabled(const bool bEnabled)
{
	bValidationPanelEnabled = bEnabled;

	if (IConsoleVariable* const ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(TEXT("mabs.DemoHUD.Validation")))
	{
		ConsoleVariable->Set(bEnabled ? 1 : 0);
	}
}

void AMABSDemoHUD::ToggleValidationPanelEnabled()
{
	SetValidationPanelEnabled(!IsValidationPanelEnabled());
}

bool AMABSDemoHUD::IsValidationPanelEnabled() const
{
	return bValidationPanelEnabled && CVarMABSDemoHUDValidation.GetValueOnGameThread() != 0;
}

void AMABSDemoHUD::DrawDemoOverlay(UMABSAbilityComponent& AbilityComponent)
{
	UFont* const OverlayFont = GEngine != nullptr ? GEngine->GetSmallFont() : nullptr;
	if (Canvas == nullptr || OverlayFont == nullptr)
	{
		return;
	}

	const FVector2D DrawSize(
		FMath::Min(DemoOverlaySize.X, Canvas->SizeX - (DemoOverlayMargin.X * 2.0f)),
		FMath::Min(DemoOverlaySize.Y, Canvas->SizeY - (DemoOverlayMargin.Y * 2.0f)));
	const FVector2D DemoDrawPosition(
		Canvas->SizeX - DrawSize.X - DemoOverlayMargin.X,
		Canvas->SizeY - DrawSize.Y - DemoOverlayMargin.Y);

	FCanvasTileItem BackgroundTile(DemoDrawPosition, DrawSize, DemoBackgroundColor);
	BackgroundTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BackgroundTile);

	const TArray<FMABSGrantedAbilityDebugSummary> GrantedAbilitySummaries = AbilityComponent.GetGrantedAbilityDebugSummaries();
	const FMABSComboDebugSummary ComboSummary = AbilityComponent.GetComboDebugSummary();
	const FMABSTargetTraceDebugInfo TraceInfo = AbilityComponent.GetLatestTargetTraceDebugInfo();
	const TArray<FMABSPeriodicEffectDebugSummary> PeriodicSummaries = AbilityComponent.GetPeriodicEffectDebugSummaries();
	const TArray<FMABSAbilityDebugEvent> RecentEvents = AbilityComponent.GetRecentDebugEvents();
	const AMABSCharacter* const ExampleCharacter = ResolveExampleCharacter();

	const FText OverlayTitle = DemoDisplayConfig != nullptr && !DemoDisplayConfig->OverlayTitle.IsEmpty()
		? DemoDisplayConfig->OverlayTitle
		: FText::FromString(TEXT("MABS Sample Scene"));
	const FText OverlaySubtitle = DemoDisplayConfig != nullptr && !DemoDisplayConfig->OverlaySubtitle.IsEmpty()
		? DemoDisplayConfig->OverlaySubtitle
		: FText::FromString(TEXT("Readable demo layer for Phase 13. F1 Help | F2 Debug Harness | F3 Validation"));

	const float HeaderX = DemoDrawPosition.X + 16.0f;
	float HeaderY = DemoDrawPosition.Y + 10.0f;

	FCanvasTextItem TitleItem(FVector2D(HeaderX, HeaderY), OverlayTitle, OverlayFont, DemoTitleColor);
	TitleItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TitleItem);
	HeaderY += 18.0f;

	FCanvasTextItem SubtitleItem(FVector2D(HeaderX, HeaderY), OverlaySubtitle, OverlayFont, DemoMutedTextColor);
	SubtitleItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(SubtitleItem);

	FMABSDemoHUDSection AbilityBarSection;
	AbilityBarSection.Title = TEXT("Ability Bar");
	AbilityBarSection.AccentColor = FLinearColor(0.83f, 0.74f, 0.39f, 1.0f);

	TArray<FMABSDemoAbilityDisplayEntry> ConfiguredEntries = DemoDisplayConfig != nullptr
		? DemoDisplayConfig->AbilityBarEntries
		: TArray<FMABSDemoAbilityDisplayEntry>();
	ConfiguredEntries.Sort([](const FMABSDemoAbilityDisplayEntry& Left, const FMABSDemoAbilityDisplayEntry& Right)
	{
		return Left.SortOrder < Right.SortOrder;
	});

	if (ConfiguredEntries.IsEmpty())
	{
		const int32 MaxAbilityLines = FMath::Min(6, GrantedAbilitySummaries.Num());
		for (int32 AbilityIndex = 0; AbilityIndex < MaxAbilityLines; ++AbilityIndex)
		{
			const FMABSGrantedAbilityDebugSummary& Summary = GrantedAbilitySummaries[AbilityIndex];
			const FString AbilityLine = FString::Printf(
				TEXT("[%d] %s | %s | Cost %.0f | %s"),
				AbilityIndex + 1,
				*GetAbilityLabel(&Summary, nullptr),
				*GetAbilityStateLabel(Summary),
				Summary.ResourceCost,
				*UMABSDebugBlueprintLibrary::GetDebugEventCategoryLabel(EMABSDebugEventCategory::Delivery));
			AddSectionLine(
				AbilityBarSection,
				AbilityLine,
				UMABSDebugBlueprintLibrary::GetGrantedAbilityDebugSummaryColor(Summary));
		}
	}
	else
	{
		for (int32 EntryIndex = 0; EntryIndex < ConfiguredEntries.Num(); ++EntryIndex)
		{
			const FMABSDemoAbilityDisplayEntry& Entry = ConfiguredEntries[EntryIndex];
			const FMABSGrantedAbilityDebugSummary* const AbilitySummary = FindAbilitySummary(GrantedAbilitySummaries, Entry.AbilityTag);
			if (AbilitySummary == nullptr)
			{
				AddSectionLine(
					AbilityBarSection,
					FString::Printf(
						TEXT("[%s] %s | Not Granted"),
						*GetAbilityInputLabel(&Entry, EntryIndex),
						*GetAbilityLabel(nullptr, &Entry)),
					FLinearColor(0.65f, 0.67f, 0.69f, 1.0f));
				continue;
			}

			const FString AbilityLine = FString::Printf(
				TEXT("[%s] %s | %s | Cost %.0f | %s"),
				*GetAbilityInputLabel(&Entry, EntryIndex),
				*GetAbilityLabel(AbilitySummary, &Entry),
				*GetAbilityStateLabel(*AbilitySummary),
				AbilitySummary->ResourceCost,
				*TrimLine(Entry.FeatureSummary.ToString(), 34));
			AddSectionLine(
				AbilityBarSection,
				AbilityLine,
				GetAbilityColor(AbilitySummary, &Entry, DemoTitleColor));
		}
	}

	if (AbilityBarSection.Lines.IsEmpty())
	{
		AddSectionLine(AbilityBarSection, TEXT("Grant abilities on the sample pawn to populate the hotbar."), DemoMutedTextColor);
	}

	FMABSDemoHUDSection StatusSection;
	StatusSection.Title = TEXT("Status");
	StatusSection.AccentColor = FLinearColor(0.28f, 0.74f, 0.48f, 1.0f);
	if (ExampleCharacter != nullptr)
	{
		AddSectionLine(
			StatusSection,
			GetValueLine(TEXT("Health"), ExampleCharacter->GetExampleHealth(), ExampleCharacter->GetMaxExampleHealth()),
			FLinearColor(0.77f, 0.93f, 0.81f, 1.0f));
		AddSectionLine(
			StatusSection,
			GetValueLine(TEXT("Resource"), ExampleCharacter->GetExampleResource(), ExampleCharacter->GetMaxExampleResource()),
			FLinearColor(0.76f, 0.89f, 0.98f, 1.0f));
	}
	else
	{
		AddSectionLine(StatusSection, TEXT("No sample character found."), DemoMutedTextColor);
	}

	AddSectionLine(
		StatusSection,
		ComboSummary.bHasActiveComboState
			? FString::Printf(
				TEXT("Combo %s | Next %s"),
				ComboSummary.bComboWindowOpen ? TEXT("Window Open") : TEXT("Queued"),
				ComboSummary.NextComboAbilityTag.IsValid() ? *ComboSummary.NextComboAbilityTag.ToString() : TEXT("None"))
			: TEXT("Combo Closed"),
		ComboSummary.bComboWindowOpen
			? FLinearColor(0.49f, 0.77f, 1.0f, 1.0f)
			: DemoMutedTextColor);

	AddSectionLine(
		StatusSection,
		FString::Printf(TEXT("Periodic Effects %d"), PeriodicSummaries.Num()),
		PeriodicSummaries.IsEmpty()
			? DemoMutedTextColor
			: FLinearColor(0.47f, 0.92f, 0.55f, 1.0f));

	const FMABSAbilityDebugEvent* const LatestActivationEvent = FindLatestEvent(
		RecentEvents,
		[](const FMABSAbilityDebugEvent& DebugEvent)
		{
			return DebugEvent.Category == EMABSDebugEventCategory::Activation;
		});
	if (LatestActivationEvent != nullptr)
	{
		AddSectionLine(
			StatusSection,
			FString::Printf(TEXT("Last Activation: %s"), *UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(*LatestActivationEvent)),
			UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(*LatestActivationEvent));
	}

	const FMABSAbilityDebugEvent* const LatestDeliveryEvent = FindLatestEvent(
		RecentEvents,
		[](const FMABSAbilityDebugEvent& DebugEvent)
		{
			return DebugEvent.Category == EMABSDebugEventCategory::Delivery;
		});
	const FMABSAbilityDebugEvent* const LatestEffectEvent = FindLatestEvent(
		RecentEvents,
		[](const FMABSAbilityDebugEvent& DebugEvent)
		{
			return DebugEvent.EventName == TEXT("EffectApplied")
				|| DebugEvent.EventName == TEXT("ProjectileImpact")
				|| DebugEvent.EventName == TEXT("PeriodicEffectApplied")
				|| DebugEvent.EventName == TEXT("PeriodicEffectTick");
		});
	const FMABSAbilityDebugEvent* const LatestFeatureEvent = LatestEffectEvent != nullptr
		? LatestEffectEvent
		: LatestDeliveryEvent;
	const FMABSDemoAbilityDisplayEntry* const LatestFeatureEntry = LatestFeatureEvent != nullptr
		? FindDisplayEntry(DemoDisplayConfig, LatestFeatureEvent->AbilityTag)
		: nullptr;

	FMABSDemoHUDSection FeatureSection;
	FeatureSection.Title = TEXT("Feature Callout");
	FeatureSection.AccentColor = LatestFeatureEntry != nullptr
		? LatestFeatureEntry->AccentColor
		: FLinearColor(0.90f, 0.83f, 0.44f, 1.0f);
	if (LatestFeatureEntry != nullptr)
	{
		AddSectionLine(
			FeatureSection,
			GetAbilityLabel(nullptr, LatestFeatureEntry),
			FeatureSection.AccentColor);
		if (!LatestFeatureEntry->FeatureSummary.IsEmpty())
		{
			AddSectionLine(FeatureSection, LatestFeatureEntry->FeatureSummary.ToString(), DemoTitleColor);
		}
	}
	else
	{
		AddSectionLine(FeatureSection, TEXT("Trigger a sample ability to surface the current feature."), DemoMutedTextColor);
	}

	if (ComboSummary.bComboWindowOpen)
	{
		AddSectionLine(
			FeatureSection,
			FString::Printf(TEXT("Combo window open for %.1fs"), ComboSummary.ComboWindowRemainingSeconds),
			FLinearColor(0.49f, 0.77f, 1.0f, 1.0f));
	}

	if (!PeriodicSummaries.IsEmpty())
	{
		AddSectionLine(
			FeatureSection,
			FString::Printf(TEXT("Active periodic: %s"), *UMABSDebugBlueprintLibrary::FormatPeriodicEffectDebugSummary(PeriodicSummaries[0])),
			FLinearColor(0.47f, 0.92f, 0.55f, 1.0f));
	}

	if (LatestFeatureEvent != nullptr)
	{
		AddSectionLine(
			FeatureSection,
			FString::Printf(TEXT("Runtime: %s"), *UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(*LatestFeatureEvent)),
			UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(*LatestFeatureEvent));
	}

	FMABSDemoHUDSection TargetSection;
	TargetSection.Title = TEXT("Target / Hit Readout");
	TargetSection.AccentColor = FLinearColor(0.32f, 0.79f, 0.94f, 1.0f);
	AddSectionLine(
		TargetSection,
		FString::Printf(
			TEXT("Target: %s"),
			TraceInfo.bHasTraceData && !TraceInfo.HitActorName.IsEmpty() ? *TraceInfo.HitActorName : TEXT("None")),
		UMABSDebugBlueprintLibrary::GetTargetTraceDebugColor(TraceInfo));
	AddSectionLine(
		TargetSection,
		FString::Printf(TEXT("Trace: %s"), *UMABSDebugBlueprintLibrary::FormatTargetTraceDebugInfo(TraceInfo)),
		UMABSDebugBlueprintLibrary::GetTargetTraceDebugColor(TraceInfo));

	if (LatestEffectEvent != nullptr)
	{
		AddSectionLine(
			TargetSection,
			FString::Printf(TEXT("Last Effect: %s"), *UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(*LatestEffectEvent)),
			UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(*LatestEffectEvent));
	}

	if (LatestDeliveryEvent != nullptr)
	{
		AddSectionLine(
			TargetSection,
			FString::Printf(TEXT("Last Delivery: %s"), *UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(*LatestDeliveryEvent)),
			UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(*LatestDeliveryEvent));
	}

	FMABSDemoHUDSection HelpSection;
	HelpSection.Title = TEXT("Demo Help");
	HelpSection.AccentColor = FLinearColor(0.66f, 0.88f, 0.72f, 1.0f);
	AddSectionLine(HelpSection, TEXT("F1 Toggle Help | F2 Toggle Debug Harness | F3 Toggle Validation"), DemoTitleColor);
	if (DemoDisplayConfig != nullptr && !DemoDisplayConfig->HelpEntries.IsEmpty())
	{
		for (const FMABSDemoTextPanelEntry& Entry : DemoDisplayConfig->HelpEntries)
		{
			const FString HelpLine = Entry.Title.IsEmpty()
				? Entry.Body.ToString()
				: FString::Printf(TEXT("%s: %s"), *Entry.Title.ToString(), *Entry.Body.ToString());
			AddSectionLine(HelpSection, HelpLine, DemoMutedTextColor);
		}
	}
	else
	{
		AddSectionLine(HelpSection, TEXT("Open the sample map, trigger each hotbar ability, then inspect the matching station and target dummy."), DemoMutedTextColor);
		AddSectionLine(HelpSection, TEXT("Use the custom-handler station to compare built-in delivery against the knockback example."), DemoMutedTextColor);
	}

	FMABSDemoHUDSection ValidationSection;
	ValidationSection.Title = TEXT("Validation Notes");
	ValidationSection.AccentColor = FLinearColor(0.95f, 0.74f, 0.34f, 1.0f);
	if (DemoDisplayConfig != nullptr && !DemoDisplayConfig->ValidationEntries.IsEmpty())
	{
		for (const FMABSDemoTextPanelEntry& Entry : DemoDisplayConfig->ValidationEntries)
		{
			const FString ValidationLine = Entry.Title.IsEmpty()
				? Entry.Body.ToString()
				: FString::Printf(TEXT("%s: %s"), *Entry.Title.ToString(), *Entry.Body.ToString());
			AddSectionLine(ValidationSection, ValidationLine, DemoMutedTextColor);
		}
	}
	else
	{
		AddSectionLine(ValidationSection, TEXT("Run the map in standalone, listen server, and dedicated server flows."), DemoMutedTextColor);
		AddSectionLine(ValidationSection, TEXT("Verify readable cooldown, cost, combo, periodic, and custom-handler proof without opening logs first."), DemoMutedTextColor);
		AddSectionLine(ValidationSection, TEXT("Use editor validation before runtime if a sample ability or handler class is reauthored."), DemoMutedTextColor);
	}

	const float InnerPadding = 14.0f;
	const float ColumnGap = 16.0f;
	const float HeaderHeight = 54.0f;
	const float ContentTop = DemoDrawPosition.Y + HeaderHeight + 12.0f;
	const float ContentWidth = DrawSize.X - (InnerPadding * 2.0f);
	const float SectionSpacing = 10.0f;
	const float AbilitySectionWidth = ContentWidth;
	const float LeftColumnWidth = (ContentWidth - ColumnGap) * 0.54f;
	const float RightColumnWidth = ContentWidth - ColumnGap - LeftColumnWidth;
	const float LeftColumnX = DemoDrawPosition.X + InnerPadding;
	const float RightColumnX = LeftColumnX + LeftColumnWidth + ColumnGap;

	float LeftColumnY = ContentTop;
	LeftColumnY += DrawSection(
		Canvas,
		OverlayFont,
		FVector2D(LeftColumnX, LeftColumnY),
		AbilitySectionWidth,
		DemoPanelColor,
		DemoTitleColor,
		AbilityBarSection) + SectionSpacing;

	float SecondaryTop = LeftColumnY;

	float LeftSecondaryY = SecondaryTop;
	LeftSecondaryY += DrawSection(
		Canvas,
		OverlayFont,
		FVector2D(LeftColumnX, LeftSecondaryY),
		LeftColumnWidth,
		DemoPanelColor,
		DemoTitleColor,
		StatusSection) + SectionSpacing;
	LeftSecondaryY += DrawSection(
		Canvas,
		OverlayFont,
		FVector2D(LeftColumnX, LeftSecondaryY),
		LeftColumnWidth,
		DemoPanelColor,
		DemoTitleColor,
		FeatureSection) + SectionSpacing;

	float RightSecondaryY = SecondaryTop;
	RightSecondaryY += DrawSection(
		Canvas,
		OverlayFont,
		FVector2D(RightColumnX, RightSecondaryY),
		RightColumnWidth,
		DemoPanelColor,
		DemoTitleColor,
		TargetSection) + SectionSpacing;

	if (IsHelpPanelEnabled())
	{
		RightSecondaryY += DrawSection(
			Canvas,
			OverlayFont,
			FVector2D(RightColumnX, RightSecondaryY),
			RightColumnWidth,
			DemoPanelColor,
			DemoTitleColor,
			HelpSection) + SectionSpacing;
	}

	if (IsValidationPanelEnabled())
	{
		DrawSection(
			Canvas,
			OverlayFont,
			FVector2D(RightColumnX, RightSecondaryY),
			RightColumnWidth,
			DemoPanelColor,
			DemoTitleColor,
			ValidationSection);
	}
}

UMABSAbilityComponent* AMABSDemoHUD::ResolveDemoAbilityComponent() const
{
	AActor* DebugActor = nullptr;
	if (PlayerOwner != nullptr)
	{
		DebugActor = PlayerOwner->GetPawnOrSpectator();
		if (DebugActor == nullptr)
		{
			DebugActor = PlayerOwner->GetViewTarget();
		}
	}

	return DebugActor != nullptr ? DebugActor->FindComponentByClass<UMABSAbilityComponent>() : nullptr;
}

AMABSCharacter* AMABSDemoHUD::ResolveExampleCharacter() const
{
	return PlayerOwner != nullptr ? Cast<AMABSCharacter>(PlayerOwner->GetPawn()) : nullptr;
}
