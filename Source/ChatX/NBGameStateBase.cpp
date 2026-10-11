#include "NBGameStateBase.h"
#include "NBPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ANBGameStateBase::ANBGameStateBase()
    : RemainingTurnSeconds(0)
    , ActivePlayerName(TEXT(""))
    , bRoundActive(false)
{
    bReplicates = true;
}

void ANBGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ANBGameStateBase, RemainingTurnSeconds);
    DOREPLIFETIME(ANBGameStateBase, ActivePlayerName);
    DOREPLIFETIME(ANBGameStateBase, bRoundActive);
}

FText ANBGameStateBase::GetTurnStatusText() const
{
    if (!bRoundActive)
    {
        return FText::FromString(TEXT("대기 중 / 라운드 종료"));
    }
    return FText::FromString(FString::Printf(TEXT("현재 턴: %s | 남은 시간: %d초"),
        *ActivePlayerName, RemainingTurnSeconds));
}

void ANBGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
    // This legacy RPC is not used by the new GameMode to prevent duplicate notices.
    // An owning controller exists on clients and on a listen server, not on a dedicated server.
    if (GetNetMode() == NM_DedicatedServer)
    {
        return;
    }
    if (ANBPlayerController* LocalPC = Cast<ANBPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
    {
        LocalPC->PrintChatMessageString(InNameString + TEXT(" has joined the game."));
    }
}
