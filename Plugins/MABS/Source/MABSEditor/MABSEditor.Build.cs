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
				"Engine",
				"UnrealEd",
				"MABSCore",
				"MABSGameplay",
				"MABSDebug"
			});
	}
}
