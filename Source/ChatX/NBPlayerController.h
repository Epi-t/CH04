#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NBPlayerController.generated.h"

class UNBChatInput;
class UNBMatchStatusWidget;
class UNBChatLogWidget;
class UUserWidget;

UCLASS()
class CHATX_API ANBPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ANBPlayerController();
    virtual void BeginPlay() override;

    // Called only by the local chat widget. The server adds the real player name.
    void SetChatMessageString(const FString& InChatMessageString);
    void PrintChatMessageString(const FString& InChatMessageString);

    UFUNCTION(Client, Reliable)
    void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

    UFUNCTION(Server, Reliable)
    void ServerRPCPrintChatMessageString(const FString& InChatMessageString);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UPROPERTY(EditDefaultsOnly, Category="Number Baseball|UI")
    TSubclassOf<UNBChatInput> ChatInputWidgetClass;

    UPROPERTY()
    TObjectPtr<UNBChatInput> ChatInputWidgetInstance;

    UPROPERTY(EditDefaultsOnly, Category="Number Baseball|UI")
    TSubclassOf<UUserWidget> NotificationTextWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> NotificationTextWidgetInstance;

    UPROPERTY()
    TObjectPtr<UNBMatchStatusWidget> MatchStatusWidgetInstance;

    UPROPERTY()
    TObjectPtr<UNBChatLogWidget> ChatLogWidgetInstance;

    TArray<FString> PendingChatMessages;

public:
    // Existing WBP_NotificationText may bind to this Text property.
    UPROPERTY(Replicated, BlueprintReadOnly, Category="Number Baseball|UI")
    FText NotificationText;
};
