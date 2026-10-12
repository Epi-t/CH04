#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NBRoundResultWidget.generated.h"

class UTextBlock;

// C++ owns the presentation data; the Widget Blueprint owns the layout and style.
// Create WBP_RoundResult with parent class NBRoundResultWidget and the exact TextBlock names.
UCLASS()
class CHATX_API UNBRoundResultWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Number Baseball|Result")
    void ShowResult(const FText& InTitle, const FText& InDescription, const FString& InSecretNumber);

    UFUNCTION(BlueprintCallable, Category="Number Baseball|Result")
    void HideResult();

protected:
    virtual void NativeConstruct() override;

    // UMG designer elements with Is Variable enabled.
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UTextBlock> ResultTitleText;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UTextBlock> ResultDescriptionText;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UTextBlock> SecretNumberText;
};
