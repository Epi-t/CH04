#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NBMatchStatusWidget.generated.h"

class UTextBlock;

// Builds a simple status overlay in C++, so existing .uasset widgets need no edits.
UCLASS()
class CHATX_API UNBMatchStatusWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> TurnText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> AttemptsText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> HelpText;
};
