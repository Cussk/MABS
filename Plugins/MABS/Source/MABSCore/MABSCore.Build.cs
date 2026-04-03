using UnrealBuildTool;

public class MABSCore : ModuleRules
{
	public MABSCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"Niagara"
			});
	}
}
