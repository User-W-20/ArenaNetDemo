// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "ADemoCharacter.generated.h"

UCLASS()
class ARENANETDEMO_API AADemoCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AADemoCharacter();

    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> &OutLifetimeProps) const override;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    //视角
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    UCameraComponent *FirstPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Mesh")
    USkeletalMeshComponent *FirstPersonMesh;

    void UpdateViewMeshesVisibility();
    
    //移动
    void MoveForward(float Value);
    void MoveRight(float Value);

    virtual void PossessedBy(AController *NewController) override;

    UFUNCTION(Client, Reliable)
    void Client_SetInputMode();

    void FireWeapon();

    UFUNCTION()
    void ReloadWeapon();
    
    //武器
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Weapon")
    TSubclassOf<class AAWeapon>DefaultWeaponClass;
    
    

    UFUNCTION()
    void OnRep_Current_Weapon();

    //生命值
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Health")
    float MaxHealth = 100.0f;

    UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, Category= "Health")
    float Health;

    UFUNCTION()
    void OnRep_Health();

    virtual float TakeDamage(float              DamageAmount, struct FDamageEvent const &DamageEvent,
                             class AController *EventInstigator, AActor *                DamageCauser) override;

    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Mesh")
    USkeletalMeshComponent* Mesh1P;
public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

    UFUNCTION(Server, Reliable)
    void Server_EquipWeapon(TSubclassOf<AAWeapon> WeaponClass);


    UPROPERTY(ReplicatedUsing=OnRep_Current_Weapon, BlueprintReadOnly, Category= "Weapon")
    class AAWeapon *CurrentWeapon;

    
    void OnResapawn();

};