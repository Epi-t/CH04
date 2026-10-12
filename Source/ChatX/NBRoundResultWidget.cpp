#include "NBRoundResultWidget.h"
#include "Components/TextBlock.h"

void UNBRoundResultWidget::NativeConstruct()
{
    Super::NativeConstruct();
    HideResult();
}

void UNBRoundResultWidget::ShowResult(
    const FText& InTitle, const FText& InDescription, const FString& InSecretNumber)
{
    if (!IsValid(ResultTitleText) || !IsValid(ResultDescriptionText) || !IsValid(SecretNumberText))
    {
        UE_LOG(LogTemp, Warning, TEXT("WBP_RoundResult: missing required TextBlock bindings."));
        return;
    }

    ResultTitleText->SetText(InTitle);
    ResultDescriptionText->SetText(InDescription);
    SecretNumberText->SetText(FText::FromString(
        FString::Printf(TEXT("정답 숫자 : %s"), *InSecretNumber)));
    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UNBRoundResultWidget::HideResult()
{
    SetVisibility(ESlateVisibility::Collapsed);
}
