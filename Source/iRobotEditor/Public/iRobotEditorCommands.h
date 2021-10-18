#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Commands/UICommandInfo.h"
#include "iRobotEditorStyle.h"

class FiRobotEditorCommands : public TCommands <FiRobotEditorCommands>
{
public:
	FiRobotEditorCommands()
		: TCommands<FiRobotEditorCommands>(TEXT("iRobotEditorCommands"),
			NSLOCTEXT("Contexts", "iRobotEditorCommands", "iRobot Editor Commands"),
			NAME_None, FiRobotEditorStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

	TSharedPtr< FUICommandInfo > iRobotEditor_EditEditorConfig;
};