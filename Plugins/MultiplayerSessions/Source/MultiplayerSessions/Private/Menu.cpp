// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	const auto World = GetWorld();
	if (!World)
		return;

	const auto PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
		return;

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputModeData);
	PlayerController->bShowMouseCursor = ShowMouseCursor;

	const auto GameInstance = GetGameInstance();
	if (!GameInstance)
		return;

	MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	if (!MultiplayerSessionsSubsystem)
		return;

	MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
	MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
	MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
	MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
	MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
		return false;

	if (!HostButton || !JoinButton)
		return false;

	HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);

	return true;
}

void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (!GEngine)
		return;

	if (!bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString(TEXT("Failed to create session")));
		HostButton->SetIsEnabled(true);
		return;
	}

	if (const auto World = GetWorld())
		World->ServerTravel(PathToLobby);
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
		return;

	for (auto Result : SessionResults)
	{
		if (!Result.IsValid())
			continue;

		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue != MatchType)
			continue;

		MultiplayerSessionsSubsystem->JoinSession(Result);
		return;
	}

	if (!bWasSuccessful || SessionResults.Num() <= 0)
		JoinButton->SetIsEnabled(true);
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	const auto Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem)
		return;

	const auto SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
		return;

	FString Address;
	SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

	const auto PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (!PlayerController)
		return;

	PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);

	if (Result == EOnJoinSessionCompleteResult::Success)
		return;

	JoinButton->SetIsEnabled(true);
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);

	if (!MultiplayerSessionsSubsystem)
		return;

	MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);

	if (!MultiplayerSessionsSubsystem)
		return;

	MultiplayerSessionsSubsystem->FindSessions(10000);
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();

	const auto World = GetWorld();
	if (!World)
		return;

	const auto PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
		return;

	const FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);
	PlayerController->bShowMouseCursor = false;
}
