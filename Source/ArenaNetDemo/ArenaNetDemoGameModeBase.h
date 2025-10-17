// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ArenaNetDemoGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class ARENANETDEMO_API AArenaNetDemoGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

protected:
    //武器列表
    UPROPERTY(EditDefaultsOnly, Category= "Gameplay")
    TArray<TSubclassOf<class AAWeapon>> DefaultWeaponClasses;

public:
    virtual void PostLogin(APlayerController *NewPlayer) override;

    //击杀注册函数
    void Authority_RegisterKill(AController *KillerController, AActor *VictimActor);

    //游戏结束函数
    void HandleMatchEnd(class AADemoPlayerState *Winner);

    AArenaNetDemoGameModeBase();

    //重生请求
    void RequestRespawn(ACharacter*DeadCharter);
};