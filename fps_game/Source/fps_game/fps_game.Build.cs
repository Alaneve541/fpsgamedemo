using UnrealBuildTool;

public class fps_game : ModuleRules
{
	public fps_game(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Slate",
			"SlateCore",
			"UMG",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Json",
			"JsonUtilities",
			"ApplicationCore",
			"NavigationSystem",
			"AIModule",
			"GameplayStateTreeModule",
			"StateTreeModule"
		});
	}
}
