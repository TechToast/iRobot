#include "iRobotEditor.h"
#include "iRobotEditorCommands.h"

#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/SlateDelegates.h"
#include "Framework/Commands/UICommandList.h"
//#include "Toolkits/AssetEditorManager.h"
#include "LevelEditor.h"


#define LOCTEXT_NAMESPACE "FiRobotEditorModule"


void FiRobotEditorModule::StartupModule()
{
	FiRobotEditorStyle::Initialize();
	FiRobotEditorStyle::ReloadTextures();
	FiRobotEditorCommands::Register();

	// Extend the Main Toolbar
	CreateMainToolbarExtender();
}


void FiRobotEditorModule::ShutdownModule()
{
	FiRobotEditorStyle::Shutdown();
	FiRobotEditorCommands::Unregister();
}


void FiRobotEditorModule::CreateMainToolbarExtender()
{
	TSharedPtr<FUICommandList> Commands = MakeShared<FUICommandList>();
	Commands->MapAction(
		FiRobotEditorCommands::Get().iRobotEditor_EditEditorConfig,
		FExecuteAction::CreateRaw(&ConfigurationEditor, &FGameConfigurationEditor::EditEditorConfig),
		FCanExecuteAction());

	TSharedRef<FExtender> MainToolbarExtender(new FExtender());
	MainToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, Commands,
		FToolBarExtensionDelegate::CreateRaw(this, &FiRobotEditorModule::AddMainToolbarExtension));

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(MainToolbarExtender);
}


void FiRobotEditorModule::AddMainToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(
		FiRobotEditorCommands::Get().iRobotEditor_EditEditorConfig,
		NAME_None,
		TAttribute<FText>(),
		TAttribute<FText>(),
		FSlateIcon(FiRobotEditorStyle::GetStyleSetName(), "iRobotEditorStyle.GameConfigurationEditor")
	);
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FiRobotEditorModule, iRobotEditor)
