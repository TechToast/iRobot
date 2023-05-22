#include "MainMenu.h"
#include "iRobot.h"
#include "GameFramework/PlayerController.h"
#include "SessionRow.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"


void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (HostSessionButton && SearchSessionsButton)
	{
		HostSessionButton->OnClicked.AddDynamic(this, &UMainMenu::OnHostSessionPressed);
		SearchSessionsButton->OnClicked.AddDynamic(this, &UMainMenu::OnSearchSessionsPressed);
		//CancelJoinSessionButton->OnClicked.AddDynamic(this, &UMainMenu::OnCancelJoinSessionPressed);
		//JoinSelectedSessionButton->OnClicked.AddDynamic(this, &UMainMenu::OnJoinSelectedSessionPressed);
	}
}


void UMainMenu::NativeDestruct()
{
	Super::NativeDestruct();

	if (HostSessionButton)
		HostSessionButton->OnClicked.RemoveDynamic(this, &UMainMenu::OnHostSessionPressed);

	if (SearchSessionsButton)
		SearchSessionsButton->OnClicked.RemoveDynamic(this, &UMainMenu::OnSearchSessionsPressed);

	//if (CancelJoinSessionButton)
	//	CancelJoinSessionButton->OnClicked.RemoveDynamic(this, &UMainMenu::OnCancelJoinSessionPressed);

	//if (JoinSelectedSessionButton)
	//	JoinSelectedSessionButton->OnClicked.RemoveDynamic(this, &UMainMenu::OnJoinSelectedSessionPressed);
}


void UMainMenu::Setup(ISessionMenu* MenuInterface)
{
	SessionMenuInterface = MenuInterface;

	this->AddToViewport();

	UWorld* World = GetWorld();
	if (World == nullptr) 
		return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController == nullptr)
		return;

	// Set the Input Mode for the Player Controller as the UI only
	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(this->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputModeData);
	//PlayerController->LockInput();
	PlayerController->bShowMouseCursor = true;
}


void UMainMenu::Teardown()
{
	this->RemoveFromParent();

	UWorld* World = GetWorld();
	if (World == nullptr)
		return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController == nullptr) 
		return;

	// Set the Input Mode for game mode: allows only the player input / player controller to respond to user input.
	FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);
	//PlayerController->UnLockInput();
	PlayerController->bShowMouseCursor = false;
}


void UMainMenu::OnHostSessionPressed()
{
	if (SessionMenuInterface)
		SessionMenuInterface->Host(TEXT("iRobot"));
}


void UMainMenu::OnSearchSessionsPressed()
{
	if (ScrollSessionList)
	{
		UTextBlock* SearchingText = WidgetTree->ConstructWidget<UTextBlock>();
		if (SearchingText)
		{
			SearchingText->SetText(FText::FromString(TEXT("Searching...")));
			ScrollSessionList->AddChild(SearchingText);
		}
	}

	if (!SessionMenuInterface) 
		return;

	SessionMenuInterface->BeginSessionSearch();
}


/*void UMainMenu::OnCancelJoinSessionPressed()
{
	if ((MenuSwitcher == nullptr) || (HostSessionMenuWidget == nullptr)) return;

	MenuSwitcher->SetActiveWidget(HostSessionMenuWidget);
}*/


/*void UMainMenu::OnJoinSelectedSessionPressed()
{
	if (!ScrollSessionList || !SessionMenuInterface) 
		return;

	if (SelectedScrollIndex.IsSet())
	{
		int32 ScrollCount = ScrollSessionList->GetChildrenCount();
		int32 SelectedIndex = (int32)SelectedScrollIndex.GetValue();
		if ((ScrollCount > 0) && (SelectedIndex >= 0) && (SelectedIndex < ScrollCount))
		{
			SessionMenuInterface->JoinSession(SelectedScrollIndex.GetValue());
		}
		else
		{
			UE_LOG(LogiRobot, Warning, TEXT("[UMainMenu::InitializeSessionsList] No sessions available"));
		}
		
	}
	else
	{
		UE_LOG(LogiRobot, Warning, TEXT("[UMainMenu::InitializeSessionsList] Unable to Join Session"));
	}
}*/


/*void UMainMenu::SelectIndexSessionList(uint32 Index)
{
	UE_LOG(LogiRobot, Warning, TEXT("[UMainMenu::SelectIndex] SelectIndex: %i"), Index);

	SelectedScrollIndex = Index;
	UpdateSessionList();
}


void UMainMenu::UpdateSessionList()
{
	if (!ScrollSessionList) 
		return;

	// Start from 1, not counting Header
	int indexRow = 0;
	for (int32 i = 1; i < ScrollSessionList->GetChildrenCount(); ++i)
	{
		auto Row = Cast<USessionRow>(ScrollSessionList->GetChildAt(i));
		if (Row != nullptr)
		{
			Row->Selected = (SelectedScrollIndex.IsSet() && (SelectedScrollIndex.GetValue() == indexRow));

			indexRow++;
		}
	}
}*/


void UMainMenu::PopulateSessionList(TArray<FServerData> Data)
{
	UE_LOG(LogiRobot, Warning, TEXT("UMainMenu::PopulateSessionList() - %i"), Data.Num());

	if (!ScrollSessionList) 
		return;

	UWorld* World = this->GetWorld();
	if (!World) 
		return;

	ScrollSessionList->ClearChildren();
	uint32 IndexRow = 0;
	for (const FServerData& ServerData : Data)
	{
		UE_LOG(LogiRobot, Warning, TEXT("UMainMenu::PopulateSessionList() - %s"), *ServerData.Name);

		USessionRow* Row = CreateWidget<USessionRow>(World, SessionRowClass);
		if (!Row) 
			return;

		Row->ServerName->SetText(FText::FromString(ServerData.Name));
		Row->HostUser->SetText(FText::FromString(ServerData.HostUsername));

		FString FractionText = FString::Printf(TEXT("%d/%d"), ServerData.CurrentPlayers, ServerData.MaxPlayers);
		Row->ConnectionFraction->SetText(FText::FromString(FractionText));

		Row->Setup(this, IndexRow);
		++IndexRow;
		ScrollSessionList->AddChild(Row);
	}
}


void UMainMenu::OnSessionRowClicked(uint32 Index)
{
	if (SessionMenuInterface)
	{
		SessionMenuInterface->JoinSession(Index);
	}
}