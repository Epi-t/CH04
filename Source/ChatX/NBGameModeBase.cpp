#include "NBGameModeBase.h"
#include "NBGameStateBase.h"
#include "NBPlayerController.h"
#include "NBPlayerState.h"
#include "NBPawn.h"
#include "Engine/World.h"
#include "TimerManager.h"

ANBGameModeBase::ANBGameModeBase()
{
    GameStateClass = ANBGameStateBase::StaticClass();
    PlayerControllerClass = ANBPlayerController::StaticClass();
    PlayerStateClass = ANBPlayerState::StaticClass();
    DefaultPawnClass = ANBPawn::StaticClass();
}

void ANBGameModeBase::BeginPlay()
{
    Super::BeginPlay();
    // GameState starts in waiting state. Do not reset it here: PostLogin may
    // already have started the first round before BeginPlay during PIE.
}

void ANBGameModeBase::OnPostLogin(AController* NewPlayer)
{
    Super::OnPostLogin(NewPlayer);

    ANBPlayerController* PC = Cast<ANBPlayerController>(NewPlayer);
    if (!IsValid(PC))
    {
        return;
    }

    const TWeakObjectPtr<ANBPlayerController> WeakPC(PC);
    AllPlayerControllers.AddUnique(WeakPC);
    // IMPORTANT: check the PlayerState before dereferencing it (fixes the original login crash).
    ANBPlayerState* PS = PC->GetPlayerState<ANBPlayerState>();
    if (!IsValid(PS))
    {
        AllPlayerControllers.Remove(WeakPC);
        UE_LOG(LogTemp, Warning, TEXT("NBGameMode: missing NBPlayerState on login. Check GameMode PlayerStateClass."));
        return;
    }

    ++NextPlayerNumber;
    PS->PlayerNameString = FString::Printf(TEXT("Player%d"), NextPlayerNumber);
    PS->MaxGuessCount = MaxAttemptsPerPlayer;
    PS->CurrentGuessCount = 0;
    PS->bSubmittedGuessThisTurn = false;
    PC->NotificationText = FText::FromString(TEXT("게임 서버에 접속했습니다."));

    BroadcastMessage(PS->PlayerNameString + TEXT(" 님이 입장했습니다."));

    if (ANBGameStateBase* GS = GetGameState<ANBGameStateBase>())
    {
        if (!GS->bRoundActive && !bRoundEnding && AllPlayerControllers.Num() >= MinimumPlayersToStart)
        {
            ResetGame();
        }
        else if (!GS->bRoundActive && !bRoundEnding)
        {
            SetNotificationForAll(TEXT("다른 플레이어를 기다리는 중입니다."));
        }
    }
}

void ANBGameModeBase::Logout(AController* Exiting)
{
    ANBPlayerController* PC = Cast<ANBPlayerController>(Exiting);
    int32 RemovedIndex = INDEX_NONE;
    const bool bWasCurrentTurn = (PC && GetCurrentTurnPlayer() == PC);
    if (PC)
    {
        for (int32 i = 0; i < AllPlayerControllers.Num(); ++i)
        {
            if (AllPlayerControllers[i].Get() == PC)
            {
                RemovedIndex = i;
                AllPlayerControllers.RemoveAt(i);
                break;
            }
        }
    }
    Super::Logout(Exiting);

    if (RemovedIndex == INDEX_NONE)
    {
        return;
    }
    if (AllPlayerControllers.Num() < MinimumPlayersToStart)
    {
        GetWorldTimerManager().ClearTimer(TurnTimerHandle);
        GetWorldTimerManager().ClearTimer(ResetTimerHandle);
        bRoundEnding = false;
        CurrentTurnIndex = INDEX_NONE;
        EnterWaitingState();
        SetNotificationForAll(TEXT("플레이어가 나갔습니다. 다른 플레이어를 기다립니다."));
        return;
    }
    if (bRoundEnding)
    {
        return;
    }
    if (bWasCurrentTurn)
    {
        GetWorldTimerManager().ClearTimer(TurnTimerHandle);
        StartTurn(RemovedIndex % AllPlayerControllers.Num());
    }
    else if (RemovedIndex < CurrentTurnIndex)
    {
        --CurrentTurnIndex;
    }
}

FString ANBGameModeBase::GenerateSecretNumber()
{
    TArray<int32> Digits;
    for (int32 Digit = 1; Digit <= 9; ++Digit)
    {
        Digits.Add(Digit);
    }

    FString Result;
    for (int32 i = 0; i < 3; ++i)
    {
        const int32 PickIndex = FMath::RandRange(0, Digits.Num() - 1);
        Result += FString::FromInt(Digits[PickIndex]);
        Digits.RemoveAtSwap(PickIndex);
    }
    return Result;
}

