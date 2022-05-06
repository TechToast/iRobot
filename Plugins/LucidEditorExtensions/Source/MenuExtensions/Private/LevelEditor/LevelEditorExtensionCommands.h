#pragma once

#include "CoreMinimal.h"
#include "EditorStyleSet.h"


class FLevelEditorExtensionCommands : public TCommands<FLevelEditorExtensionCommands>
{
public:
	FLevelEditorExtensionCommands();

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> MoveSelectionToRoot;
};
