#include "iRobotEditorCommands.h"

#define LOCTEXT_NAMESPACE "iRobotEditorCommands"


void FiRobotEditorCommands::RegisterCommands()
{
	UI_COMMAND(iRobotEditor_EditEditorConfig, "Config Editor", "Open the configuration editor to edit the local GameConfigurationData.json file", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE 