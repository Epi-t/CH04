#include "NBPawn.h"
#include "ChatX.h"

void ANBPawn::BeginPlay()
{
    Super::BeginPlay();
    const FString Msg = FString::Printf(TEXT("NBPawn::BeginPlay() %s [%s]"),
        *ChatXFunctionLibrary::GetNetModeString(this), *ChatXFunctionLibrary::GetRoleString(this));
    ChatXFunctionLibrary::MyPrintString(this, Msg, 4.f);
}

void ANBPawn::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    const FString Msg = FString::Printf(TEXT("NBPawn::PossessedBy() %s [%s]"),
        *ChatXFunctionLibrary::GetNetModeString(this), *ChatXFunctionLibrary::GetRoleString(this));
    ChatXFunctionLibrary::MyPrintString(this, Msg, 4.f);
}
