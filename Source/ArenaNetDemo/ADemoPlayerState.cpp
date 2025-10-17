// Fill out your copyright notice in the Description page of Project Settings.


#include "ADemoPlayerState.h"
#include  "Net/UnrealNetwork.h"

AADemoPlayerState::AADemoPlayerState()
{
    Kills=0;
    Deaths=0;
}


void AADemoPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AADemoPlayerState,Kills);
    DOREPLIFETIME(AADemoPlayerState,Deaths);
}

void AADemoPlayerState::IncrementKills(int32 Amount)
{
    if (HasAuthority())
    {
        Kills+=Amount;
    }
}

void AADemoPlayerState::IncrementDeaths(int Amount)
{
    if (HasAuthority())
    {
        Deaths+=Amount;
    }
}
