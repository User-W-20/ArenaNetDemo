// Fill out your copyright notice in the Description page of Project Settings.


#include "ADemoGameState.h"

AADemoGameState::AADemoGameState()
{
    CurrentMatchState = EMatchState::WaitingToStart;
}

void AADemoGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AADemoGameState, CurrentMatchState);
    DOREPLIFETIME(AADemoGameState, WinningPlayerState);
}