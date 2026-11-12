using UnrealBuildTool;
using System.Collections.Generic;

public class fps_gameTarget : TargetRules
{
	public fps_gameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("fps_game");
	}
}
