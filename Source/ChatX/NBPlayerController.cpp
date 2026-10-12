#include "NBPlayerController.h"
#include "NBGameModeBase.h"
#include "NBChatInput.h"
#include "NBMatchStatusWidget.h"
#include "NBChatLogWidget.h"
#include "NBRoundResultWidget.h"
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

    if (ChatLogWidgetClass)
    {
        ChatLogWidgetInstance = CreateWidget<UNBChatLogWidget>(
            this, ChatLogWidgetClass);
    }

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


    if (RoundResultWidgetClass)
    {
        RoundResultWidgetInstance = CreateWidget<UNBRoundResultWidget>(this, RoundResultWidgetClass);
        if (IsValid(RoundResultWidgetInstance))
        {
            RoundResultWidgetInstance->AddToViewport(10);
            ApplyPendingRoundResult();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NBPlayerController: assign WBP_RoundResult to Round Result Widget Class."));
    }

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

void ANBPlayerController::ApplyPendingRoundResult()
{
    if (!bHasPendingRoundResult || !IsValid(RoundResultWidgetInstance))
    {
        return;
    }

    RoundResultWidgetInstance->ShowResult(
        FText::FromString(PendingResultTitle),
        FText::FromString(PendingResultDescription),
        PendingSecretNumber);

    if (IsValid(NotificationTextWidgetInstance))
    {
        NotificationTextWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void ANBPlayerController::ClientRPCShowRoundResult_Implementation(
    const FString& InTitle, const FString& InDescription, const FString& InSecretNumber)
{
    bHasPendingRoundResult = true;
    PendingResultTitle = InTitle;
    PendingResultDescription = InDescription;
    PendingSecretNumber = InSecretNumber;
    ApplyPendingRoundResult();
}

void ANBPlayerController::ClientRPCHideRoundResult_Implementation()
{
    bHasPendingRoundResult = false;
    PendingResultTitle.Empty();
    PendingResultDescription.Empty();
    PendingSecretNumber.Empty();

    if (IsValid(RoundResultWidgetInstance))
    {
        RoundResultWidgetInstance->HideResult();
    }
    if (IsValid(NotificationTextWidgetInstance))
    {
        NotificationTextWidgetInstance->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
}

void ANBPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(ANBPlayerController, NotificationText, COND_OwnerOnly);
}
