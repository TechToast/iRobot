// Fill out your copyright notice in the Description page of Project Settings.


#include "P4MenuExtensionCommands.h"

#define LOCTEXT_NAMESPACE "P4MenuExtensionCommands"

void FP4MenuExtensionCommands::RegisterCommands()
{
	UI_COMMAND(CheckoutSourceAsset, "Check Out Source", "Check out source asset.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(RevertSourceAsset, "Revert Source", "Revert source asset.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(AddSourceAsset, "Mark Source for Add", "Open source asset for add.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(FocusSourceWorkspaceTree, "Focus Source in P4V", "Focus source asset in P4V workspace tree.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(FocusWorkspaceTree, "Focus in P4V", "Focus asset in P4V workspace tree.", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE 