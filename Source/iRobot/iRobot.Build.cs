// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class iRobot : ModuleRules
{
	public iRobot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"OnlineSubsystemSteam",
				"PhysicsCore",
				"UMG",
				"Json",
				"JsonUtilities",
				"NetCore",
			}
		);

		PublicIncludePaths.AddRange(
			new string[] {
				"iRobot"
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				"iRobot"
			}
		);
	}
}
