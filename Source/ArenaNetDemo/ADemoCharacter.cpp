// Fill out your copyright notice in the Description page of Project Settings.


#include "ADemoCharacter.h"

#include "AWeapon.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ArenaNetDemoGameModeBase.h"

// Sets default values
AADemoCharacter::AADemoCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    bReplicates                   = true;

    //SetReplicates(true);

    //第一人称
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));

    //摄像机附加到胶囊体
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(-10.0f, 0.0f, BaseEyeHeight));
    FirstPersonCamera->bUsePawnControlRotation = true;

    //第一人称手臂
    FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
    FirstPersonMesh->SetupAttachment(FirstPersonCamera);
    FirstPersonMesh->SetOnlyOwnerSee(true);
    FirstPersonMesh->bCastDynamicShadow = false;
    FirstPersonMesh->CastShadow         = false;
    FirstPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    //第三人称
    GetMesh()->SetupAttachment(GetCapsuleComponent());
    GetMesh()->SetOwnerNoSee(true);
    GetMesh()->bCastDynamicShadow = true;
    GetMesh()->CastShadow         = true;

    //绕Z轴旋转
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = true;
    bUseControllerRotationRoll  = false;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxAcceleration           = 2048.0f;
    GetCharacterMovement()->SetIsReplicated(true);

    Health = MaxHealth;

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
        TEXT("SkeletalMesh'/Game/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin'"));
    if (MeshAsset.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(MeshAsset.Object);
        FirstPersonMesh->SetSkeletalMesh(MeshAsset.Object);
    }
}

// Called when the game starts or when spawned
void AADemoCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (FirstPersonCamera && GetCapsuleComponent())
    {
        FirstPersonCamera->AttachToComponent(
            GetCapsuleComponent(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale);

        FirstPersonCamera->SetRelativeLocation(FVector(-10.0f, 0.0f, BaseEyeHeight));
        FirstPersonCamera->bUsePawnControlRotation = true;
    }

    UpdateViewMeshesVisibility();

    if (HasAuthority() && DefaultWeaponClass)
    {
        Server_EquipWeapon(DefaultWeaponClass);
    }

    UE_LOG(LogTemp, Warning, TEXT("[%s] BeginPlay 完成，摄像机已重设"), *GetName());
}

// Called every frame
void AADemoCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AADemoCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AADemoCharacter, CurrentWeapon);
}


// Called to bind functionality to input
void AADemoCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    //视角
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

    //移动
    PlayerInputComponent->BindAxis("MoveForward", this, &AADemoCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AADemoCharacter::MoveRight);

    //跳跃
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AADemoCharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &AADemoCharacter::StopJumping);

    //射击
    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AADemoCharacter::FireWeapon);

    //换弹
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AADemoCharacter::ReloadWeapon);
}

void AADemoCharacter::MoveForward(float Value)
{
    if (Value != 0.0f)
    {
        const FRotator Rotation = GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector  Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        AddMovementInput(Direction, Value);
    }
}

void AADemoCharacter::MoveRight(float Value)
{
    if (Value != 0.0f)
    {
        const FRotator Rotation = GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector  Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(Direction, Value);
    }
}


void AADemoCharacter::PossessedBy(AController *NewController)
{
    Super::PossessedBy(NewController);

    if (HasAuthority())
    {
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);

            GetCharacterMovement()->MaxAcceleration = 2048.0f;

            GetCharacterMovement()->SetIsReplicated(true);
        }
    }

    Client_SetInputMode();
}

void AADemoCharacter::Client_SetInputMode_Implementation()
{
    if (APlayerController *PC = Cast<APlayerController>(GetController()))
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}

void AADemoCharacter::FireWeapon()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->Fire();
    }
}

