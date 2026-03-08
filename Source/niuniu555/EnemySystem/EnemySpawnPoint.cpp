// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemySpawnPoint.h"
#include "EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"

// 定义日志分类
DEFINE_LOG_CATEGORY(LogEnemySpawn);

UEnemySpawnPoint::UEnemySpawnPoint()
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoActivate = true;

    // 默认设置
    EnemyClass = nullptr;
    SpawnRadius = 0.0f;
    SpawnHeightOffset = 100.0f;
    CooldownTime = 5.0f;
    MaxAliveCount = 1;
    bAutoSpawn = false;
    AutoSpawnInterval = 10.0f;
    
    LastSpawnTime = -999.0f;
    AutoSpawnTimer = 0.0f;
}

void UEnemySpawnPoint::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogEnemySpawn, Log, TEXT("[EnemySpawnPoint] %s 初始化完成"), *GetName());
}

void UEnemySpawnPoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 清理引用
    CleanupDeadEnemies();
    
    Super::EndPlay(EndPlayReason);
}

bool UEnemySpawnPoint::CanSpawn() const
{
    // 检查是否有敌人配置
    if (!EnemyClass)
    {
        UE_LOG(LogEnemySpawn, Warning, TEXT("[EnemySpawnPoint] %s 未配置EnemyClass"), *GetName());
        return false;
    }

    // 检查冷却时间
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastSpawnTime < CooldownTime)
    {
        return false;
    }

    // 检查最大存活数量
    if (MaxAliveCount > 0)
    {
        int32 AliveCount = GetAliveEnemyCount();
        if (AliveCount >= MaxAliveCount)
        {
            return false;
        }
    }

    return true;
}

AActor* UEnemySpawnPoint::SpawnEnemy()
{
    if (!CanSpawn())
    {
        return nullptr;
    }

    // 清理已死亡的敌人
    CleanupDeadEnemies();

    // 获取生成位置和旋转
    FVector SpawnLocation = GetSpawnLocation();
    FRotator SpawnRotation = GetSpawnRotation();

    // 生成参数
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    SpawnParams.Owner = GetOwner();
    SpawnParams.Instigator = nullptr;

    // 生成敌人
    AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(
        EnemyClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (SpawnedEnemy)
    {
        // 添加到列表
        SpawnedEnemies.Add(SpawnedEnemy);
        
        // 更新上次生成时间
        LastSpawnTime = GetWorld()->GetTimeSeconds();
        
        // 播放生成特效
        if (SpawnEffect.IsValid())
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                SpawnEffect.Get(),
                SpawnLocation,
                FRotator::ZeroRotator,
                true
            );
        }
        
        // 广播事件
        OnEnemySpawned.Broadcast(SpawnedEnemy);
        OnSpawnCompleted.Broadcast();
        
        UE_LOG(LogEnemySpawn, Log, TEXT("[EnemySpawnPoint] %s 生成敌人: %s"), 
            *GetName(), *SpawnedEnemy->GetName());
    }
    else
    {
        UE_LOG(LogEnemySpawn, Warning, TEXT("[EnemySpawnPoint] %s 生成敌人失败"), *GetName());
    }

    return SpawnedEnemy;
}

FVector UEnemySpawnPoint::GetSpawnLocation() const
{
    FVector BaseLocation = GetOwner()->GetActorLocation();
    
    // 添加高度偏移
    BaseLocation.Z += SpawnHeightOffset;
    
    // 添加随机水平偏移
    if (SpawnRadius > 0.0f)
    {
        FVector2D RandomOffset = FMath::RandPointInCircle(SpawnRadius);
        BaseLocation.X += RandomOffset.X;
        BaseLocation.Y += RandomOffset.Y;
    }
    
    return BaseLocation;
}

FRotator UEnemySpawnPoint::GetSpawnRotation() const
{
    // 默认使用拥有者的旋转
    return GetOwner()->GetActorRotation();
}

void UEnemySpawnPoint::StartCooldown()
{
    LastSpawnTime = GetWorld()->GetTimeSeconds();
}

void UEnemySpawnPoint::CleanupDeadEnemies()
{
    // 移除无效或死亡的敌人引用
    for (int32 i = SpawnedEnemies.Num() - 1; i >= 0; --i)
    {
        if (!SpawnedEnemies[i].IsValid() || SpawnedEnemies[i]->IsPendingKillPending())
        {
            SpawnedEnemies.RemoveAt(i);
        }
    }
}

int32 UEnemySpawnPoint::GetAliveEnemyCount() const
{
    int32 Count = 0;
    for (const auto& Enemy : SpawnedEnemies)
    {
        if (Enemy.IsValid() && !Enemy->IsPendingKillPending())
        {
            Count++;
        }
    }
    return Count;
}
