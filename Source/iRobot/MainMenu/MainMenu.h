#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ISessionMenu.h"
#include "MainMenu.generated.h"

class UButton;

// Struct for sessions
USTRUCT()
struct FServerData
{
	GENERATED_BODY()

	FString Name;
	uint16 CurrentPlayers;
	uint16 MaxPlayers;
	FString HostUsername;
};


UCLASS()
class UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	void Setup(ISessionMenu* SessionMenuInterface);
	void Teardown();

	//void InitializeSessionsList(TArray<FServerData> Data);
	//void SelectIndexSessionList(uint32 Index);
	void PopulateSessionList(TArray<FServerData> Data);
	void OnSessionRowClicked(uint32 Index);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	ISessionMenu* SessionMenuInterface;
	
	/// New session button
	UPROPERTY(meta = (BindWidget))
	UButton* HostSessionButton;

	/// Join session button
	UPROPERTY(meta = (BindWidget))
	UButton* SearchSessionsButton;

	/// Cancel joining session button 
	//UPROPERTY(meta = (BindWidget))
	//UButton* CancelJoinSessionButton;

	/// Join the selected session button
	//UPROPERTY(meta = (BindWidget))
	//UButton* JoinSelectedSessionButton;

	/// Menu switcher for switching between various sub menus
	//UPROPERTY(meta = (BindWidget))
	//class UWidgetSwitcher* MenuSwitcher;

	/// Host session menu
	//UPROPERTY(meta = (BindWidget))
	//class UWidget* HostSessionMenuWidget;

	/// Session list menu
	//UPROPERTY(meta = (BindWidget))
	//class UWidget* SessionListMenuWidget;

	/// Scroll list of sessions
	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* ScrollSessionList;

	/// Class to use for each session row 
	UPROPERTY(EditAnywhere)
	TSubclassOf<class USessionRow> SessionRowClass;
	
	// Callbacks
	UFUNCTION()
	void OnHostSessionPressed();
	UFUNCTION()
	void OnSearchSessionsPressed();
	//UFUNCTION()
	//void OnCancelJoinSessionPressed();
	//UFUNCTION()
	//void OnJoinSelectedSessionPressed();

private:
	//TOptional<uint32> SelectedScrollIndex;
};
