#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Modules/ModuleInterface.h"
#include "Logging/LogMacros.h"
#include "Delegates/IDelegateInstance.h"
#include "Templates/SharedPointer.h"
#include "Widgets/SWidget.h"
#include "GameConfigurationEditor.h"

DECLARE_LOG_CATEGORY_EXTERN(iRobotEditorLog, Log, All)

class FToolBarBuilder;
class FUICommandList;

class FiRobotEditorModule : public IModuleInterface
{
public:

	/// IModuleInterface implementation
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	/// Create the extender for the main toolbar
	void CreateMainToolbarExtender();

	/// UI Generation
	void AddMainToolbarExtension(FToolBarBuilder& Builder);

	// Object which deals with all GameConfigurationEditor requests
	FGameConfigurationEditor ConfigurationEditor;
};
