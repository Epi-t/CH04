#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NBGameStateBase.generated.h"

// Replicated match information: clients display this, but never change it.
UCLASS()
class CHATX_API ANBGameStateBase : public AGameStateBase
{
    GENERATED_BODY()

public:
    ANBGameStateBase();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Retained for compatibility with any existing Blueprint references.
    // New login messages are sent once via each owning PlayerController instead.
    UFUNCTION(NetMulticast, Reliable)
    void MulticastRPCBroadcastLoginMessage(const FString& InNameString);

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Number Baseball")
    int32 RemainingTurnSeconds;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Number Baseball")
    FString ActivePlayerName;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Number Baseball")
    bool bRoundActive;

    UFUNCTION(BlueprintPure, Category="Number Baseball")
    FText GetTurnStatusText() const;
};
