// Copyright Epic Games, Inc. All Rights Reserved.


#include "MABSPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "Blueprint/UserWidget.h"
#include "Debug/MABSDebugHUD.h"
#include "MABS.h"
#include "MABSDemoHUD.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AMABSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogMABS, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AMABSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}

	if (IsLocalPlayerController() && InputComponent != nullptr)
	{
		InputComponent->BindKey(EKeys::F1, IE_Pressed, this, &AMABSPlayerController::HandleToggleDemoHelp);
		InputComponent->BindKey(EKeys::F2, IE_Pressed, this, &AMABSPlayerController::HandleToggleDebugHarness);
		InputComponent->BindKey(EKeys::F3, IE_Pressed, this, &AMABSPlayerController::HandleToggleValidationNotes);
	}
}

bool AMABSPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void AMABSPlayerController::HandleToggleDemoHelp()
{
	if (AMABSDemoHUD* const DemoHUD = Cast<AMABSDemoHUD>(GetHUD()))
	{
		DemoHUD->ToggleHelpPanelEnabled();
	}
}

void AMABSPlayerController::HandleToggleDebugHarness()
{
	if (AMABSDebugHUD* const DebugHUD = Cast<AMABSDebugHUD>(GetHUD()))
	{
		DebugHUD->ToggleOverlayEnabled();
	}
}

void AMABSPlayerController::HandleToggleValidationNotes()
{
	if (AMABSDemoHUD* const DemoHUD = Cast<AMABSDemoHUD>(GetHUD()))
	{
		DemoHUD->ToggleValidationPanelEnabled();
	}
}
