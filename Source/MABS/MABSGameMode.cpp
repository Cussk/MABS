// Copyright Epic Games, Inc. All Rights Reserved.

#include "MABSGameMode.h"
#include "Debug/MABSDebugHUD.h"

AMABSGameMode::AMABSGameMode()
{
	HUDClass = AMABSDebugHUD::StaticClass();
}
