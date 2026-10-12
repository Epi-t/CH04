
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NBChatLogWidget.generated.h"

class UScrollBox;

UCLASS()
class CHATX_API UNBChatLogWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void AppendMessage(const FString& Message);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UScrollBox> MessageScrollBox;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,
        Category = "Chat UI")
    int32 ChatFontSize = 13;
};
