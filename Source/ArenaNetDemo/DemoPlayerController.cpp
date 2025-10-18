// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoPlayerController.h"

#include "Components/TextBlock.h"

ADemoPlayerController::ADemoPlayerController()
{
    static ConstructorHelpers::FClassFinder<UUserWidget> MatchResultBPClass(
        TEXT("/Game/UI/WBP_MatchResult.WBP_MatchResult_C"));

    if (MatchResultBPClass.Succeeded())
    {
        MatchResultClass = MatchResultBPClass.Class;
        UE_LOG(LogTemp, Warning, TEXT("MatchResultClass 加载成功！"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MatchResultClass 加载失败！"));
    }
}

void ADemoPlayerController::BeginPlay()
{
    Super::BeginPlay();

}


void ADemoPlayerController::Client_ShowMatchResult_Implementation(const FString &WinnerName, int32 WinnerKills,
                                                                  int32          WinnerDeaths)
{

    UE_LOG(LogTemp, Warning, TEXT("Client_ShowMatchResult called on PC=%s, Winner=%s, K=%d, D=%d"),
           *GetNameSafe(this),
           *WinnerName,
           WinnerKills,
           WinnerDeaths);

    if (!MatchResultClass)
        return;

    UUserWidget *ResultUI = CreateWidget<UUserWidget>(this, MatchResultClass);
    ResultUI->SetVisibility(ESlateVisibility::Visible);
    if (ResultUI)
    {
        ResultUI->AddToViewport();
        UE_LOG(LogTemp, Warning, TEXT("UI 已添加到视口"));
        UTextBlock *WinnerText = Cast<UTextBlock>(ResultUI->GetWidgetFromName(TEXT("Text_Winner")));
        if (WinnerText)
        {
            WinnerText->SetText(FText::FromString(FString::Printf(TEXT("赢家：%s"), *WinnerName)));
        }

        UTextBlock *KDAText = Cast<UTextBlock>(ResultUI->GetWidgetFromName(TEXT("Text_KDA")));
        if (KDAText)
        {
            KDAText->SetText(FText::FromString(FString::Printf(TEXT("击杀：%d / 死亡：%d"), WinnerKills, WinnerDeaths)));
        }
    }
}