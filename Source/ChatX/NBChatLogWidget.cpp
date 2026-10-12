
#include "NBChatLogWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void UNBChatLogWidget::AppendMessage(const FString& Message)
{
    if (!IsValid(MessageScrollBox) || !WidgetTree)
    {
        return;
    }

    UTextBlock* Line =
        WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass());

    if (!IsValid(Line))
    {
        return;
    }

    Line->SetText(FText::FromString(Message));
    Line->SetAutoWrapText(true);
    Line->SetJustification(ETextJustify::Left);

    FSlateFontInfo FontInfo = Line->GetFont();
    FontInfo.Size = ChatFontSize;
    Line->SetFont(FontInfo);

    MessageScrollBox->AddChild(Line);

    constexpr int32 MaxVisibleMessages = 15;

    if (MessageScrollBox->GetChildrenCount()
        > MaxVisibleMessages)
    {
        MessageScrollBox->RemoveChildAt(0);
    }

    MessageScrollBox->ScrollToEnd();
}
