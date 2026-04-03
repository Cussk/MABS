// Copyright Epic Games, Inc. All Rights Reserved.

#include "Debug/MABSDebugHUD.h"
#include "CanvasItem.h"
#include "Components/MABSAbilityComponent.h"
#include "Debug/MABSDebugBlueprintLibrary.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

namespace
{
	static TAutoConsoleVariable<int32> CVarMABSDebugHarness(
		TEXT("mabs.DebugHarness"),
		1,
		TEXT("Enables the MABS runtime debug harness."));

	static TAutoConsoleVariable<int32> CVarMABSDebugHarnessGeneral(
		TEXT("mabs.DebugHarness.General"),
		1,
		TEXT("Shows general MABS harness sections."));

	static TAutoConsoleVariable<int32> CVarMABSDebugHarnessActivation(
		TEXT("mabs.DebugHarness.Activation"),
		1,
		TEXT("Shows activation MABS harness sections."));

	static TAutoConsoleVariable<int32> CVarMABSDebugHarnessTargeting(
		TEXT("mabs.DebugHarness.Targeting"),
		1,
		TEXT("Shows targeting MABS harness sections."));

	static TAutoConsoleVariable<int32> CVarMABSDebugHarnessDelivery(
		TEXT("mabs.DebugHarness.Delivery"),
		1,
		TEXT("Shows delivery MABS harness sections."));

	static TAutoConsoleVariable<int32> CVarMABSDebugHarnessCostCooldown(
		TEXT("mabs.DebugHarness.CostCooldown"),
		1,
		TEXT("Shows cost and cooldown MABS harness sections."));

	static TAutoConsoleVariable<int32> CVarMABSDebugHarnessCombo(
		TEXT("mabs.DebugHarness.Combo"),
		1,
		TEXT("Shows combo MABS harness sections."));

	static TAutoConsoleVariable<int32> CVarMABSDebugHarnessPeriodic(
		TEXT("mabs.DebugHarness.Periodic"),
		1,
		TEXT("Shows periodic-effect MABS harness sections."));

	static TAutoConsoleVariable<int32> CVarMABSDebugHarnessPresentation(
		TEXT("mabs.DebugHarness.Presentation"),
		1,
		TEXT("Shows presentation and cue MABS harness sections."));

	struct FMABSDebugHarnessLine
	{
		FString Text;
		FLinearColor Color = FLinearColor::White;
	};

	struct FMABSDebugHarnessSection
	{
		FString Title;
		FLinearColor AccentColor = FLinearColor::White;
		TArray<FMABSDebugHarnessLine> Lines;
	};

	FString TrimDebugLine(const FString& Text, const int32 MaxCharacters)
	{
		if (Text.Len() <= MaxCharacters)
		{
			return Text;
		}

		return Text.Left(FMath::Max(0, MaxCharacters - 3)) + TEXT("...");
	}

	bool IsFailureActivationResult(const EMABSAbilityActivationResult Result)
	{
		switch (Result)
		{
		case EMABSAbilityActivationResult::InvalidAbility:
		case EMABSAbilityActivationResult::NotGranted:
		case EMABSAbilityActivationResult::AlreadyActive:
		case EMABSAbilityActivationResult::Blocked:
		case EMABSAbilityActivationResult::OnCooldown:
		case EMABSAbilityActivationResult::InsufficientResources:
		case EMABSAbilityActivationResult::ComboRejected:
		case EMABSAbilityActivationResult::AuthorityRejected:
		case EMABSAbilityActivationResult::DeliveryFailed:
		case EMABSAbilityActivationResult::TargetResolutionFailed:
		case EMABSAbilityActivationResult::EffectApplicationFailed:
			return true;

		default:
			return false;
		}
	}

	FString GetNetModeLabel(const UWorld* const World)
	{
		if (World == nullptr)
		{
			return TEXT("Unknown");
		}

		switch (World->GetNetMode())
		{
		case NM_Standalone:
			return TEXT("Standalone");

		case NM_ListenServer:
			return TEXT("ListenServer");

		case NM_DedicatedServer:
			return TEXT("DedicatedServer");

		case NM_Client:
			return TEXT("Client");

		default:
			return TEXT("Unknown");
		}
	}

