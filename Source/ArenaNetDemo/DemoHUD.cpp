// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoHUD.h"

#include "ADemoCharacter.h"
#include "AWeapon.h"
#include "Engine/Canvas.h"
#include "UObject/ConstructorHelpers.h"


ADemoHUD::ADemoHUD()
{
    static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(
        TEXT("Texture2D'/Game/FirstPerson/Textures/FirstPersonCrosshair.FirstPersonCrosshair'"));

    if (CrosshairTexObj.Succeeded())
    {
        CrosshairTexture = CrosshairTexObj.Object;
    }
}

void ADemoHUD::DrawHUD()
{
    Super::DrawHUD();

    if (CrosshairTexture)
    {
        FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
        FVector2D CrosshairDrawPos(Center.X - (CrosshairTexture->GetSurfaceWidth() * 0.5f),
                                   Center.Y - (CrosshairTexture->GetSurfaceHeight() * 0.5f));

        FCanvasTileItem TileItem(CrosshairDrawPos, CrosshairTexture->GetResource(), FLinearColor::White);
        TileItem.BlendMode = SE_BLEND_Translucent;
        Canvas->DrawItem(TileItem);
    }

    APlayerController *PC = GetOwningPlayerController();
    if (!PC || !PC->IsLocalController())
        return;

    APawn *MyPawn = PC->GetPawn();
    if (!MyPawn)
        return;

    AADemoCharacter *MyCharacter = Cast<AADemoCharacter>(MyPawn);
    if (!MyCharacter)
        return;

    AAWeapon *Weapon = MyCharacter->CurrentWeapon;
    if (!Weapon)
        return;

    const int32 CurrentAmmo = Weapon->CurrentAmmo;
    const int32 MaxAmmo     = Weapon->GetMaxAmmo();

    FString AmmoText = FString::Printf(TEXT("Ammo: %d / %d"), CurrentAmmo, MaxAmmo);

    FVector2D    TextPos(50.0f, Canvas->ClipY - 100.0f);
    FLinearColor TextColor = FLinearColor::White;
    float        FontScale = 1.3f;

    DrawText(AmmoText, TextColor, TextPos.X, TextPos.Y, nullptr, FontScale, nullptr);
}