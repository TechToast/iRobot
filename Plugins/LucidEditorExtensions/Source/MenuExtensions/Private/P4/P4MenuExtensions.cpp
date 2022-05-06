#if 0

// Fill out your copyright notice in the Description page of Project Settings.

#include "P4MenuExtensions.h"

#include "P4MenuExtensionCommands.h"
#include "MenuExtensionsMiscUtils.h"
#include "AssetToolsModule.h"
#include "P4ExtensionEditorModule.h"
#include "Windows/AllowWindowsPlatformTypes.h" // For WinExec
#include "IContentBrowserSingleton.h" // For content browser singleton
#include "Misc/Paths.h"

FP4MenuExtensions::FP4MenuExtensions()
{
	MapCommands();
	InstallContentBrowserHooks();
}

FP4MenuExtensions::~FP4MenuExtensions()
{
	FP4MenuExtensionCommands::Unregister();
	RemoveHooks();
}

void FP4MenuExtensions::MapCommands()
{
	FP4MenuExtensionCommands::Register();

	Commands = MakeShareable(new FUICommandList);

	// Map commands
	Commands->MapAction(
		FP4MenuExtensionCommands::Get().FocusWorkspaceTree,
		FExecuteAction::CreateLambda([&]()
			{
				// Open P4V focused on file in workspace tree
				if (FPaths::FileExists(SelectedAssetPath))
				{
					FString command = FString::Format(TEXT("p4v -s {0}"), { SelectedAssetPath });
					WinExec(TCHAR_TO_UTF8(*command), SW_SHOWNORMAL);
				}
			})
	);
	Commands->MapAction(
		FP4MenuExtensionCommands::Get().FocusSourceWorkspaceTree,
		FExecuteAction::CreateLambda([&]()
			{
				if (SourceFilePaths.Num() > 0)
				{
					// Open P4V focused on file in workspace tree
					FString command = FString::Format(TEXT("p4v -s {0}"), { SourceFilePaths.Last() });
					WinExec(TCHAR_TO_UTF8(*command), SW_SHOWNORMAL);
				}
			})
	); 
	Commands->MapAction(
		FP4MenuExtensionCommands::Get().CheckoutSourceAsset,
		FExecuteAction::CreateLambda([&]()
			{
				for (FString file : SourceFilePaths)
				{
					P4ExtensionEditorModule::Get().CheckOutFile(file);
				}
			})
	);
	Commands->MapAction(
		FP4MenuExtensionCommands::Get().RevertSourceAsset,
		FExecuteAction::CreateLambda([&]()
			{
				for (FString file : SourceFilePaths)
				{
					P4ExtensionEditorModule::Get().RevertFile(file);
				}
			})
	);
	Commands->MapAction(
		FP4MenuExtensionCommands::Get().AddSourceAsset,
		FExecuteAction::CreateLambda([&]()
			{
				for (FString file : SourceFilePaths)
				{
					P4ExtensionEditorModule::Get().AddFile(file);
				}
			})
	);
}

void FP4MenuExtensions::InstallContentBrowserHooks()
{
	// Get module
	FContentBrowserModule& contentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	// Extend asset menu in content browser
	ContentBrowserAssetContextExtenderDelegate = FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FP4MenuExtensions::OnExtendContentBrowserAssetSelectionMenu);

	TArray<FContentBrowserMenuExtender_SelectedAssets>& contentBrowserMenuExtenderDelegates = contentBrowserModule.GetAllAssetViewContextMenuExtenders();
	contentBrowserMenuExtenderDelegates.Add(ContentBrowserAssetContextExtenderDelegate);
	ContentBrowserAssetContextExtenderDelegateHandle = contentBrowserMenuExtenderDelegates.Last().GetHandle();
}