	template <typename PredicateType>
	const FMABSAbilityDebugEvent* FindLatestDebugEvent(
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

	void AddSectionLine(
		FMABSDebugHarnessSection& Section,
		const FString& Text,
		const FLinearColor& Color)
	{
		Section.Lines.Add({TrimDebugLine(Text, 140), Color});
	}

	float DrawSection(
		UCanvas* const Canvas,
		UFont* const Font,
		const FVector2D& Position,
		const float Width,
		const FLinearColor& BackgroundColor,
		const FLinearColor& TitleColor,
		const FMABSDebugHarnessSection& Section)
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
		for (const FMABSDebugHarnessLine& Line : Section.Lines)
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
}

AMABSDebugHUD::AMABSDebugHUD()
{
}

void AMABSDebugHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!IsOverlayEnabled() || Canvas == nullptr || !IsValid(PlayerOwner) || !PlayerOwner->IsLocalPlayerController())
	{
		return;
	}

	UMABSAbilityComponent* const AbilityComponent = ResolveAbilityComponent();
	if (AbilityComponent == nullptr)
	{
		return;
	}

	UFont* const OverlayFont = GEngine != nullptr ? GEngine->GetSmallFont() : nullptr;
	if (OverlayFont == nullptr)
	{
		return;
	}

	const FVector2D DrawSize(
		FMath::Min(OverlaySize.X, Canvas->SizeX - (OverlayPosition.X * 2.0f)),
		FMath::Min(OverlaySize.Y, Canvas->SizeY - (OverlayPosition.Y * 2.0f)));

	FCanvasTileItem BackgroundTile(OverlayPosition, DrawSize, OverlayBackgroundColor);
	BackgroundTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BackgroundTile);

	const TArray<FMABSGrantedAbilityDebugSummary> GrantedAbilitySummaries = AbilityComponent->GetGrantedAbilityDebugSummaries();
	const TArray<FMABSCooldownGroupDebugSummary> CooldownSummaries = AbilityComponent->GetCooldownGroupDebugSummaries();
	const FMABSComboDebugSummary ComboSummary = AbilityComponent->GetComboDebugSummary();
	const FMABSTargetTraceDebugInfo TraceInfo = AbilityComponent->GetLatestTargetTraceDebugInfo();
	const TArray<FMABSPeriodicEffectDebugSummary> PeriodicSummaries = AbilityComponent->GetPeriodicEffectDebugSummaries();
	const TArray<FMABSAbilityDebugEvent> RecentEvents = AbilityComponent->GetRecentDebugEvents();

	auto IsEventEnabled = [this](const FMABSAbilityDebugEvent& DebugEvent)
	{
		return IsCategoryEnabled(DebugEvent.Category);
	};

	auto FindLatestEventByCategory = [&RecentEvents](const EMABSDebugEventCategory Category)
	{
		return FindLatestDebugEvent(
			RecentEvents,
			[Category](const FMABSAbilityDebugEvent& DebugEvent)
			{
				return DebugEvent.Category == Category;
			});
	};

	auto FindLatestEnabledFailureEvent = [this, &RecentEvents]()
	{
		return FindLatestDebugEvent(
			RecentEvents,
			[this](const FMABSAbilityDebugEvent& DebugEvent)
			{
				return IsCategoryEnabled(DebugEvent.Category) && IsFailureActivationResult(DebugEvent.ActivationResult);
			});
	};

	TArray<FString> EnabledCategoryLabels;
	static const EMABSDebugEventCategory OrderedCategories[] =
	{
		EMABSDebugEventCategory::General,
		EMABSDebugEventCategory::Activation,
		EMABSDebugEventCategory::Targeting,
		EMABSDebugEventCategory::Delivery,
		EMABSDebugEventCategory::CostCooldown,
		EMABSDebugEventCategory::Combo,
		EMABSDebugEventCategory::Periodic,
		EMABSDebugEventCategory::Presentation
	};
	for (const EMABSDebugEventCategory Category : OrderedCategories)
	{
		if (IsCategoryEnabled(Category))
		{
			EnabledCategoryLabels.Add(UMABSDebugBlueprintLibrary::GetDebugEventCategoryLabel(Category));
		}
	}

	const float HeaderX = OverlayPosition.X + 14.0f;
	float HeaderY = OverlayPosition.Y + 10.0f;
	FCanvasTextItem TitleItem(
		FVector2D(HeaderX, HeaderY),
		FText::FromString(TEXT("MABS Debug Harness")),
		OverlayFont,
		OverlayTitleColor);
	TitleItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TitleItem);
	HeaderY += 18.0f;

	FCanvasTextItem SubtitleItem(
		FVector2D(HeaderX, HeaderY),
		FText::FromString(FString::Printf(
			TEXT("Categories: %s"),
			EnabledCategoryLabels.IsEmpty() ? TEXT("None") : *FString::Join(EnabledCategoryLabels, TEXT(", ")))),
		OverlayFont,
		FLinearColor(0.72f, 0.76f, 0.82f, 1.0f));
	SubtitleItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(SubtitleItem);

	TArray<FMABSDebugHarnessSection> LeftSections;
	TArray<FMABSDebugHarnessSection> RightSections;

	if (IsCategoryEnabled(EMABSDebugEventCategory::General))
	{
		FMABSDebugHarnessSection StatusSection;
		StatusSection.Title = TEXT("Status");
		StatusSection.AccentColor = UMABSDebugBlueprintLibrary::GetDebugEventCategoryColor(EMABSDebugEventCategory::General);
		AddSectionLine(
			StatusSection,
			FString::Printf(
				TEXT("Owner=%s Net=%s DebugReplication=%s"),
				*GetNameSafe(AbilityComponent->GetOwner()),
				*GetNetModeLabel(GetWorld()),
				AbilityComponent->IsDebugReplicationEnabled() ? TEXT("true") : TEXT("false")),
			FLinearColor(0.82f, 0.85f, 0.9f, 1.0f));
		AddSectionLine(
			StatusSection,
			FString::Printf(
				TEXT("Granted=%d CooldownGroups=%d Periodic=%d RecentEvents=%d"),
				GrantedAbilitySummaries.Num(),
				CooldownSummaries.Num(),
				PeriodicSummaries.Num(),
				RecentEvents.Num()),
			FLinearColor(0.82f, 0.85f, 0.9f, 1.0f));
		LeftSections.Add(MoveTemp(StatusSection));

		FMABSDebugHarnessSection AbilitySection;
		AbilitySection.Title = TEXT("Granted Abilities");
		AbilitySection.AccentColor = UMABSDebugBlueprintLibrary::GetDebugEventCategoryColor(EMABSDebugEventCategory::General);
		const int32 AbilityCount = FMath::Min(MaxDisplayedAbilities, GrantedAbilitySummaries.Num());
		if (AbilityCount == 0)
		{
			AddSectionLine(AbilitySection, TEXT("No granted abilities."), FLinearColor(0.75f, 0.78f, 0.82f, 1.0f));
		}
		else
		{
			for (int32 AbilityIndex = 0; AbilityIndex < AbilityCount; ++AbilityIndex)
			{
				const FMABSGrantedAbilityDebugSummary& AbilitySummary = GrantedAbilitySummaries[AbilityIndex];
				AddSectionLine(
					AbilitySection,
					UMABSDebugBlueprintLibrary::FormatGrantedAbilityDebugSummary(AbilitySummary),
					UMABSDebugBlueprintLibrary::GetGrantedAbilityDebugSummaryColor(AbilitySummary));
			}
		}
		LeftSections.Add(MoveTemp(AbilitySection));
	}

	if (IsCategoryEnabled(EMABSDebugEventCategory::CostCooldown))
	{
		FMABSDebugHarnessSection CooldownSection;
		CooldownSection.Title = TEXT("Cooldowns / Costs");
		CooldownSection.AccentColor = UMABSDebugBlueprintLibrary::GetDebugEventCategoryColor(EMABSDebugEventCategory::CostCooldown);
		const int32 CooldownCount = FMath::Min(MaxDisplayedCooldownGroups, CooldownSummaries.Num());
		if (CooldownCount == 0)
		{
			AddSectionLine(CooldownSection, TEXT("No active cooldown groups."), FLinearColor(0.75f, 0.78f, 0.82f, 1.0f));
		}
		else
		{
			for (int32 CooldownIndex = 0; CooldownIndex < CooldownCount; ++CooldownIndex)
			{
				AddSectionLine(
					CooldownSection,
					UMABSDebugBlueprintLibrary::FormatCooldownGroupDebugSummary(CooldownSummaries[CooldownIndex]),
					FLinearColor(0.95f, 0.85f, 0.25f, 1.0f));
			}
		}

		if (const FMABSAbilityDebugEvent* const LatestCooldownEvent = FindLatestEventByCategory(EMABSDebugEventCategory::CostCooldown))
		{
			AddSectionLine(
				CooldownSection,
				FString::Printf(TEXT("Latest: %s"), *UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(*LatestCooldownEvent)),
				UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(*LatestCooldownEvent));
		}

		LeftSections.Add(MoveTemp(CooldownSection));
	}

	if (IsCategoryEnabled(EMABSDebugEventCategory::Combo))
	{
		FMABSDebugHarnessSection ComboSection;
		ComboSection.Title = TEXT("Combo State");
		ComboSection.AccentColor = UMABSDebugBlueprintLibrary::GetDebugEventCategoryColor(EMABSDebugEventCategory::Combo);
		AddSectionLine(
			ComboSection,
			UMABSDebugBlueprintLibrary::FormatComboDebugSummary(ComboSummary),
			UMABSDebugBlueprintLibrary::GetDebugEventCategoryColor(EMABSDebugEventCategory::Combo));

		if (const FMABSAbilityDebugEvent* const LatestComboEvent = FindLatestEventByCategory(EMABSDebugEventCategory::Combo))
		{
			AddSectionLine(
				ComboSection,
				FString::Printf(TEXT("Latest: %s"), *UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(*LatestComboEvent)),
				UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(*LatestComboEvent));
		}

		LeftSections.Add(MoveTemp(ComboSection));
	}

	if (IsCategoryEnabled(EMABSDebugEventCategory::Periodic))
	{
		FMABSDebugHarnessSection PeriodicSection;
		PeriodicSection.Title = TEXT("Periodic Effects");
		PeriodicSection.AccentColor = UMABSDebugBlueprintLibrary::GetDebugEventCategoryColor(EMABSDebugEventCategory::Periodic);
		const int32 PeriodicCount = FMath::Min(MaxDisplayedPeriodicEffects, PeriodicSummaries.Num());
		if (PeriodicCount == 0)
		{
			AddSectionLine(PeriodicSection, TEXT("No active periodic effects."), FLinearColor(0.75f, 0.78f, 0.82f, 1.0f));
		}
		else
		{
			for (int32 PeriodicIndex = 0; PeriodicIndex < PeriodicCount; ++PeriodicIndex)
			{
				AddSectionLine(
					PeriodicSection,
					UMABSDebugBlueprintLibrary::FormatPeriodicEffectDebugSummary(PeriodicSummaries[PeriodicIndex]),
					FLinearColor(0.45f, 0.9f, 0.48f, 1.0f));
			}
		}

		if (const FMABSAbilityDebugEvent* const LatestPeriodicEvent = FindLatestEventByCategory(EMABSDebugEventCategory::Periodic))
		{
			AddSectionLine(
				PeriodicSection,
				FString::Printf(TEXT("Latest: %s"), *UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(*LatestPeriodicEvent)),
				UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(*LatestPeriodicEvent));
		}

		LeftSections.Add(MoveTemp(PeriodicSection));
	}

	if (IsCategoryEnabled(EMABSDebugEventCategory::Activation))
	{
		FMABSDebugHarnessSection ActivationSection;
		ActivationSection.Title = TEXT("Latest Activation");
		ActivationSection.AccentColor = UMABSDebugBlueprintLibrary::GetDebugEventCategoryColor(EMABSDebugEventCategory::Activation);

		if (const FMABSAbilityDebugEvent* const LatestActivationEvent = FindLatestEventByCategory(EMABSDebugEventCategory::Activation))
		{
			AddSectionLine(
				ActivationSection,
				FString::Printf(TEXT("Latest: %s"), *UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(*LatestActivationEvent)),
				UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(*LatestActivationEvent));
		}
		else
		{
			AddSectionLine(ActivationSection, TEXT("No recent activation request or startup event."), FLinearColor(0.75f, 0.78f, 0.82f, 1.0f));
		}

		if (const FMABSAbilityDebugEvent* const LatestFailureEvent = FindLatestEnabledFailureEvent())
		{
			AddSectionLine(
				ActivationSection,
				FString::Printf(TEXT("Failure: %s"), *UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(*LatestFailureEvent)),
				UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(*LatestFailureEvent));
		}

		RightSections.Add(MoveTemp(ActivationSection));
	}

	if (IsCategoryEnabled(EMABSDebugEventCategory::Targeting) || IsCategoryEnabled(EMABSDebugEventCategory::Delivery))
	{
		FMABSDebugHarnessSection TargetingSection;
		TargetingSection.Title = TEXT("Targeting / Delivery");
		TargetingSection.AccentColor = IsCategoryEnabled(EMABSDebugEventCategory::Targeting)
			? UMABSDebugBlueprintLibrary::GetDebugEventCategoryColor(EMABSDebugEventCategory::Targeting)
			: UMABSDebugBlueprintLibrary::GetDebugEventCategoryColor(EMABSDebugEventCategory::Delivery);

		if (IsCategoryEnabled(EMABSDebugEventCategory::Targeting))
		{
			AddSectionLine(
				TargetingSection,
				UMABSDebugBlueprintLibrary::FormatTargetTraceDebugInfo(TraceInfo),
				UMABSDebugBlueprintLibrary::GetTargetTraceDebugColor(TraceInfo));

			if (const FMABSAbilityDebugEvent* const LatestTargetingEvent = FindLatestEventByCategory(EMABSDebugEventCategory::Targeting))
			{
				AddSectionLine(
					TargetingSection,
					FString::Printf(TEXT("Targeting: %s"), *UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(*LatestTargetingEvent)),
					UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(*LatestTargetingEvent));
			}
		}

		if (IsCategoryEnabled(EMABSDebugEventCategory::Delivery))
		{
			if (const FMABSAbilityDebugEvent* const LatestDeliveryEvent = FindLatestEventByCategory(EMABSDebugEventCategory::Delivery))
			{
				AddSectionLine(
					TargetingSection,
					FString::Printf(TEXT("Delivery: %s"), *UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(*LatestDeliveryEvent)),
					UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(*LatestDeliveryEvent));
			}
		}

		RightSections.Add(MoveTemp(TargetingSection));
	}

	if (IsCategoryEnabled(EMABSDebugEventCategory::Presentation))
	{
		FMABSDebugHarnessSection PresentationSection;
		PresentationSection.Title = TEXT("Presentation / Cues");
		PresentationSection.AccentColor = UMABSDebugBlueprintLibrary::GetDebugEventCategoryColor(EMABSDebugEventCategory::Presentation);

		if (const FMABSAbilityDebugEvent* const LatestPresentationEvent = FindLatestEventByCategory(EMABSDebugEventCategory::Presentation))
		{
			AddSectionLine(
				PresentationSection,
				FString::Printf(TEXT("Latest: %s"), *UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(*LatestPresentationEvent)),
				UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(*LatestPresentationEvent));
		}
		else
		{
			AddSectionLine(PresentationSection, TEXT("No recent presentation or cue events."), FLinearColor(0.75f, 0.78f, 0.82f, 1.0f));
		}

		RightSections.Add(MoveTemp(PresentationSection));
	}

	FMABSDebugHarnessSection RecentEventsSection;
	RecentEventsSection.Title = TEXT("Recent Events");
	RecentEventsSection.AccentColor = UMABSDebugBlueprintLibrary::GetDebugEventCategoryColor(EMABSDebugEventCategory::General);
	int32 AddedEventCount = 0;
	for (int32 EventIndex = RecentEvents.Num() - 1; EventIndex >= 0 && AddedEventCount < MaxDisplayedEvents; --EventIndex)
	{
		const FMABSAbilityDebugEvent& DebugEvent = RecentEvents[EventIndex];
		if (!IsEventEnabled(DebugEvent))
		{
			continue;
		}

		AddSectionLine(
			RecentEventsSection,
			UMABSDebugBlueprintLibrary::FormatCompactAbilityDebugEvent(DebugEvent),
			UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(DebugEvent));
		++AddedEventCount;
	}

	if (AddedEventCount == 0)
	{
		AddSectionLine(RecentEventsSection, TEXT("No recent events for the enabled categories."), FLinearColor(0.75f, 0.78f, 0.82f, 1.0f));
	}

	RightSections.Add(MoveTemp(RecentEventsSection));

	const float InnerPadding = 14.0f;
	const float ColumnGap = 18.0f;
	const float ColumnWidth = (DrawSize.X - (InnerPadding * 2.0f) - ColumnGap) * 0.5f;
	const float LeftColumnX = OverlayPosition.X + InnerPadding;
	const float RightColumnX = LeftColumnX + ColumnWidth + ColumnGap;
	const float ContentTop = OverlayPosition.Y + 52.0f;
	const float SectionSpacing = 10.0f;

	float LeftColumnY = ContentTop;
	for (const FMABSDebugHarnessSection& Section : LeftSections)
	{
		LeftColumnY += DrawSection(
			Canvas,
			OverlayFont,
			FVector2D(LeftColumnX, LeftColumnY),
			ColumnWidth,
			SectionBackgroundColor,
			OverlayTitleColor,
			Section) + SectionSpacing;
	}

	float RightColumnY = ContentTop;
	for (const FMABSDebugHarnessSection& Section : RightSections)
	{
		RightColumnY += DrawSection(
			Canvas,
			OverlayFont,
			FVector2D(RightColumnX, RightColumnY),
			ColumnWidth,
			SectionBackgroundColor,
			OverlayTitleColor,
			Section) + SectionSpacing;
	}
}

