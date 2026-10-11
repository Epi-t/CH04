#include "NBChatInput.h"
#include "Components/EditableTextBox.h"
#include "NBPlayerController.h"

void UNBChatInput::NativeConstruct()
{
    Super::NativeConstruct();
    if (IsValid(EditableTextBox_ChatInput) &&
        !EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted))
    {
        EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &ThisClass::OnChatInputTextCommitted);
    }
}

void UNBChatInput::NativeDestruct()
{
    if (IsValid(EditableTextBox_ChatInput))
    {
        EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnChatInputTextCommitted);
    }
    Super::NativeDestruct();
}

void UNBChatInput::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod != ETextCommit::OnEnter || !IsValid(EditableTextBox_ChatInput))
    {
        return;
    }

    const FString Message = Text.ToString().TrimStartAndEnd();
    if (Message.IsEmpty())
    {
        return;
    }

    if (ANBPlayerController* PC = Cast<ANBPlayerController>(GetOwningPlayer()))
    {
        PC->SetChatMessageString(Message);
        EditableTextBox_ChatInput->SetText(FText::GetEmpty());
    }
}
