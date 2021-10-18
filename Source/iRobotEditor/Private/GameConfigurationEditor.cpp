#include "GameConfigurationEditor.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformProcess.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "FGameConfigurationEditor"


void FGameConfigurationEditor::EditEditorConfig()
{
	FString ExecutablePath;
	if (FindGameConfigurationBuilderExecutable(ExecutablePath))
	{
		// Launch the exe
		FProcHandle Handle = FPlatformProcess::CreateProc(*ExecutablePath, *GetEditorConfigFile(), true, false, false, nullptr, 0, nullptr, nullptr);
		if (!Handle.IsValid())
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("EditEditorConfig_ProcessFailed", "Unable to launch GameConfigurationBuilder.exe"));
			FPlatformProcess::CloseProc(Handle);
			return;
		}
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("EditEditorConfig_ExeNotFound", "Unable to find GameConfigurationBuilder.exe at expected location. Do you have the latest Tools folder from Perforce?"));
		return;
	}
}


FString FGameConfigurationEditor::GetEditorConfigFile()
{
	FString EditorConfigFilePath = FPaths::ProjectDir() / GAME_CONFIGURATION_FILE_NAME;
	EditorConfigFilePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*EditorConfigFilePath);

	// If the file doesn't exist, create one
	if (!FPaths::FileExists(EditorConfigFilePath))
		FFileHelper::SaveStringToFile(FString::Printf(TEXT("{\"%s\":{}}"), *GAME_CONFIGURATION_ROOT_NODE_NAME), *EditorConfigFilePath, FFileHelper::EEncodingOptions::AutoDetect);

	return EditorConfigFilePath;
}


bool FGameConfigurationEditor::FindGameConfigurationBuilderExecutable(FString& OutPath)
{
	FString ExpectedExecutablePath = TEXT("D:/Work/Lucid Games/Bluestar/Tools/GameConfigurationBuilder/GameConfigurationBuilder/bin/Release/GameConfigurationBuilder.exe");
	//FString ExpectedExecutablePath = TEXT("../") / FPaths::GetPath(FPaths::GetPath(FPaths::ProjectDir())) / TEXT("Tools/GameConfigurationBuilder/GameConfigurationBuilder.exe");
	//ExpectedExecutablePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*ExpectedExecutablePath);
	if (FPaths::FileExists(ExpectedExecutablePath))
	{
		OutPath = ExpectedExecutablePath;
		return true;
	}

	return false;
}

#undef LOCTEXT_NAMESPACE 