void AMABSDebugHUD::SetOverlayEnabled(const bool bEnabled)
{
	bOverlayEnabled = bEnabled;

	if (IConsoleVariable* const ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(TEXT("mabs.DebugHarness")))
	{
		ConsoleVariable->Set(bEnabled ? 1 : 0);
	}
}

void AMABSDebugHUD::ToggleOverlayEnabled()
{
	SetOverlayEnabled(!IsOverlayEnabled());
}

bool AMABSDebugHUD::IsOverlayEnabled() const
{
	return bOverlayEnabled && CVarMABSDebugHarness.GetValueOnGameThread() != 0;
}

void AMABSDebugHUD::SetCategoryEnabled(const EMABSDebugEventCategory Category, const bool bEnabled)
{
	SetCategoryFlagEnabled(Category, bEnabled);
	SetConsoleCategoryEnabled(Category, bEnabled);
}

void AMABSDebugHUD::ToggleCategoryEnabled(const EMABSDebugEventCategory Category)
{
	SetCategoryEnabled(Category, !IsCategoryEnabled(Category));
}

bool AMABSDebugHUD::IsCategoryEnabled(const EMABSDebugEventCategory Category) const
{
	return IsCategoryFlagEnabled(Category) && IsConsoleCategoryEnabled(Category);
}

