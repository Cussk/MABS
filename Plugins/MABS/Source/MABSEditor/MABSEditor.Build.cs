using UnrealBuildTool;

public class MABSEditor : ModuleRules
{
	public MABSEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CoreUObject",
				"DataValidation",
				"Engine",
				"UnrealEd",
				"MABSCore",
				"MABSGameplay",
				"MABSDebug"
			});
	}
}
