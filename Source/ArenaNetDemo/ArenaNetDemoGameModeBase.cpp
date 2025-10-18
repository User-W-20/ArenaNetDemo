// Copyright Epic Games, Inc. All Rights Reserved.


#include "ArenaNetDemoGameModeBase.h"
#include "ADemoCharacter.h"
#include "ADemoGameState.h"
#include "AWeapon.h"
#include "ADemoPlayerState.h"
#include "DemoHUD.h"

AArenaNetDemoGameModeBase::AArenaNetDemoGameModeBase()
{
    DefaultPawnClass=AADemoCharacter::StaticClass();

    PlayerStateClass=AADemoPlayerState::StaticClass();

    GameStateClass=AADemoGameState::StaticClass();

    HUDClass=ADemoHUD::StaticClass();
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
    if (!HasAuthority())return;
    
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

    UE_LOG(LogTemp, Warning, TEXT("比赛结束！赢家：%s"), Winner ? *Winner->GetPlayerName() : TEXT("未知"));
    
    if (AADemoGameState*DemoGameState=Cast<AADemoGameState>(GameState))
    {
        DemoGameState->WinningPlayerState=Winner;
        DemoGameState->CurrentMatchState=EMatchState::Finished;
    }

  
    Multicast_OnMatchEnd(Winner);
        
    for (FConstPlayerControllerIterator It=GetWorld()->GetPlayerControllerIterator();It;++It)
    {
        if (APlayerController*PC=It->Get())
        {
            PC->DisableInput(PC);
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
        Controller->UnPossess();

        DeadCharter->Destroy();

        AActor *StartSpot=FindPlayerStart(Controller);
        
        RestartPlayerAtPlayerStart(Controller,StartSpot);

       APlayerController*PC=Cast<APlayerController>(Controller);
        if (PC)
        {
            APawn* NewPawn=PC->GetPawn();

            if (NewPawn)
            {
                PC->SetViewTarget(NewPawn);

                AADemoCharacter*DemoChar=Cast<AADemoCharacter>(NewPawn);
                if (DemoChar)
                {
                    DemoChar->OnResapawn();
                }
            }
        }
    }
}

void AArenaNetDemoGameModeBase::Multicast_OnMatchEnd_Implementation(AADemoPlayerState *WinnerPS)
{
    FString WinnerName=WinnerPS?WinnerPS->GetPlayerName(): TEXT("未知");
    FString EndMsg=FString::Printf(TEXT("🏆 比赛结束！赢家：%s"), *WinnerName);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Yellow, EndMsg);
    }

    UE_LOG(LogTemp, Warning, TEXT("[Multicast] 比赛结束：赢家为 %s"), *WinnerName);
}