bool AMABSDebugHUD::IsCategoryFlagEnabled(const EMABSDebugEventCategory Category) const
{
	switch (Category)
	{
	case EMABSDebugEventCategory::Activation:
		return bShowActivationCategory;

	case EMABSDebugEventCategory::Targeting:
		return bShowTargetingCategory;

	case EMABSDebugEventCategory::Delivery:
		return bShowDeliveryCategory;

	case EMABSDebugEventCategory::CostCooldown:
		return bShowCostCooldownCategory;

	case EMABSDebugEventCategory::Combo:
		return bShowComboCategory;

	case EMABSDebugEventCategory::Periodic:
		return bShowPeriodicCategory;

	case EMABSDebugEventCategory::Presentation:
		return bShowPresentationCategory;

	case EMABSDebugEventCategory::General:
	default:
		return bShowGeneralCategory;
	}
}

bool AMABSDebugHUD::IsConsoleCategoryEnabled(const EMABSDebugEventCategory Category) const
{
	switch (Category)
	{
	case EMABSDebugEventCategory::Activation:
		return CVarMABSDebugHarnessActivation.GetValueOnGameThread() != 0;

	case EMABSDebugEventCategory::Targeting:
		return CVarMABSDebugHarnessTargeting.GetValueOnGameThread() != 0;

	case EMABSDebugEventCategory::Delivery:
		return CVarMABSDebugHarnessDelivery.GetValueOnGameThread() != 0;

	case EMABSDebugEventCategory::CostCooldown:
		return CVarMABSDebugHarnessCostCooldown.GetValueOnGameThread() != 0;

	case EMABSDebugEventCategory::Combo:
		return CVarMABSDebugHarnessCombo.GetValueOnGameThread() != 0;

	case EMABSDebugEventCategory::Periodic:
		return CVarMABSDebugHarnessPeriodic.GetValueOnGameThread() != 0;

	case EMABSDebugEventCategory::Presentation:
		return CVarMABSDebugHarnessPresentation.GetValueOnGameThread() != 0;

	case EMABSDebugEventCategory::General:
	default:
		return CVarMABSDebugHarnessGeneral.GetValueOnGameThread() != 0;
	}
}

