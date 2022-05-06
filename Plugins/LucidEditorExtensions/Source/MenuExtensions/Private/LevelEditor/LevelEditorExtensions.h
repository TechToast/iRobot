#pragma once

#include "CoreMinimal.h"

///
/// Class to handle Level Editor extensions
///
class FLevelEditorExtensions
{
public:
	FLevelEditorExtensions();
	~FLevelEditorExtensions();

private:
	void MapCommands();

	TSharedPtr<class FUICommandList> Commands;
};