void AADemoCharacter::OnRep_Current_Weapon()
{
    if (!CurrentWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] OnRep_Current_Weapon: CurrentWeapon == nullptr"), *GetName());
        return;
    }

    USkeletalMeshComponent *AttachMesh     = nullptr;
    const bool              bIsLocalPlayer = IsLocallyControlled();

    AttachMesh = bIsLocalPlayer ? (FirstPersonMesh ? FirstPersonMesh : Mesh1P) : GetMesh();
    if (!AttachMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] OnRep_Current_Weapon: AttachMesh is null!"), *GetName());
        return;
    }

    if (!AttachMesh->DoesSocketExist(TEXT("GripPoint")))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Attach mesh missing GripPoint socket! Attaching to actor fallback."),
               *GetName());
        CurrentWeapon->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
    }
    else
    {
        CurrentWeapon->AttachToComponent(AttachMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                                         TEXT("GripPoint"));
    }

    if (USkeletalMeshComponent *WMesh = CurrentWeapon->GetWeaponMesh())
    {
        WMesh->SetOnlyOwnerSee(bIsLocalPlayer);
        WMesh->SetOwnerNoSee(!bIsLocalPlayer);

        WMesh->SetCastShadow(!bIsLocalPlayer);
    }

    if (bIsLocalPlayer)
    {
        if (FirstPersonMesh)
            FirstPersonMesh->SetOwnerNoSee(false);
        if (GetMesh())
            GetMesh()->SetOwnerNoSee(true);
    }

    else
    {
        if (FirstPersonMesh)
            FirstPersonMesh->SetOwnerNoSee(true);
        if (GetMesh())
            GetMesh()->SetOwnerNoSee(false);
    }

    UE_LOG(LogTemp, Warning, TEXT("[OnRep] %s attached weapon %s to %s"),
           *GetName(),
           *CurrentWeapon->GetName(),
           bIsLocalPlayer ? TEXT("FirstPersonMesh") : TEXT("Mesh3P"));

    if (USkeletalMeshComponent *Mesh3P = GetMesh())
    {
        if (UAnimInstance *AnimInstance = Mesh3P->GetAnimInstance())
        {
            bool bWeaponEquipped = (CurrentWeapon != nullptr);

            UClass *AnimClass = AnimInstance->GetClass();

            if (FBoolProperty *BoolProp = FindFProperty<FBoolProperty>(AnimClass,TEXT("bIsRifleEquipped")))
            {
                BoolProp->SetPropertyValue_InContainer(AnimInstance, bWeaponEquipped);

                UE_LOG(LogTemp, Warning, TEXT("AnimBP updated: bIsRifleEquipped set to %s"),
                       bWeaponEquipped ? TEXT("True") : TEXT("False"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("AnimBP variable 'bIsRifleEquipped' not found or not a boolean."));
            }

        }
    }
}


void AADemoCharacter::Server_EquipWeapon_Implementation(TSubclassOf<AAWeapon> WeaponClass)
{
    if (!HasAuthority())
        return;

    if (CurrentWeapon)
    {
        CurrentWeapon->Destroy();
        CurrentWeapon = nullptr;
    }

    if (!WeaponClass)
    {
        CurrentWeapon = nullptr;
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner                          = this;
    SpawnParams.Instigator                     = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AAWeapon *NewWeapon = GetWorld()->SpawnActor<AAWeapon>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator,
                                                           SpawnParams);

    if (!NewWeapon)
    {
        UE_LOG(LogTemp, Error, TEXT("[Server] Failed to spawn weapon"));
        return;
    }

    NewWeapon->SetReplicates(true);
    NewWeapon->SetReplicateMovement(true);
    if (NewWeapon->GetWeaponMesh())
    {
        NewWeapon->GetWeaponMesh()->SetIsReplicated(true);
    }

    CurrentWeapon = NewWeapon;
    OnRep_Current_Weapon();
}


void AADemoCharacter::OnRep_Health()
{
    UE_LOG(LogTemp, Warning, TEXT("Client: Health updated to %f"), Health);

    if (Health <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Client: %s has been killed."), *GetName());
    }
}

float AADemoCharacter::TakeDamage(float              DamageAmount, struct FDamageEvent const &DamageEvent,
                                  class AController *EventInstigator, AActor *                DamageCauser)
{
    if (!HasAuthority())
    {
        return 0.0f;
    }

    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (ActualDamage <= 0)
    {
        return 0.0f;
    }

    Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);

    if (Health <= 0)
    {
        if (AArenaNetDemoGameModeBase *GM = Cast<AArenaNetDemoGameModeBase>(GetWorld()->GetAuthGameMode()))
        {
            GM->Authority_RegisterKill(EventInstigator, this);

            GM->RequestRespawn(this);
        }

        DisableInput(nullptr);
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    return ActualDamage;
}

void AADemoCharacter::UpdateViewMeshesVisibility()
{
    if (IsLocallyControlled())
    {
        FirstPersonMesh->SetOwnerNoSee(false);
        GetMesh()->SetOwnerNoSee(true);
    }
    else
    {
        FirstPersonMesh->SetOwnerNoSee(true);
        GetMesh()->SetOwnerNoSee(false);
    }
}

void AADemoCharacter::OnResapawn()
{

    UpdateViewMeshesVisibility();

    Health = MaxHealth;

    if (FirstPersonCamera && GetCapsuleComponent())
    {
        FirstPersonCamera->AttachToComponent(
            GetCapsuleComponent(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        FirstPersonCamera->SetRelativeLocation(FVector(-10.0f, 0.0f, BaseEyeHeight));
        FirstPersonCamera->bUsePawnControlRotation = true;
    }

    if (HasAuthority() && DefaultWeaponClass)
    {
        Server_EquipWeapon(DefaultWeaponClass);
    }

    UE_LOG(LogTemp, Warning, TEXT("[%s] OnRespawn 执行完毕，摄像机与可见性重置完成"), *GetName());
}

void AADemoCharacter::ReloadWeapon()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->Reload();
    }
}