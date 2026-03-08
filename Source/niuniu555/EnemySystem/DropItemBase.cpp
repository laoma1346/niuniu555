// Copyright Epic Games, Inc. All Rights Reserved.

#include "DropItemBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

// 定义日志分类
DEFINE_LOG_CATEGORY(LogDropItem);

ADropItemBase::ADropItemBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = false;

    // 创建拾取半径
    PickupRadius = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRadius"));
    RootComponent = PickupRadius;
    PickupRadius->SetSphereRadius(50.0f);
    PickupRadius->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    // 创建网格体
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));

    // 默认配置
    bEnableAutoMagnet = true;
    MagnetRange = 300.0f;
    MagnetSpeed = 800.0f;
    MagnetAcceleration = 2000.0f;
    LifeTime = 30.0f;
    BlinkStartTime = 5.0f;
    
    bIsMagneting = false;
    CurrentMagnetSpeed = 0.0f;
    AliveTime = 0.0f;
    bIsBlinking = false;
}

void ADropItemBase::BeginPlay()
{
    Super::BeginPlay();

    // 绑定碰撞回调
    PickupRadius->OnComponentBeginOverlap.AddDynamic(this, &ADropItemBase::OnPickupOverlap);

    // 设置生命周期定时器
    if (LifeTime > 0.0f)
    {
        // 设置闪烁定时器
        if (BlinkStartTime > 0.0f && BlinkStartTime < LifeTime)
        {
            FTimerHandle BlinkTimer;
            GetWorldTimerManager().SetTimer(BlinkTimer, this, &ADropItemBase::StartBlinking, LifeTime - BlinkStartTime, false);
        }

        // 设置销毁定时器
        FTimerHandle DestroyTimer;
        GetWorldTimerManager().SetTimer(DestroyTimer, this, &ADropItemBase::ExpireAndDestroy, LifeTime, false);
    }

    UE_LOG(LogDropItem, Verbose, TEXT("[DropItemBase] %s 已生成"), *GetName());
}

void ADropItemBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AliveTime += DeltaTime;

    // 检测玩家是否在吸附范围内
    if (bEnableAutoMagnet && !bIsMagneting)
    {
        CheckPlayerInRange();
    }

    // 更新吸附移动
    if (bIsMagneting)
    {
        UpdateMagnetMovement(DeltaTime);
    }

    // 更新闪烁效果
    if (bIsBlinking)
    {
        UpdateBlink(DeltaTime);
    }
}

void ADropItemBase::InitializeDrop(const FVector& SpawnLocation, const FVector& SpawnImpulse)
{
    // 设置位置
    SetActorLocation(SpawnLocation);

    // 确保MeshComponent可见（修复模型不显示问题）
    if (MeshComponent)
    {
        // 强制开启可视性
        MeshComponent->SetVisibility(true);
        MeshComponent->SetHiddenInGame(false);
        
        // 确保缩放不为0（防止某些蓝图配置错误）
        FVector CurrentScale = MeshComponent->GetRelativeScale3D();
        if (CurrentScale.IsNearlyZero())
        {
            MeshComponent->SetRelativeScale3D(FVector(1.0f));
            UE_LOG(LogDropItem, Warning, TEXT("[DropItemBase] %s Mesh缩放为0，已重置为1"), *GetName());
        }
        
        // 如果有物理，添加初始冲量
        if (MeshComponent->IsSimulatingPhysics())
        {
            MeshComponent->AddImpulse(SpawnImpulse, NAME_None, true);
        }
    }
    else
    {
        UE_LOG(LogDropItem, Error, TEXT("[DropItemBase] %s MeshComponent为空！模型将无法显示"), *GetName());
    }

    UE_LOG(LogDropItem, Log, TEXT("[DropItemBase] %s 初始化在位置 %s, Mesh=%s, 冷却=%.1f秒"), 
        *GetName(), *SpawnLocation.ToString(), 
        MeshComponent ? *MeshComponent->GetName() : TEXT("NULL"),
        SpawnCooldown);
}

