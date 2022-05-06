#include "LevelEditor/LevelEditorExtensionCommands.h"

#define LOCTEXT_NAMESPACE "LevelEditorCommands"


FLevelEditorExtensionCommands::FLevelEditorExtensionCommands()
	: TCommands<FLevelEditorExtensionCommands>(
		TEXT("LevelEditorExtensionCommands"), 
		NSLOCTEXT("Contexts", "LevelEditorExtensionCommands", "Level Editor Extension Commands"), 
		"LevelEditor", 
		FEditorStyle::GetStyleSetName())
{
}


void FLevelEditorExtensionCommands::RegisterCommands()
{
	UI_COMMAND(MoveSelectionToRoot, "Remove From Folder", "Remove the selected actors from any folder and parent to the world root", EUserInterfaceActionType::Button, FInputChord());
}


#undef LOCTEXT_NAMESPACE 