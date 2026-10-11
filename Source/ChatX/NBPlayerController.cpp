#include "NBPlayerController.h"
#include "NBGameModeBase.h"
#include "NBChatInput.h"
#include "NBMatchStatusWidget.h"
#include "NBChatLogWidget.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"

ANBPlayerController::ANBPlayerController()
{
    bReplicates = true;
}

void ANBPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController())
    {
        return;
    }

    FInputModeUIOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = true;

    // Per-player UMG chat feed instead of shared AddOnScreenDebugMessage.
    ChatLogWidgetInstance = CreateWidget<UNBChatLogWidget>(this, UNBChatLogWidget::StaticClass());
    if (IsValid(ChatLogWidgetInstance))
    {
        ChatLogWidgetInstance->AddToViewport(1);
        for (const FString& Pending : PendingChatMessages)
        {
            ChatLogWidgetInstance->AppendMessage(Pending);
        }
        PendingChatMessages.Empty();
    }

    if (ChatInputWidgetClass)
    {
        ChatInputWidgetInstance = CreateWidget<UNBChatInput>(this, ChatInputWidgetClass);
        if (IsValid(ChatInputWidgetInstance))
        {
            ChatInputWidgetInstance->AddToViewport();
        }
    }
    if (NotificationTextWidgetClass)
    {
        NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
        if (IsValid(NotificationTextWidgetInstance))
        {
            NotificationTextWidgetInstance->AddToViewport(3);
        }
    }

    // Native UI: no additional WBP is required for the replicated timer/attempt HUD.
    MatchStatusWidgetInstance = CreateWidget<UNBMatchStatusWidget>(this, UNBMatchStatusWidget::StaticClass());
    if (IsValid(MatchStatusWidgetInstance))
    {
        MatchStatusWidgetInstance->AddToViewport(2);
    }
}

void ANBPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
    if (IsLocalController())
    {
        // Send RAW input, not a client-generated name/attempt count.
        ServerRPCPrintChatMessageString(InChatMessageString);
    }
}

void ANBPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
    if (IsValid(ChatLogWidgetInstance))
    {
        ChatLogWidgetInstance->AppendMessage(InChatMessageString);
    }
    else
    {
        // The login message can arrive before the local widget is constructed.
        PendingChatMessages.Add(InChatMessageString);
    }
}

void ANBPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
    PrintChatMessageString(InChatMessageString);
}

void ANBPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
    if (ANBGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ANBGameModeBase>() : nullptr)
    {
        GM->PrintChatMessageString(this, InChatMessageString);
    }
}

void ANBPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(ANBPlayerController, NotificationText, COND_OwnerOnly);
}
