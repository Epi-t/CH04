#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NBPlayerController.generated.h"

class UNBChatInput;
class UNBMatchStatusWidget;
class UNBChatLogWidget;
class UNBRoundResultWidget;
class UUserWidget;

UCLASS()
class CHATX_API ANBPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ANBPlayerController();
    virtual void BeginPlay() override;

    void SetChatMessageString(const FString& InChatMessageString);
    void PrintChatMessageString(const FString& InChatMessageString);

    UFUNCTION(Client, Reliable)
    void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

    UFUNCTION(Server, Reliable)
    void ServerRPCPrintChatMessageString(const FString& InChatMessageString);

    UFUNCTION(Client, Reliable)
    void ClientRPCShowRoundResult(const FString& InTitle, const FString& InDescription,
                                  const FString& InSecretNumber);

    UFUNCTION(Client, Reliable)
    void ClientRPCHideRoundResult();

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

    UPROPERTY(EditDefaultsOnly, Category="Number Baseball|UI")
    TSubclassOf<UNBRoundResultWidget> RoundResultWidgetClass;

    UPROPERTY()
    TObjectPtr<UNBRoundResultWidget> RoundResultWidgetInstance;

    bool bHasPendingRoundResult = false;
    FString PendingResultTitle;
    FString PendingResultDescription;
    FString PendingSecretNumber;

    void ApplyPendingRoundResult();

    TArray<FString> PendingChatMessages;

    UPROPERTY(EditDefaultsOnly, Category = "Number Baseball|UI")
    TSubclassOf<UNBChatLogWidget> ChatLogWidgetClass;

public:
    UPROPERTY(Replicated, BlueprintReadOnly, Category="Number Baseball|UI")
    FText NotificationText;
};
