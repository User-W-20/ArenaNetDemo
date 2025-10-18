// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "AWeapon.generated.h"

UCLASS()
class ARENANETDEMO_API AAWeapon : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AAWeapon();

    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> &OutLifetimeProps) const override;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    class USkeletalMeshComponent *WeaponMesh;

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_Fire(const FVector &TraceStart, const FVector_NetQuantizeNormal &ShootDir);
    bool Server_Fire_Validate(const FVector &TraceStart, const FVector_NetQuantizeNormal &ShootDir);
    void Server_Fire_Implementation(const FVector &TraceStart, const FVector_NetQuantizeNormal &ShootDir);


    UPROPERTY(EditDefaultsOnly, Category="Weapon")
    float BaseDamage;

    UPROPERTY(EditDefaultsOnly, Category= "Weapon")
    FName MuzzleSocketName = TEXT("Muzzle");

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayFireEffects(const FVector_NetQuantize &MuzzleLocation, const FVector_NetQuantize &TraceEnd);

    //弹孔
    UPROPERTY(EditDefaultsOnly, Category="FX")
    class UMaterialInterface *ImpactDecal;

    UPROPERTY(EditDefaultsOnly, Category="FX")
    FVector ImpactDecalSize = FVector(20.0f, 20.0f, 20.0f);

    //枪口火焰特效
    UPROPERTY(EditDefaultsOnly, Category="FX")
    class UParticleSystem *MuzzleFlashFX;

    //开火音效
    UPROPERTY(EditDefaultsOnly, Category="FX")
    class USoundBase *FireSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
    int32 MaxAmmo = 30;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    void Fire();

    UFUNCTION(BlueprintCallable, Category="Weapon")
    void Reload();

    UPROPERTY(ReplicatedUsing=OnRep_CurrentAmmo, BlueprintReadOnly, Category="Weapon")
    int32 CurrentAmmo;

    UFUNCTION()
    void OnRep_CurrentAmmo();

    UFUNCTION(Server, Reliable)
    void Server_Reload();

    UFUNCTION(BlueprintCallable, Category="Weapon")
    int32 GetCurrentAmmo() const;

    UFUNCTION(BlueprintCallable, Category="Weapon")
    int32 GetMaxAmmo() const;

    FORCEINLINE USkeletalMeshComponent *GetWeaponMesh() const
    {
        return WeaponMesh;
    }
};