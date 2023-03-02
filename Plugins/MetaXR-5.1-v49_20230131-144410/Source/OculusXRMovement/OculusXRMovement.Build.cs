// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class OculusXRMovement : ModuleRules
	{
		public OculusXRMovement(ReadOnlyTargetRules Target) : base(Target)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"ApplicationCore",
					"Engine",
					"InputCore",
					"HeadMountedDisplay",
					"OVRPluginXR",
					"OculusXRHMD",
				});

			PrivateIncludePaths.AddRange(
				new string[] {
					// Relative to Engine\Plugins\Runtime\OculusXR\OculusXRVR\Source
					"OculusXRHMD/Private",
				});
		}
	}
}
