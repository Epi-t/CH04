#include "NBChatLogWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void UNBChatLogWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    if (!WidgetTree)
    {
        return;
    }

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("NBChatRoot"));
    WidgetTree->RootWidget = Root;
    Root->SetVisibility(ESlateVisibility::HitTestInvisible);
    MessageScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("NBMessages"));
    if (UCanvasPanelSlot* CanvasSlot = Root->AddChildToCanvas(MessageScrollBox))
    {
        CanvasSlot->SetAnchors(FAnchors(1.f, 0.f, 1.f, 0.f));
        CanvasSlot->SetAlignment(FVector2D(1.f, 0.f));
        CanvasSlot->SetPosition(FVector2D(-16.f, 55.f));
        CanvasSlot->SetSize(FVector2D(470.f, 260.f));
    }
}

void UNBChatLogWidget::AppendMessage(const FString& Message)
{
    if (!IsValid(MessageScrollBox) || !WidgetTree)
    {
        return;
    }
    UTextBlock* Line = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    Line->SetText(FText::FromString(Message));
    Line->SetAutoWrapText(true);
    MessageScrollBox->AddChild(Line);

    constexpr int32 MaxVisibleMessages = 15;
    if (MessageScrollBox->GetChildrenCount() > MaxVisibleMessages)
    {
        MessageScrollBox->RemoveChildAt(0);
    }
    MessageScrollBox->ScrollToEnd();
}
