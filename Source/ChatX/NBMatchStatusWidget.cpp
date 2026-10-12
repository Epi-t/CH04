#include "NBMatchStatusWidget.h"
#include "NBGameStateBase.h"
#include "NBPlayerController.h"
#include "NBPlayerState.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/World.h"

void UNBMatchStatusWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!WidgetTree)
    {
        return;
    }
    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("NBRoot"));
    WidgetTree->RootWidget = Root;
    Root->SetVisibility(ESlateVisibility::HitTestInvisible);
    UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NBStatusColumn"));
    if (UCanvasPanelSlot* CanvasSlot = Root->AddChildToCanvas(Column))
    {
        CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
        CanvasSlot->SetPosition(FVector2D(16.f, 55.f));
        CanvasSlot->SetAutoSize(true);
    }

    TurnText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NBActiveTurn"));
    AttemptsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NBAttempts"));
    HelpText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NBHelp"));

    Column->AddChildToVerticalBox(TurnText);
    Column->AddChildToVerticalBox(AttemptsText);
    Column->AddChildToVerticalBox(HelpText);

    HelpText->SetText(FText::FromString(TEXT("숫자 입력: 1~9, 서로 다른 3자리 | 채팅: 일반 문장")));
}

void UNBMatchStatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    if (!IsValid(TurnText) || !IsValid(AttemptsText) || !GetWorld())
    {
        return;
    }

    const ANBGameStateBase* GS = GetWorld()->GetGameState<ANBGameStateBase>();
    const ANBPlayerController* PC = Cast<ANBPlayerController>(GetOwningPlayer());
    const ANBPlayerState* PS = PC ? PC->GetPlayerState<ANBPlayerState>() : nullptr;

    TurnText->SetText(GS ? GS->GetTurnStatusText() : FText::FromString(TEXT("서버에 접속 중...")));
    if (!PS)
    {
        AttemptsText->SetText(FText::FromString(TEXT("플레이어 정보 대기 중...")));
        return;
    }
    const bool bMyTurn = GS && GS->bRoundActive && GS->RemainingTurnSeconds > 0
        && GS->ActivePlayerName == PS->PlayerNameString;

    AttemptsText->SetText(FText::FromString(FString::Printf(TEXT("%s | %s"),
        *PS->GetAttemptInfoText().ToString(), bMyTurn ? TEXT("내 턴") : TEXT("대기 중"))));
}
