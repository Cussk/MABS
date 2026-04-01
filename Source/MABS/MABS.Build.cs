// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MABS : ModuleRules
{
	public MABS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTags",
			"MABSGameplay",
			"MABSDebug",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MABS",
			"MABS/Variant_Platforming",
			"MABS/Variant_Platforming/Animation",
			"MABS/Variant_Combat",
			"MABS/Variant_Combat/AI",
			"MABS/Variant_Combat/Animation",
			"MABS/Variant_Combat/Gameplay",
			"MABS/Variant_Combat/Interfaces",
			"MABS/Variant_Combat/UI",
			"MABS/Variant_SideScrolling",
			"MABS/Variant_SideScrolling/AI",
			"MABS/Variant_SideScrolling/Gameplay",
			"MABS/Variant_SideScrolling/Interfaces",
			"MABS/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
