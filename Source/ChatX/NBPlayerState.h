#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NBPlayerState.generated.h"

// Each player's attempts are owned by the server and replicated to clients.
UCLASS()
class CHATX_API ANBPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ANBPlayerState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure, Category="Number Baseball")
    FString GetPlayerInfoString() const;

    UFUNCTION(BlueprintPure, Category="Number Baseball")
    FText GetAttemptInfoText() const;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Number Baseball")
    FString PlayerNameString;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Number Baseball")
    int32 CurrentGuessCount;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Number Baseball")
    int32 MaxGuessCount;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Number Baseball")
    bool bSubmittedGuessThisTurn;
};
