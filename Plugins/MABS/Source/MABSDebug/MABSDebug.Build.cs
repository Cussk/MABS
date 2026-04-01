using UnrealBuildTool;

public class MABSDebug : ModuleRules
{
	public MABSDebug(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"MABSCore"
			});

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"MABSGameplay"
			});
	}
}
