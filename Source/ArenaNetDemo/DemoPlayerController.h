// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "DemoPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ARENANETDEMO_API ADemoPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ADemoPlayerController();

    virtual void BeginPlay() override;

    UFUNCTION(Client, Reliable)
    void Client_ShowMatchResult(const FString &WinnerName, int32 WinnerKills, int32 WinnerDeaths);

protected:
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UUserWidget> MatchResultClass;
};