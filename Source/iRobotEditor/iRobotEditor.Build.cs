using UnrealBuildTool;


public class iRobotEditor : ModuleRules
{
 	public iRobotEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        bBuildLocallyWithSNDBS = true;

        PublicIncludePaths.AddRange(
			new string[] {
				"iRobotEditor/Public",
                "iRobot"
            }
		);

        PrivateIncludePaths.AddRange(
			new string[] {
				"iRobotEditor/Private"
            }
        );


        PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "CoreUObject",
                "Engine",
                "Core",
                "UnrealEd",
                "iRobot",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "RenderCore",
                "UMG",
				"Json", 
				"JsonUtilities"
            }
		);

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
            }
        );
    }
}