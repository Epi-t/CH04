#include "NBPlayerState.h"
#include "Net/UnrealNetwork.h"

ANBPlayerState::ANBPlayerState()
    : PlayerNameString(TEXT("Waiting"))
    , CurrentGuessCount(0)
    , MaxGuessCount(3)
    , bSubmittedGuessThisTurn(false)
{
    bReplicates = true;
}

void ANBPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ANBPlayerState, PlayerNameString);
    DOREPLIFETIME(ANBPlayerState, CurrentGuessCount);
    DOREPLIFETIME(ANBPlayerState, MaxGuessCount);
    DOREPLIFETIME(ANBPlayerState, bSubmittedGuessThisTurn);
}

FString ANBPlayerState::GetPlayerInfoString() const
{
    return FString::Printf(TEXT("%s [%d/%d]"), *PlayerNameString, CurrentGuessCount, MaxGuessCount);
}

FText ANBPlayerState::GetAttemptInfoText() const
{
    return FText::FromString(FString::Printf(TEXT("시도 횟수: %d / %d"), CurrentGuessCount, MaxGuessCount));
}
