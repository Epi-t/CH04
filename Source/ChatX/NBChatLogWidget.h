#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NBChatLogWidget.generated.h"

class UScrollBox;

// Dedicated local chat UI. Avoids shared GEngine screen debug messages in multi-PIE.
UCLASS()
class CHATX_API UNBChatLogWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void AppendMessage(const FString& Message);

protected:
    virtual void NativeOnInitialized() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UScrollBox> MessageScrollBox;
};
