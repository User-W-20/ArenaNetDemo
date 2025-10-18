// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DemoHUD.generated.h"

/**
 * 
 */
UCLASS()
class ARENANETDEMO_API ADemoHUD : public AHUD
{
    GENERATED_BODY()

protected:
    ADemoHUD();
    virtual void DrawHUD() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crosshair")
    UTexture2D *CrosshairTexture;
};