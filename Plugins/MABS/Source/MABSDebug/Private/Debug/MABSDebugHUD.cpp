// Copyright Epic Games, Inc. All Rights Reserved.

#include "Debug/MABSDebugHUD.h"
#include "CanvasItem.h"
#include "Components/MABSAbilityComponent.h"
#include "Debug/MABSDebugBlueprintLibrary.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

AMABSDebugHUD::AMABSDebugHUD()
{
}

void AMABSDebugHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!bOverlayEnabled || Canvas == nullptr || !IsValid(PlayerOwner) || !PlayerOwner->IsLocalPlayerController())
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

	FCanvasTileItem BackgroundTile(OverlayPosition, OverlaySize, OverlayBackgroundColor);
	BackgroundTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BackgroundTile);

	float CurrentY = OverlayPosition.Y + 10.0f;
	const float LeftX = OverlayPosition.X + 12.0f;
	const float LineHeight = 16.0f;

	FCanvasTextItem TitleItem(FVector2D(LeftX, CurrentY), FText::FromString(TEXT("MABS Debug Overlay")), OverlayFont, OverlayTitleColor);
	TitleItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TitleItem);
	CurrentY += 24.0f;

	const FMABSTargetTraceDebugInfo TraceInfo = AbilityComponent->GetLatestTargetTraceDebugInfo();
	const FString TraceLine = UMABSDebugBlueprintLibrary::FormatTargetTraceDebugInfo(TraceInfo);
	FCanvasTextItem TraceItem(
		FVector2D(LeftX, CurrentY),
		FText::FromString(TraceLine),
		OverlayFont,
		UMABSDebugBlueprintLibrary::GetTargetTraceDebugColor(TraceInfo));
	TraceItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TraceItem);
	CurrentY += 28.0f;

	TArray<FMABSAbilityDebugEvent> RecentEvents = AbilityComponent->GetRecentDebugEvents();
	const int32 EventCount = FMath::Min(MaxDisplayedEvents, RecentEvents.Num());
	const int32 FirstEventIndex = RecentEvents.Num() - EventCount;
	for (int32 EventIndex = FirstEventIndex; EventIndex < RecentEvents.Num(); ++EventIndex)
	{
		const FMABSAbilityDebugEvent& DebugEvent = RecentEvents[EventIndex];
		FCanvasTextItem EventItem(
			FVector2D(LeftX, CurrentY),
			FText::FromString(UMABSDebugBlueprintLibrary::FormatAbilityDebugEvent(DebugEvent)),
			OverlayFont,
			UMABSDebugBlueprintLibrary::GetAbilityDebugEventColor(DebugEvent));
		EventItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(EventItem);
		CurrentY += LineHeight;
	}
}

void AMABSDebugHUD::SetOverlayEnabled(const bool bEnabled)
{
	bOverlayEnabled = bEnabled;
}

void AMABSDebugHUD::ToggleOverlayEnabled()
{
	bOverlayEnabled = !bOverlayEnabled;
}

bool AMABSDebugHUD::IsOverlayEnabled() const
{
	return bOverlayEnabled;
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