void AMABSDebugHUD::SetCategoryFlagEnabled(const EMABSDebugEventCategory Category, const bool bEnabled)
{
	switch (Category)
	{
	case EMABSDebugEventCategory::Activation:
		bShowActivationCategory = bEnabled;
		break;

	case EMABSDebugEventCategory::Targeting:
		bShowTargetingCategory = bEnabled;
		break;

	case EMABSDebugEventCategory::Delivery:
		bShowDeliveryCategory = bEnabled;
		break;

	case EMABSDebugEventCategory::CostCooldown:
		bShowCostCooldownCategory = bEnabled;
		break;

	case EMABSDebugEventCategory::Combo:
		bShowComboCategory = bEnabled;
		break;

	case EMABSDebugEventCategory::Periodic:
		bShowPeriodicCategory = bEnabled;
		break;

	case EMABSDebugEventCategory::Presentation:
		bShowPresentationCategory = bEnabled;
		break;

	case EMABSDebugEventCategory::General:
	default:
		bShowGeneralCategory = bEnabled;
		break;
	}
}

void AMABSDebugHUD::SetConsoleCategoryEnabled(const EMABSDebugEventCategory Category, const bool bEnabled) const
{
	const TCHAR* ConsoleVariableName = TEXT("mabs.DebugHarness.General");
	switch (Category)
	{
	case EMABSDebugEventCategory::Activation:
		ConsoleVariableName = TEXT("mabs.DebugHarness.Activation");
		break;

	case EMABSDebugEventCategory::Targeting:
		ConsoleVariableName = TEXT("mabs.DebugHarness.Targeting");
		break;

	case EMABSDebugEventCategory::Delivery:
		ConsoleVariableName = TEXT("mabs.DebugHarness.Delivery");
		break;

	case EMABSDebugEventCategory::CostCooldown:
		ConsoleVariableName = TEXT("mabs.DebugHarness.CostCooldown");
		break;

	case EMABSDebugEventCategory::Combo:
		ConsoleVariableName = TEXT("mabs.DebugHarness.Combo");
		break;

	case EMABSDebugEventCategory::Periodic:
		ConsoleVariableName = TEXT("mabs.DebugHarness.Periodic");
		break;

	case EMABSDebugEventCategory::Presentation:
		ConsoleVariableName = TEXT("mabs.DebugHarness.Presentation");
		break;

	case EMABSDebugEventCategory::General:
	default:
		break;
	}

	if (IConsoleVariable* const ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(ConsoleVariableName))
	{
		ConsoleVariable->Set(bEnabled ? 1 : 0);
	}
}

UMABSAbilityComponent* AMABSDebugHUD::ResolveAbilityComponent() const
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
