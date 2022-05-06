#include "LevelEditorExtensions.h"

#include "LevelEditorExtensionCommands.h"
#include "EditorActorFolders.h"
#include "LevelEditor.h"


FLevelEditorExtensions::FLevelEditorExtensions()
{
	FLevelEditorExtensionCommands::Register();

	MapCommands();
}


FLevelEditorExtensions::~FLevelEditorExtensions()
{
	FLevelEditorExtensionCommands::Unregister();
}


void FLevelEditorExtensions::MapCommands()
{
	Commands = MakeShareable(new FUICommandList);
	Commands->MapAction(
		FLevelEditorExtensionCommands::Get().MoveSelectionToRoot,
		FExecuteAction::CreateLambda([&]()
			{
				FActorFolders::Get().CreateFolderContainingSelection(*GEditor->GetEditorWorldContext().World(), NAME_None);
			})
	);

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	LevelEditorModule.GetGlobalLevelEditorActions()->Append(Commands.ToSharedRef());
}