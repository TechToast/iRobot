#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MainMenu/ISessionMenu.h"
#include "iRobotGameInstance.generated.h"


class UMainMenu;
class UUserWidget;
class FOnlineSessionSearch;


UCLASS(MinimalAPI)
class UiRobotGameInstance : public UGameInstance, public ISessionMenu
{
	GENERATED_BODY()

public:

	UiRobotGameInstance(const FObjectInitializer& ObjectInitializer);

	virtual void Init();

	/// Begin ISessionMenu
	virtual void Host(FString ServerName) override;
	virtual void JoinSession(uint32 Index) override;
	//virtual void EndSession() override;
	virtual void BeginSessionSearch() override;
	/// End ISessionMenu

	/// Entry point for the main menu. Called from MainMenu map's level BP
	UFUNCTION(BlueprintCallable, Category = "iRobot")
	void LoadMainMenu();

	/// The Main Menu widget class to use
	UPROPERTY(EditDefaultsOnly, Category = "iRobot")
	TSoftClassPtr<UUserWidget> MenuClass;
	
	/// The Hunter Game Map to load
	UPROPERTY(EditDefaultsOnly, Category = "iRobot")
	TSoftObjectPtr<UWorld> HunterGameMap;

private:
	/// The Main Menu widget
	TWeakObjectPtr<UMainMenu> MainMenu;

	/// Session stuff
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	// Session Events
	FString DesiredServerName;
	void OnCreateSessionComplete(FName SessionName, bool bSuccess);
	void OnDestroySessionComplete(FName SessionName, bool bSuccess);
	void OnFindSessionsComplete(bool bSuccess);
	void OnJoinSessionsComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void CreateSession();
};