bool ANBGameModeBase::IsGuessNumberString(const FString& Input) const
{
    if (Input.Len() != 3)
    {
        return false;
    }
    TSet<TCHAR> Used;
    for (const TCHAR Digit : Input)
    {
        if (Digit < TEXT('1') || Digit > TEXT('9') || Used.Contains(Digit))
        {
            return false;
        }
        Used.Add(Digit);
    }
    return true;
}

FString ANBGameModeBase::JudgeResult(const FString& Secret, const FString& Guess) const
{
    if (Secret.Len() != 3 || Guess.Len() != 3)
    {
        return TEXT("INVALID");
    }
    int32 Strikes = 0;
    int32 Balls = 0;
    for (int32 i = 0; i < 3; ++i)
    {
        if (Guess[i] == Secret[i])
        {
            ++Strikes;
        }
        else if (Secret.Contains(FString::Chr(Guess[i])))
        {
            ++Balls;
        }
    }
    return (Strikes + Balls == 0) ? TEXT("OUT") :
        FString::Printf(TEXT("%dS%dB"), Strikes, Balls);
}

void ANBGameModeBase::PrintChatMessageString(ANBPlayerController* Sender, const FString& RawInput)
{
    if (!IsValid(Sender))
    {
        return;
    }
    ANBPlayerState* PS = Sender->GetPlayerState<ANBPlayerState>();
    if (!IsValid(PS))
    {
        return;
    }

    FString Message = RawInput.Left(140).TrimStartAndEnd();
    if (Message.IsEmpty())
    {
        return;
    }

    // Digits or /guess <input> are guesses; normal sentences remain chat.
    const bool bExplicitGuess = Message.Equals(TEXT("/guess"), ESearchCase::IgnoreCase)
        || Message.StartsWith(TEXT("/guess "), ESearchCase::IgnoreCase);
    FString Guess = bExplicitGuess ? Message.RightChop(6).TrimStartAndEnd() : Message;
    bool bContainsNumber = false;
    for (const TCHAR Ch : Message)
    {
        if (Ch >= TEXT('0') && Ch <= TEXT('9'))
        {
            bContainsNumber = true;
            break;
        }
    }
    const bool bGuessInput = bExplicitGuess || (bContainsNumber && !Message.Contains(TEXT(" ")));

    if (!bGuessInput)
    {
        BroadcastMessage(PS->PlayerNameString + TEXT(": ") + Message);
        return;
    }

    const ANBGameStateBase* GS = GetGameState<ANBGameStateBase>();
    if (!GS || !GS->bRoundActive || bRoundEnding)
    {
        SendPlayerMessage(Sender, TEXT("지금은 숫자 야구를 진행할 수 없습니다. 채팅은 가능합니다."));
        return;
    }
    if (GetCurrentTurnPlayer() != Sender)
    {
        SendPlayerMessage(Sender, TEXT("현재 본인의 턴이 아닙니다."));
        return;
    }
    if (GS->RemainingTurnSeconds <= 0)
    {
        SendPlayerMessage(Sender, TEXT("시간이 종료되어 숫자를 입력할 수 없습니다."));
        return;
    }
    if (PS->CurrentGuessCount >= PS->MaxGuessCount || PS->bSubmittedGuessThisTurn)
    {
        SendPlayerMessage(Sender, TEXT("현재 사용할 수 있는 기회가 없습니다."));
        return;
    }
    if (!IsGuessNumberString(Guess))
    {
        FString Reason;
        if (Guess.Len() != 3)
        {
            Reason = TEXT("3자리 숫자를 입력해주세요.");
        }
        else
        {
            TSet<TCHAR> Digits;
            bool bBadDigit = false;
            for (const TCHAR Ch : Guess)
            {
                if (Ch < TEXT('1') || Ch > TEXT('9'))
                {
                    bBadDigit = true;
                    break;
                }
                Digits.Add(Ch);
            }
            Reason = bBadDigit ? TEXT("1부터 9까지의 숫자만 입력해주세요.")
                : TEXT("중복되지 않은 숫자를 입력해주세요.");
        }
        SendPlayerMessage(Sender, Reason + TEXT(" 다시 입력하세요. (기회 차감 없음)"));
        return;
    }

    // Only the SERVER can spend an attempt or judge the answer.
    GetWorldTimerManager().ClearTimer(TurnTimerHandle);
    PS->bSubmittedGuessThisTurn = true;
    IncreaseGuessCount(Sender);
    const FString Result = JudgeResult(SecretNumberString, Guess);
    BroadcastMessage(FString::Printf(TEXT("%s [%d/%d]: %s -> %s"),
        *PS->PlayerNameString, PS->CurrentGuessCount, PS->MaxGuessCount, *Guess, *Result));

    // CRITICAL: judge ONCE, not inside the recipients' broadcast loop.
    JudgeGame(Sender, Result == TEXT("3S0B") ? 3 : 0);
}

