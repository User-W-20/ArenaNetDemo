// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "ADemoPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ARENANETDEMO_API AADemoPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AADemoPlayerState();

    UPROPERTY(ReplicatedUsing = Onrep_kills)
    int32 Kills;

    UPROPERTY(ReplicatedUsing = OnRep_Deaths)
    int32 Deaths;

    UFUNCTION()
    void OnRep_kills();

    UFUNCTION()
    void OnRep_Deaths();

    UFUNCTION(BlueprintCallable)
    void IncrementKills();

    UFUNCTION(BlueprintCallable)
    void IncrementDeaths();

protected:
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> &OutLifetimeProps) const override;
};