using UnrealBuildTool;
using System.Collections.Generic;

public class fps_gameEditorTarget : TargetRules
{
	public fps_gameEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("fps_game");
	}
}