void ANBGameModeBase::IncreaseGuessCount(ANBPlayerController* Sender)
{
    if (IsValid(Sender))
    {
        if (ANBPlayerState* PS = Sender->GetPlayerState<ANBPlayerState>())
        {
            PS->CurrentGuessCount = FMath::Min(PS->CurrentGuessCount + 1, PS->MaxGuessCount);
        }
    }
}

void ANBGameModeBase::JudgeGame(ANBPlayerController* Sender, int32 StrikeCount)
{
    if (bRoundEnding || !IsValid(Sender))
    {
        return;
    }
    if (StrikeCount == 3)
    {
        if (const ANBPlayerState* PS = Sender->GetPlayerState<ANBPlayerState>())
        {
            FinishRound(PS->PlayerNameString + TEXT(" 승리!"));
        }
    }
    else if (HaveAllPlayersUsedAttempts())
    {
        FinishRound(TEXT("무승부! 모든 플레이어가 기회를 소진했습니다."));
    }
    else
    {
        AdvanceTurn();
    }
}

void ANBGameModeBase::ResetGame()
{
    GetWorldTimerManager().ClearTimer(TurnTimerHandle);
    GetWorldTimerManager().ClearTimer(ResetTimerHandle);
    bRoundEnding = false;
    CurrentTurnIndex = INDEX_NONE;

    if (AllPlayerControllers.Num() < MinimumPlayersToStart)
    {
        EnterWaitingState();
        return;
    }

    SecretNumberString = GenerateSecretNumber();
    UE_LOG(LogTemp, Log, TEXT("NBGameMode: new secret generated (not sent to clients)."));
    for (const TWeakObjectPtr<ANBPlayerController>& PCWeak : AllPlayerControllers)
    {
        if (ANBPlayerController* PC = PCWeak.Get())
        {
            if (ANBPlayerState* PS = PC->GetPlayerState<ANBPlayerState>())
            {
                PS->CurrentGuessCount = 0;
                PS->MaxGuessCount = MaxAttemptsPerPlayer;
                PS->bSubmittedGuessThisTurn = false;
            }
        }
    }
    BroadcastMessage(TEXT("새 라운드 시작! 서로 다른 1~9 숫자 세 자리를 맞혀보세요."));
    StartTurn(0);
}

ANBPlayerController* ANBGameModeBase::GetCurrentTurnPlayer() const
{
    return AllPlayerControllers.IsValidIndex(CurrentTurnIndex)
        ? AllPlayerControllers[CurrentTurnIndex].Get() : nullptr;
}

void ANBGameModeBase::StartTurn(int32 StartIndex)
{
    GetWorldTimerManager().ClearTimer(TurnTimerHandle);
    if (bRoundEnding || AllPlayerControllers.IsEmpty())
    {
        return;
    }

    const int32 Count = AllPlayerControllers.Num();
    for (int32 Offset = 0; Offset < Count; ++Offset)
    {
        const int32 Index = (StartIndex + Offset) % Count;
        ANBPlayerController* PC = AllPlayerControllers[Index].Get();
        ANBPlayerState* PS = IsValid(PC) ? PC->GetPlayerState<ANBPlayerState>() : nullptr;
        if (!IsValid(PS) || PS->CurrentGuessCount >= PS->MaxGuessCount)
        {
            continue;
        }
        CurrentTurnIndex = Index;
        PS->bSubmittedGuessThisTurn = false;
        if (ANBGameStateBase* GS = GetGameState<ANBGameStateBase>())
        {
            GS->ActivePlayerName = PS->PlayerNameString;
            GS->RemainingTurnSeconds = TurnDurationSeconds;
            GS->bRoundActive = true;
            GS->ForceNetUpdate();
        }
        SetNotificationForAll(FString::Printf(TEXT("%s님의 턴입니다."), *PS->PlayerNameString));
        GetWorldTimerManager().SetTimer(TurnTimerHandle, this, &ANBGameModeBase::OnTurnTimerTick, 1.f, true);
        return;
    }

    FinishRound(TEXT("무승부! 모든 플레이어가 기회를 소진했습니다."));
}

