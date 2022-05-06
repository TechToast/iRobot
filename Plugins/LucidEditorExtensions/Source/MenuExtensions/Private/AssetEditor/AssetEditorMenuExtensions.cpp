// Copyright Lucid Games, Inc. All Rights Reserved.

#include "AssetEditorMenuExtensions.h"

#include "AssetEditorCommands.h"
#include "MenuExtensionsMiscUtils.h"

FAssetEditorMenuExtensions::FAssetEditorMenuExtensions()
{
	MapCommands();
	InstallAssetEditorHooks();
}

FAssetEditorMenuExtensions::~FAssetEditorMenuExtensions()
{
	FAssetEditorCommands::Unregister();
	RemoveHooks();
}

void FAssetEditorMenuExtensions::MapCommands()
{
	FAssetEditorCommands::Register();

	Commands = MakeShareable(new FUICommandList);

	Commands->MapAction(
		FAssetEditorCommands::Get().OpenSourceLocation,
		FExecuteAction::CreateLambda([&]()
			{
				FPlatformProcess::ExploreFolder(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*SourceFilePath));
			})
	);
}

void FAssetEditorMenuExtensions::InstallAssetEditorHooks()
{
	AssetEditorMenuExtenderDelegate = FAssetEditorExtender::CreateRaw(this, &FAssetEditorMenuExtensions::OnExtendAssetEditorMenu);

	TArray<FAssetEditorExtender>& AssetEditorMenuExtenderDelegates = FAssetEditorToolkit::GetSharedMenuExtensibilityManager()->GetExtenderDelegates();
	AssetEditorMenuExtenderDelegates.Add(AssetEditorMenuExtenderDelegate);
	AssetEditorMenuExtenderDelegateHandle = AssetEditorMenuExtenderDelegates.Last().GetHandle();
}

TSharedRef<FExtender> FAssetEditorMenuExtensions::OnExtendAssetEditorMenu(const TSharedRef<FUICommandList> CommandList, const TArray<UObject*> Assets)
{
	TSharedRef<FExtender> Extender(new FExtender());

	// If assets have source file add sub-menu
	TArray<FString> sourcePaths;
	if (FMenuExtensionsMiscUtils::TryGetAssetSourcePaths({ FAssetData(Assets.Last()) }, sourcePaths))
	{
		SourceFilePath = sourcePaths.Last();

		Extender->AddMenuExtension("FindInContentBrowser",
			EExtensionHook::Before,
			Commands,
			FNewMenuDelegate::CreateLambda([](FMenuBuilder& SubMenuBuilder)
				{
					SubMenuBuilder.AddMenuEntry(
						FAssetEditorCommands::Get().OpenSourceLocation,
						FName("OpenSourceLocation"),
						TAttribute<FText>(),
						TAttribute<FText>(),
						FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.AssetActions.OpenSourceLocation")
					);
				}
			)
		);
	}

	return Extender;
}

void FAssetEditorMenuExtensions::RemoveHooks()
{
	TArray<FAssetEditorExtender>& AssetEditorMenuExtenderDelegates = FAssetEditorToolkit::GetSharedMenuExtensibilityManager()->GetExtenderDelegates();
	AssetEditorMenuExtenderDelegates.RemoveAll([&](const FAssetEditorExtender& Delegate) { return Delegate.GetHandle() == AssetEditorMenuExtenderDelegateHandle; });
}
