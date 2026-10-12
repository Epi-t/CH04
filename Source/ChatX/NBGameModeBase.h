#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "NBGameModeBase.generated.h"

class ANBPlayerController;
class ANBPlayerState;

// Server-authoritative number baseball + chat + turn timer.
UCLASS()
class CHATX_API ANBGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    ANBGameModeBase();
    virtual void BeginPlay() override;
    virtual void OnPostLogin(AController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    // Called exclusively by a server RPC on the owning player controller.
    void PrintChatMessageString(ANBPlayerController* Sender, const FString& RawInput);

    FString GenerateSecretNumber();
    bool IsGuessNumberString(const FString& Input) const;
    FString JudgeResult(const FString& Secret, const FString& Guess) const;
    void IncreaseGuessCount(ANBPlayerController* Sender);
    void JudgeGame(ANBPlayerController* Sender, int32 StrikeCount);
    void ResetGame();

protected:
    UPROPERTY(EditDefaultsOnly, Category="Number Baseball|Rules", meta=(ClampMin="1"))
    int32 MinimumPlayersToStart = 2;

    UPROPERTY(EditDefaultsOnly, Category="Number Baseball|Rules", meta=(ClampMin="1"))
    int32 MaxAttemptsPerPlayer = 3;

    UPROPERTY(EditDefaultsOnly, Category="Number Baseball|Rules", meta=(ClampMin="1"))
    int32 TurnDurationSeconds = 20;

    UPROPERTY(EditDefaultsOnly, Category="Number Baseball|Rules", meta=(ClampMin="0.1"))
    float RoundResetDelaySeconds = 5.f;

    // Intentionally server-only: never replicate the answer.
    FString SecretNumberString;

private:
    TArray<TWeakObjectPtr<ANBPlayerController>> AllPlayerControllers;
    int32 NextPlayerNumber = 0;
    int32 CurrentTurnIndex = INDEX_NONE;
    bool bRoundEnding = false;
    FTimerHandle TurnTimerHandle;
    FTimerHandle ResetTimerHandle;

    ANBPlayerController* GetCurrentTurnPlayer() const;
    void StartTurn(int32 StartIndex);
    void AdvanceTurn();
    void OnTurnTimerTick();
    void FinishRound(const FString& ResultText, bool bIsDraw);
    bool HaveAllPlayersUsedAttempts() const;
    void BroadcastMessage(const FString& Message) const;
    void SendPlayerMessage(ANBPlayerController* Receiver, const FString& Message) const;
    void SetNotificationForAll(const FString& Message) const;
    void EnterWaitingState();
};
