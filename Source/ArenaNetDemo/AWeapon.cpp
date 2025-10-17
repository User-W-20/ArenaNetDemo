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

    WeaponMesh=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent=WeaponMesh;
    
	bReplicates=true;
    SetReplicateMovement(true);
    WeaponMesh->SetIsReplicated(true);
    
	BaseDamage=20.0f;
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

void AAWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


void AAWeapon::Fire()
{
    APawn* OwnerPawn=Cast<APawn>(GetOwner());
    if (!OwnerPawn)return;

    FVector ViewLoc;
    FRotator ViewRot;
    if (AController*PC=OwnerPawn->GetController())
    {
        PC->GetPlayerViewPoint(ViewLoc,ViewRot);
    }
    else
    {
        ViewLoc=WeaponMesh?WeaponMesh->GetSocketLocation(TEXT("Muzzle")):OwnerPawn->GetActorLocation();
        ViewRot=OwnerPawn->GetControlRotation();
    }

    FVector ShootDir=ViewRot.Vector();
    FVector TraceEnd=ViewLoc+ShootDir*50000.0f;
    FVector MuzzleLoc=WeaponMesh?WeaponMesh->GetSocketLocation(TEXT("Muzzle")):ViewLoc;

    if (OwnerPawn->IsLocallyControlled())
    {

        if (!OwnerPawn->HasAuthority())
        {
            Server_Fire(ViewLoc,FVector_NetQuantizeNormal(ShootDir));

            Multicast_PlayFireEffects_Implementation(MuzzleLoc,TraceEnd);
            return;
        }
    }

    if (HasAuthority())
    {
        Server_Fire_Implementation(ViewLoc,FVector_NetQuantize(ShootDir));
    }
}


void AAWeapon::Server_Fire_Implementation(const FVector &TraceStart, const FVector_NetQuantizeNormal &ShootDir)
{
    if (!HasAuthority())return;

    FVector TraceEnd=TraceStart+ShootDir*50000.0f;

    FHitResult Hit;
    FCollisionQueryParams QueryParams;
    APawn *OwnerPawn=Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        QueryParams.AddIgnoredActor(OwnerPawn);
    }
    QueryParams.AddIgnoredActor(this);
    QueryParams.bTraceComplex=true;

    bool bHit=GetWorld()->LineTraceSingleByChannel(Hit,TraceStart,TraceEnd,ECC_Visibility,QueryParams);

    if (bHit)
    {
        AActor*HitActor=Hit.GetActor();
        if (HitActor)
        {
            UGameplayStatics::ApplyPointDamage(HitActor, BaseDamage, ShootDir, Hit, OwnerPawn ? OwnerPawn->GetController() : nullptr, this, nullptr);
        }
        Multicast_PlayFireEffects(WeaponMesh ? WeaponMesh->GetSocketLocation(MuzzleSocketName) : TraceStart, Hit.ImpactPoint);
    }else
    {
        Multicast_PlayFireEffects(WeaponMesh ? WeaponMesh->GetSocketLocation(MuzzleSocketName) : TraceStart, TraceEnd);
    }
    DrawDebugLine(GetWorld(), TraceStart, bHit ? Hit.ImpactPoint : TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
}


void AAWeapon::Multicast_PlayFireEffects_Implementation(const FVector_NetQuantize &MuzzleLocation, const FVector_NetQuantize &TraceEnd)
{
    UE_LOG(LogTemp, Warning, TEXT("MULTICAST TRIGGERED! TraceEnd: %s"), *TraceEnd.ToString());

    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this,FireSound,MuzzleLocation);
    }

    if (MuzzleFlashFX)
    {
        FRotator MuzzleRotation=WeaponMesh->GetSocketRotation(TEXT("Muzzle"));
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),MuzzleFlashFX,MuzzleLocation,MuzzleRotation);
    }

    APawn* OwnerPawn=Cast<APawn>(GetOwner());
    if (OwnerPawn&& OwnerPawn->IsLocallyControlled())
    {
        DrawDebugLine(GetWorld(),MuzzleLocation,TraceEnd,FColor::Yellow,true,5.0f,0,1.0f);
    }

    FHitResult Hit;
    FCollisionQueryParams QueryParams;

    if (AActor*OwnerActor=GetOwner())
    {
        QueryParams.AddIgnoredActor(OwnerActor);
    }
    QueryParams.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(Hit,MuzzleLocation,TraceEnd,ECC_Visibility,QueryParams))
    {
        if (ImpactDecal)
        {
            FRotator DecalRotation=Hit.ImpactNormal.Rotation();

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
