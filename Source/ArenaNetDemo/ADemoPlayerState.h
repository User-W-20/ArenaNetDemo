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

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int32 Kills;

	UPROPERTY(Replicated)
	int32 Deaths;

	void IncrementKills(int32 Amount=1);

	void IncrementDeaths(int Amount=1);
};
