// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "ADemoGameState.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EMatchState:uint8
{
    WaitingToStart,
    InProgress,
    Finished
};

UCLASS()
class ARENANETDEMO_API AADemoGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AADemoGameState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;


    UPROPERTY(Replicated)
    EMatchState CurrentMatchState;

    UPROPERTY(Replicated)
    APlayerState *WinningPlayerState;
};