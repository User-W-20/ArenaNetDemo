// Fill out your copyright notice in the Description page of Project Settings.


#include "ADemoPlayerState.h"
#include  "Net/UnrealNetwork.h"

AADemoPlayerState::AADemoPlayerState()
{
    bReplicates = true;
    Kills       = 0;
    Deaths      = 0;
}


void AADemoPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AADemoPlayerState, Kills);
    DOREPLIFETIME(AADemoPlayerState, Deaths);
}

void AADemoPlayerState::IncrementKills()
{
    if (HasAuthority())
    {
        ++Kills;
        OnRep_kills();
    }
}

void AADemoPlayerState::IncrementDeaths()
{
    if (HasAuthority())
    {
        ++Deaths;
        OnRep_Deaths();
    }
}

void AADemoPlayerState::OnRep_kills()
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] Kill Count Updated: %d"), *GetPlayerName(), Kills);
}

void AADemoPlayerState::OnRep_Deaths()
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] Death Count Updated: %d"), *GetPlayerName(), Deaths);
}