TSharedRef<FExtender> FP4MenuExtensions::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender(new FExtender());

	if (SelectedAssets.Num() > 0)
	{
		// Get physical file path - if we run into problems with this we should get path from FContentBrowserItem instead
		FAssetData asset = SelectedAssets.Last();
		FString relativePath = asset.PackagePath.ToString();
		relativePath.RemoveFromStart(TEXT("/Game"));
		relativePath.RemoveFromStart(TEXT("/"));
		FString absolutePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
		SelectedAssetPath = FPaths::Combine(absolutePath, relativePath, asset.AssetName.ToString() + TEXT(".uasset"));
		// Option to open P4V focused on file in workspace tree
		if (FPaths::FileExists(SelectedAssetPath))
		{
			Extender->AddMenuExtension("AssetSourceControlActions",
				EExtensionHook::After,
				Commands,
				FNewMenuDelegate::CreateLambda([](FMenuBuilder& SubMenuBuilder)
					{
						SubMenuBuilder.AddMenuEntry(
							FP4MenuExtensionCommands::Get().FocusWorkspaceTree,
							FName("SCCOpenP4V"),
							TAttribute<FText>(),
							TAttribute<FText>(),
							FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.AssetActions.OpenSourceLocation")
						);
					}
				)
			);
		}
	}

	// If assets have source file add sub-menu
	if (FMenuExtensionsMiscUtils::TryGetAssetSourcePaths(SelectedAssets, SourceFilePaths))
	{
		// Conditionally populate add/revert/checkout options
		if (!AreAssetsInDepot())
		{
			Extender->AddMenuExtension("AssetSourceControlActions",
				EExtensionHook::After,
				Commands,
				FNewMenuDelegate::CreateLambda([](FMenuBuilder& SubMenuBuilder)
					{
						SubMenuBuilder.AddMenuEntry(
							FP4MenuExtensionCommands::Get().AddSourceAsset,
							FName("SCCOpenSourceForAdd"),
							TAttribute<FText>(),
							TAttribute<FText>(),
							FSlateIcon(FEditorStyle::GetStyleSetName(), "SourceControl.Actions.Add")
						);
					}
				)
			);
		}
		else if (AreAssetsCheckedOutOrAdded())
		{
			Extender->AddMenuExtension("AssetSourceControlActions",
				EExtensionHook::After,
				Commands,
				FNewMenuDelegate::CreateLambda([](FMenuBuilder& SubMenuBuilder)
					{
						SubMenuBuilder.AddMenuEntry(
							FP4MenuExtensionCommands::Get().RevertSourceAsset,
							FName("SCCRevertSource"),
							TAttribute<FText>(),
							TAttribute<FText>(),
							FSlateIcon(FEditorStyle::GetStyleSetName(), "SourceControl.Actions.Revert")
						);
					}
				)
			);
		}
		else
		{
			Extender->AddMenuExtension("AssetSourceControlActions",
				EExtensionHook::After,
				Commands,
				FNewMenuDelegate::CreateLambda([](FMenuBuilder& SubMenuBuilder)
					{
						SubMenuBuilder.AddMenuEntry(
							FP4MenuExtensionCommands::Get().CheckoutSourceAsset,
							FName("SCCCheckoutSource"),
							TAttribute<FText>(),
							TAttribute<FText>(),
							FSlateIcon(FEditorStyle::GetStyleSetName(), "SourceControl.Actions.CheckOut")
						);
					}
				)
			);
		}

		// Option to open P4V focused on source file in workspace tree
		Extender->AddMenuExtension("AssetSourceControlActions",
			EExtensionHook::After,
			Commands,
			FNewMenuDelegate::CreateLambda([](FMenuBuilder& SubMenuBuilder)
				{
					SubMenuBuilder.AddMenuEntry(
						FP4MenuExtensionCommands::Get().FocusSourceWorkspaceTree,
						FName("SCCOpenSourceP4V"),
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

bool FP4MenuExtensions::AreAssetsInDepot() const
{
	P4ExtensionEditorModule& p4 = P4ExtensionEditorModule::Get();
	for (const FString& assetPath : SourceFilePaths)
	{
		bool result;
		if (p4.FileExistsInDepot(assetPath, result) && !result)
		{
			return false;
		}
	}

	return true;
}

bool FP4MenuExtensions::AreAssetsCheckedOutOrAdded() const
{
	P4ExtensionEditorModule& p4 = P4ExtensionEditorModule::Get();
	for (const FString& assetPath : SourceFilePaths)
	{
		bool checkOutResult;
		bool addResult;
		if (p4.FileIsCheckedOut(assetPath, checkOutResult) && !checkOutResult
			&& p4.FileIsMarkedForAdd(assetPath, addResult) && !addResult)
		{
			return false;
		}
	}

	return true;
}

void FP4MenuExtensions::RemoveHooks()
{
	FContentBrowserModule& contentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TArray<FContentBrowserMenuExtender_SelectedAssets>& contentBrowserMenuExtenderDelegates = contentBrowserModule.GetAllAssetViewContextMenuExtenders();
	contentBrowserMenuExtenderDelegates.RemoveAll([&](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == ContentBrowserAssetContextExtenderDelegateHandle; });
}

#endif