void ADropItemBase::SetTargetPlayer(APawn* Player)
{
    TargetPlayer = Player;
    
    if (TargetPlayer.IsValid())
    {
        bIsMagneting = true;
        CurrentMagnetSpeed = 0.0f;
        
        // 【修复】开始吸附时立即禁用碰撞，防止重复触发
        if (PickupRadius)
        {
            PickupRadius->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            PickupRadius->SetGenerateOverlapEvents(false);
        }
        
        UE_LOG(LogDropItem, Log, TEXT("[DropItemBase] %s 开始吸附到玩家 %s，碰撞已禁用"), *GetName(), *TargetPlayer->GetName());
    }
}

void ADropItemBase::CheckPlayerInRange()
{
    // 生成冷却期内不检测吸附
    if (AliveTime < SpawnCooldown)
    {
        return;
    }

    // 获取玩家角色
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    // 检查距离
    float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Distance <= MagnetRange)
    {
        SetTargetPlayer(PlayerPawn);
    }
}

void ADropItemBase::UpdateMagnetMovement(float DeltaTime)
{
    if (!TargetPlayer.IsValid())
    {
        bIsMagneting = false;
        return;
    }

    // 加速
    CurrentMagnetSpeed += MagnetAcceleration * DeltaTime;
    CurrentMagnetSpeed = FMath::Min(CurrentMagnetSpeed, MagnetSpeed);

    // 计算方向
    FVector Direction = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();

    // 移动
    FVector NewLocation = GetActorLocation() + Direction * CurrentMagnetSpeed * DeltaTime;
    SetActorLocation(NewLocation);

    // 检查是否到达玩家位置（添加防重复拾取保护）
    float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
    if (Distance < 50.0f && !bAlreadyPicked) // 拾取距离且未被拾取过
    {
        // 【修复】立即禁用碰撞防止重复触发
        if (PickupRadius)
        {
            PickupRadius->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            PickupRadius->SetGenerateOverlapEvents(false);
        }
        
        bAlreadyPicked = true;
        OnPickup(TargetPlayer.Get());
    }
}

void ADropItemBase::OnPickup_Implementation(APawn* Player)
{
    // 防重复拾取保护
    if (bAlreadyPicked)
    {
        return;
    }
    bAlreadyPicked = true;

    UE_LOG(LogDropItem, Log, TEXT("[DropItemBase] %s 被玩家 %s 拾取"), *GetName(), *Player->GetName());

    // 广播事件
    OnDropItemPicked.Broadcast(this, Player);

    // 销毁自身
    Destroy();
}

void ADropItemBase::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 防重复拾取保护
    if (bAlreadyPicked)
    {
        return;
    }

    // 检查是否是玩家
    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    if (!PlayerPawn || !PlayerPawn->ActorHasTag(FName(TEXT("Player"))))
    {
        return;
    }

    // 【修复】立即禁用碰撞防止重复触发
    if (PickupRadius)
    {
        PickupRadius->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PickupRadius->SetGenerateOverlapEvents(false);
    }
    
    // 执行拾取
    bAlreadyPicked = true;
    OnPickup(PlayerPawn);
}

void ADropItemBase::StartBlinking()
{
    bIsBlinking = true;
    
    UE_LOG(LogDropItem, Verbose, TEXT("[DropItemBase] %s 开始闪烁"), *GetName());
}

void ADropItemBase::UpdateBlink(float DeltaTime)
{
    // 简单的透明度闪烁效果
    if (MeshComponent)
    {
        float Alpha = FMath::Abs(FMath::Sin(AliveTime * 10.0f));
        FLinearColor Color = FLinearColor(1.0f, 1.0f, 1.0f, Alpha);
        // 注意：这里只是示例，实际应该通过材质参数或动态材质实例来实现
    }
}

void ADropItemBase::ExpireAndDestroy()
{
    UE_LOG(LogDropItem, Verbose, TEXT("[DropItemBase] %s 生命周期结束，销毁"), *GetName());
    
    Destroy();
}
