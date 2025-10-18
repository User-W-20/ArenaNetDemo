// Fill out your copyright notice in the Description page of Project Settings.


#include "AWeapon.h"

#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
// Sets default values
AAWeapon::AAWeapon()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    WeaponMesh    = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;

    bReplicates = true;
    SetReplicateMovement(true);
    WeaponMesh->SetIsReplicated(true);

    BaseDamage = 20.0f;

    CurrentAmmo = MaxAmmo;
}

// Called when the game starts or when spawned
void AAWeapon::BeginPlay()
{
    Super::BeginPlay();

}

// Called every frame
void AAWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AAWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AAWeapon, CurrentAmmo);
}

void AAWeapon::Fire()
{
    APawn *OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
        return;

    FVector  ViewLoc;
    FRotator ViewRot;
    if (AController *PC = OwnerPawn->GetController())
    {
        PC->GetPlayerViewPoint(ViewLoc, ViewRot);
    }
    else
    {
        ViewLoc = WeaponMesh ? WeaponMesh->GetSocketLocation(MuzzleSocketName) : OwnerPawn->GetActorLocation();
        ViewRot = OwnerPawn->GetControlRotation();
    }

    const FVector ShootDir   = ViewRot.Vector();
    const FVector TraceStart = ViewLoc;

    if (!HasAuthority())
    {
        Server_Fire(TraceStart, FVector_NetQuantizeNormal(ShootDir));
        if (WeaponMesh && MuzzleFlashFX && OwnerPawn->IsLocallyControlled())
        {
            UGameplayStatics::SpawnEmitterAttached(MuzzleFlashFX, WeaponMesh, MuzzleSocketName);
            if (FireSound)
                UGameplayStatics::PlaySoundAtLocation(this, FireSound, WeaponMesh->GetSocketLocation(MuzzleSocketName));
        }
        return;
    }
    Server_Fire_Implementation(TraceStart, FVector_NetQuantizeNormal(ShootDir));
}


void AAWeapon::Server_Fire_Implementation(const FVector &TraceStart, const FVector_NetQuantizeNormal &ShootDir)
{
    if (!HasAuthority())
        return;

    if (CurrentAmmo <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Server] 无子弹，拒绝开火"));
        return;
    }
    --CurrentAmmo;
    UE_LOG(LogTemp, Warning, TEXT("[Server] Fire executed. Remaining ammo: %d"), CurrentAmmo);

    const float TraceDistance = 50000.0f;
    FVector     TraceEnd      = TraceStart + ShootDir * TraceDistance;

    FHitResult            Hit;
    FCollisionQueryParams QueryParams;
    APawn *               OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
        QueryParams.AddIgnoredActor(OwnerPawn);
    QueryParams.AddIgnoredActor(this);
    QueryParams.bTraceComplex = true;

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, QueryParams);

    if (bHit)
    {
        if (AActor *HitActor = Hit.GetActor())
        {
            UGameplayStatics::ApplyPointDamage(HitActor, BaseDamage, ShootDir, Hit,
                                               OwnerPawn ? OwnerPawn->GetController() : nullptr, this, nullptr);
        }
    }

    const FVector MuzzleLoc = WeaponMesh ? WeaponMesh->GetSocketLocation(MuzzleSocketName) : TraceStart;
    Multicast_PlayFireEffects(MuzzleLoc, bHit ? Hit.ImpactPoint : TraceEnd);
}


void AAWeapon::Multicast_PlayFireEffects_Implementation(const FVector_NetQuantize &MuzzleLocation,
                                                        const FVector_NetQuantize &TraceEnd)
{
    UE_LOG(LogTemp, Warning, TEXT("MULTICAST TRIGGERED! TraceEnd: %s"), *TraceEnd.ToString());

    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleLocation);
    }

    if (MuzzleFlashFX)
    {
        FRotator MuzzleRotation = WeaponMesh->GetSocketRotation(TEXT("Muzzle"));
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlashFX, MuzzleLocation, MuzzleRotation);
    }

    APawn *OwnerPawn = Cast<APawn>(GetOwner());

    FHitResult            Hit;
    FCollisionQueryParams QueryParams;

    if (AActor *OwnerActor = GetOwner())
    {
        QueryParams.AddIgnoredActor(OwnerActor);
    }
    QueryParams.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(Hit, MuzzleLocation, TraceEnd, ECC_Visibility, QueryParams))
    {
        if (ImpactDecal)
        {
            FRotator DecalRotation = Hit.ImpactNormal.Rotation();

            UGameplayStatics::SpawnDecalAtLocation(
                this,
                ImpactDecal,
                ImpactDecalSize,
                Hit.ImpactPoint,
                DecalRotation,
                10.0f);
        }
    }
}

bool AAWeapon::Server_Fire_Validate(const FVector &TraceStart, const FVector_NetQuantizeNormal &ShootDir)
{
    return true;
}


void AAWeapon::Reload()
{
    if (!HasAuthority())
    {
        Server_Reload();
        return;
    }

    CurrentAmmo = MaxAmmo;
    UE_LOG(LogTemp, Warning, TEXT("[Server] Reload complete, ammo=%d"), CurrentAmmo);
}


void AAWeapon::OnRep_CurrentAmmo()
{
    UE_LOG(LogTemp, Warning, TEXT("客户端收到弹药更新：%d"), CurrentAmmo);
}

void AAWeapon::Server_Reload_Implementation()
{
    Reload();
}

int32 AAWeapon::GetCurrentAmmo() const
{
    return CurrentAmmo;
}

int32 AAWeapon::GetMaxAmmo() const
{
    return MaxAmmo;
}