void ANBGameModeBase::AdvanceTurn()
{
    GetWorldTimerManager().ClearTimer(TurnTimerHandle);
    if (AllPlayerControllers.IsEmpty())
    {
        EnterWaitingState();
        return;
    }
    StartTurn((CurrentTurnIndex + 1) % AllPlayerControllers.Num());
}

void ANBGameModeBase::OnTurnTimerTick()
{
    ANBGameStateBase* GS = GetGameState<ANBGameStateBase>();
    if (!GS || !GS->bRoundActive || bRoundEnding)
    {
        GetWorldTimerManager().ClearTimer(TurnTimerHandle);
        return;
    }
    GS->RemainingTurnSeconds = FMath::Max(0, GS->RemainingTurnSeconds - 1);
    GS->ForceNetUpdate();
    if (GS->RemainingTurnSeconds > 0)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(TurnTimerHandle);
    ANBPlayerController* CurrentPC = GetCurrentTurnPlayer();
    ANBPlayerState* PS = IsValid(CurrentPC) ? CurrentPC->GetPlayerState<ANBPlayerState>() : nullptr;
    if (IsValid(PS) && !PS->bSubmittedGuessThisTurn)
    {
        PS->bSubmittedGuessThisTurn = true;
        IncreaseGuessCount(CurrentPC); // Timeout consumes exactly one attempt.
        BroadcastMessage(FString::Printf(TEXT("%s 시간 초과! [%d/%d]"),
            *PS->PlayerNameString, PS->CurrentGuessCount, PS->MaxGuessCount));
    }

    if (HaveAllPlayersUsedAttempts())
    {
        FinishRound(TEXT("무승부! 모든 플레이어가 기회를 소진했습니다."));
    }
    else
    {
        AdvanceTurn();
    }
}

bool ANBGameModeBase::HaveAllPlayersUsedAttempts() const
{
    if (AllPlayerControllers.IsEmpty())
    {
        return false;
    }
    for (const TWeakObjectPtr<ANBPlayerController>& WeakPC : AllPlayerControllers)
    {
        const ANBPlayerController* PC = WeakPC.Get();
        const ANBPlayerState* PS = IsValid(PC) ? PC->GetPlayerState<ANBPlayerState>() : nullptr;
        if (IsValid(PS) && PS->CurrentGuessCount < PS->MaxGuessCount)
        {
            return false;
        }
    }
    return true;
}

void ANBGameModeBase::FinishRound(const FString& ResultText)
{
    if (bRoundEnding)
    {
        return;
    }
    bRoundEnding = true;
    GetWorldTimerManager().ClearTimer(TurnTimerHandle);
    if (ANBGameStateBase* GS = GetGameState<ANBGameStateBase>())
    {
        GS->bRoundActive = false;
        GS->RemainingTurnSeconds = 0;
        GS->ActivePlayerName = TEXT("");
        GS->ForceNetUpdate();
    }
    BroadcastMessage(ResultText);
    SetNotificationForAll(ResultText);

    // Keep the result widget visible before changing the answer and counters.
    GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &ANBGameModeBase::ResetGame,
        RoundResetDelaySeconds, false);
}

void ANBGameModeBase::BroadcastMessage(const FString& Message) const
{
    for (const TWeakObjectPtr<ANBPlayerController>& WeakPC : AllPlayerControllers)
    {
        if (ANBPlayerController* PC = WeakPC.Get())
        {
            PC->ClientRPCPrintChatMessageString(Message);
        }
    }
}

void ANBGameModeBase::SendPlayerMessage(ANBPlayerController* Receiver, const FString& Message) const
{
    if (IsValid(Receiver))
    {
        Receiver->ClientRPCPrintChatMessageString(Message);
    }
}

void ANBGameModeBase::SetNotificationForAll(const FString& Message) const
{
    for (const TWeakObjectPtr<ANBPlayerController>& WeakPC : AllPlayerControllers)
    {
        if (ANBPlayerController* PC = WeakPC.Get())
        {
            PC->NotificationText = FText::FromString(Message);
            PC->ForceNetUpdate();
        }
    }
}

void ANBGameModeBase::EnterWaitingState()
{
    if (ANBGameStateBase* GS = GetGameState<ANBGameStateBase>())
    {
        GS->bRoundActive = false;
        GS->ActivePlayerName = TEXT("");
        GS->RemainingTurnSeconds = 0;
        GS->ForceNetUpdate();
    }
}
