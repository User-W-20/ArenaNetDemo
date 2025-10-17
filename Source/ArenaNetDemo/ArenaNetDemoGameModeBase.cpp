// Copyright Epic Games, Inc. All Rights Reserved.


#include "ArenaNetDemoGameModeBase.h"
#include "ADemoCharacter.h"
#include "ADemoGameState.h"
#include "AWeapon.h"
#include "ADemoPlayerState.h"

AArenaNetDemoGameModeBase::AArenaNetDemoGameModeBase()
{
    DefaultPawnClass=AADemoCharacter::StaticClass();

    PlayerStateClass=AADemoCharacter::StaticClass();

    GameStateClass=AADemoGameState::StaticClass();
}

void AArenaNetDemoGameModeBase::PostLogin(APlayerController *NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (AADemoCharacter* Character=Cast<AADemoCharacter>(NewPlayer->GetPawn()))
    {
        if (DefaultWeaponClasses.Num()>0)
        {
            int32  RandomIndex=FMath::RandRange(0,DefaultWeaponClasses.Num()-1);

            TSubclassOf<AAWeapon> RandomWeaponClass=DefaultWeaponClasses[RandomIndex];

            Character->Server_EquipWeapon(RandomWeaponClass);
        }
    }
}

void AArenaNetDemoGameModeBase::Authority_RegisterKill(AController *KillerController, AActor *VictimActor)
{
    //更新击杀者分数
    if (AADemoPlayerState* KillerPS=KillerController?Cast<AADemoPlayerState>(KillerController->PlayerState):nullptr)
    {
        KillerPS->IncrementKills();

        //胜利条件
        constexpr int32 KillLimit=5;
        if (KillerPS->Kills>=KillLimit)
        {
            HandleMatchEnd(KillerPS);
        }
    }

    //更新被害者死亡次数
    if (APawn *VictimPawn=Cast<APawn>(VictimActor))
    {
        if (AADemoPlayerState*VictimPS=VictimPawn->GetPlayerState()?Cast<AADemoPlayerState>(VictimPawn->GetPlayerState()):nullptr)
        {
            VictimPS->IncrementDeaths();
        }
    }
}

void AArenaNetDemoGameModeBase::HandleMatchEnd(class AADemoPlayerState *Winner)
{
    if (!HasAuthority())
    {
        return;
    }

    if (AADemoGameState*DemoGameState=Cast<AADemoGameState>(GameState))
    {
        if (DemoGameState->CurrentMatchState==EMatchState::Finished)
        {
            return;
        }

        DemoGameState->WinningPlayerState=Winner;
        DemoGameState->CurrentMatchState=EMatchState::Finished;
        
    }

    for (FConstPlayerControllerIterator It=GetWorld()->GetPlayerControllerIterator();It;++It)
    {
        if (APlayerController*PC=It->Get())
        {
            PC->SetCinematicMode(true,true,true,true,true);
        }
    }
}

void AArenaNetDemoGameModeBase::RequestRespawn(ACharacter *DeadCharter)
{
    if (!HasAuthority()||!DeadCharter)
    {
        return;
    }

    AController *Controller =DeadCharter->GetController();
    if (Controller)
    {
        AActor *StartSpot=FindPlayerStart(Controller);

        RestartPlayerAtPlayerStart(Controller,StartSpot);
    }

    DeadCharter->Destroy();
}
