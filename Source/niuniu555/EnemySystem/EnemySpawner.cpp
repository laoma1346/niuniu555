// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemySpawner.h"
#include "EnemySpawnPoint.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// 定义日志分类
DEFINE_LOG_CATEGORY(LogEnemySpawner);

AEnemySpawner::AEnemySpawner(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = false;

    // 创建触发区域
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
    TriggerVolume->SetBoxExtent(FVector(500.0f, 500.0f, 200.0f));

    // 默认配置
    CurrentWaveIndex = 0;
    CurrentWaveSpawnedCount = 0;
    TotalSpawnedCount = 0;
    bIsSpawning = false;
    SpawnTimer = 0.0f;
    WaveDelayTimer = 0.0f;
    MaxAliveEnemies = 10;
    bUseTriggerVolume = true;
    bAutoStart = false;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    // 绑定触发回调
    if (bUseTriggerVolume)
    {
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AEnemySpawner::OnTriggerOverlap);
    }

    // 收集生成点
    CollectSpawnPoints();

    // 如果不使用触发区域且自动开始，则立即开始
    if (!bUseTriggerVolume && bAutoStart)
    {
        StartSpawning();
    }

    UE_LOG(LogEnemySpawner, Log, TEXT("[EnemySpawner] %s 初始化完成，收集到 %d 个生成点"),
        *GetName(), SpawnPoints.Num());
}

void AEnemySpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsSpawning)
    {
        return;
    }

    // 清理死亡的敌人
    CleanupDeadEnemies();

    // 检查当前波次是否完成
    if (IsCurrentWaveComplete())
    {
        // 广播波次完成事件
        OnWaveCompleted.Broadcast(CurrentWaveIndex);
        
        UE_LOG(LogEnemySpawner, Log, TEXT("[EnemySpawner] 波次 %d 完成"), CurrentWaveIndex);

        // 检查是否所有波次完成
        if (IsAllWavesComplete())
        {
            OnAllWavesCompleted.Broadcast();
            StopSpawning();
            
            UE_LOG(LogEnemySpawner, Log, TEXT("[EnemySpawner] 所有波次完成"));
            return;
        }

        // 开始下一波
        StartNextWave();
        return;
    }

    // 波次开始延迟
    const FSpawnWaveInfo& CurrentWave = WaveConfigs[CurrentWaveIndex];
    if (WaveDelayTimer < CurrentWave.WaveStartDelay)
    {
        WaveDelayTimer += DeltaTime;
        return;
    }

    // 生成计时
    SpawnTimer += DeltaTime;
    if (SpawnTimer >= CurrentWave.SpawnInterval)
    {
        SpawnTimer = 0.0f;
        
        if (CanSpawn())
        {
            DoSpawn();
        }
    }
}

void AEnemySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopSpawning();
    CleanupDeadEnemies();
    
    Super::EndPlay(EndPlayReason);
}

void AEnemySpawner::StartSpawning()
{
    if (bIsSpawning)
    {
        return;
    }

    if (WaveConfigs.Num() == 0)
    {
        UE_LOG(LogEnemySpawner, Warning, TEXT("[EnemySpawner] %s 未配置波次"), *GetName());
        return;
    }

    bIsSpawning = true;
    CurrentWaveIndex = 0;
    CurrentWaveSpawnedCount = 0;
    WaveDelayTimer = 0.0f;
    SpawnTimer = 0.0f;

    // 广播波次开始事件
    OnWaveStarted.Broadcast(CurrentWaveIndex);
    
    UE_LOG(LogEnemySpawner, Log, TEXT("[EnemySpawner] %s 开始生成，波次数: %d"),
        *GetName(), WaveConfigs.Num());
}

void AEnemySpawner::StopSpawning()
{
    bIsSpawning = false;
    
    UE_LOG(LogEnemySpawner, Log, TEXT("[EnemySpawner] %s 停止生成"), *GetName());
}

void AEnemySpawner::StartNextWave()
{
    if (CurrentWaveIndex >= WaveConfigs.Num() - 1)
    {
        return;
    }

    CurrentWaveIndex++;
    CurrentWaveSpawnedCount = 0;
    WaveDelayTimer = 0.0f;
    SpawnTimer = 0.0f;

    // 广播波次开始事件
    OnWaveStarted.Broadcast(CurrentWaveIndex);
    
    UE_LOG(LogEnemySpawner, Log, TEXT("[EnemySpawner] 开始波次 %d"), CurrentWaveIndex);
}

void AEnemySpawner::ResetSpawner()
{
    StopSpawning();
    
    // 清理已生成的敌人
    for (auto& Enemy : SpawnedEnemies)
    {
        if (Enemy.IsValid() && !Enemy->IsPendingKillPending())
        {
            Enemy->Destroy();
        }
    }
    SpawnedEnemies.Empty();

    CurrentWaveIndex = 0;
    CurrentWaveSpawnedCount = 0;
    TotalSpawnedCount = 0;
    WaveDelayTimer = 0.0f;
    SpawnTimer = 0.0f;
    
    UE_LOG(LogEnemySpawner, Log, TEXT("[EnemySpawner] %s 已重置"), *GetName());
}

int32 AEnemySpawner::GetAliveEnemyCount() const
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

bool AEnemySpawner::IsCurrentWaveComplete() const
{
    if (CurrentWaveIndex >= WaveConfigs.Num())
    {
        return true;
    }

    const FSpawnWaveInfo& CurrentWave = WaveConfigs[CurrentWaveIndex];

    // 检查是否生成完所有敌人
    if (CurrentWaveSpawnedCount < CurrentWave.EnemyCount)
    {
        return false;
    }

    // 如果需要等待清理，检查是否还有存活的敌人
    if (CurrentWave.bWaitForClear)
    {
        return GetAliveEnemyCount() == 0;
    }

    return true;
}

bool AEnemySpawner::IsAllWavesComplete() const
{
    return CurrentWaveIndex >= WaveConfigs.Num() - 1 && IsCurrentWaveComplete();
}

void AEnemySpawner::CollectSpawnPoints()
{
    SpawnPoints.Empty();

    // 获取所有子组件中的生成点
    // 注意：EnemySpawner是Actor，直接使用GetComponents()，不需要GetOwner()
    TArray<UActorComponent*> Components;
    GetComponents(UEnemySpawnPoint::StaticClass(), Components);
    for (UActorComponent* Component : Components)
    {
        if (UEnemySpawnPoint* SpawnPoint = Cast<UEnemySpawnPoint>(Component))
        {
            SpawnPoints.Add(SpawnPoint);
        }
    }

    // 如果没有找到子组件，搜索范围内的生成点
    if (SpawnPoints.Num() == 0)
    {
        // 获取场景中的所有生成点
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (UEnemySpawnPoint* SpawnPoint = Actor->FindComponentByClass<UEnemySpawnPoint>())
            {
                // 检查是否在范围内
                float Distance = FVector::Dist(Actor->GetActorLocation(), GetActorLocation());
                if (Distance < 2000.0f) // 2米范围内
                {
                    SpawnPoints.Add(SpawnPoint);
                }
            }
        }
    }
}

void AEnemySpawner::DoSpawn()
{
    if (CurrentWaveIndex >= WaveConfigs.Num())
    {
        return;
    }

    const FSpawnWaveInfo& CurrentWave = WaveConfigs[CurrentWaveIndex];

    // 获取生成点
    UEnemySpawnPoint* SpawnPoint = GetRandomSpawnPoint();
    if (!SpawnPoint)
    {
        UE_LOG(LogEnemySpawner, Warning, TEXT("[EnemySpawner] 没有可用的生成点"));
        return;
    }

    // 获取敌人类型
    TSubclassOf<AEnemyBase> EnemyClass = GetRandomEnemyClass();
    if (!EnemyClass)
    {
        UE_LOG(LogEnemySpawner, Warning, TEXT("[EnemySpawner] 没有可用的敌人类型"));
        return;
    }

    // 设置生成点配置
    SpawnPoint->EnemyClass = EnemyClass;

    // 执行生成
    AActor* SpawnedActor = SpawnPoint->SpawnEnemy();
    if (AEnemyBase* SpawnedEnemy = Cast<AEnemyBase>(SpawnedActor))
    {
        SpawnedEnemies.Add(SpawnedEnemy);
        CurrentWaveSpawnedCount++;
        TotalSpawnedCount++;

        // 广播事件
        OnEnemySpawned.Broadcast(SpawnedEnemy);
        
        UE_LOG(LogEnemySpawner, Log, TEXT("[EnemySpawner] 波次 %d 生成敌人 %d/%d: %s"),
            CurrentWaveIndex, CurrentWaveSpawnedCount, CurrentWave.EnemyCount, *SpawnedEnemy->GetName());
    }
}

UEnemySpawnPoint* AEnemySpawner::GetRandomSpawnPoint() const
{
    if (SpawnPoints.Num() == 0)
    {
        return nullptr;
    }

    // 筛选可用的生成点
    TArray<UEnemySpawnPoint*> AvailablePoints;
    for (UEnemySpawnPoint* Point : SpawnPoints)
    {
        if (Point && Point->CanSpawn())
        {
            AvailablePoints.Add(Point);
        }
    }

    if (AvailablePoints.Num() == 0)
    {
        return nullptr;
    }

    // 随机选择一个
    int32 RandomIndex = FMath::RandRange(0, AvailablePoints.Num() - 1);
    return AvailablePoints[RandomIndex];
}

TSubclassOf<AEnemyBase> AEnemySpawner::GetRandomEnemyClass() const
{
    if (CurrentWaveIndex >= WaveConfigs.Num())
    {
        return nullptr;
    }

    const FSpawnWaveInfo& CurrentWave = WaveConfigs[CurrentWaveIndex];
    
    if (CurrentWave.EnemyClasses.Num() == 0)
    {
        return nullptr;
    }

    // 随机选择一个敌人类型
    int32 RandomIndex = FMath::RandRange(0, CurrentWave.EnemyClasses.Num() - 1);
    return CurrentWave.EnemyClasses[RandomIndex];
}

void AEnemySpawner::CleanupDeadEnemies()
{
    for (int32 i = SpawnedEnemies.Num() - 1; i >= 0; --i)
    {
        if (!SpawnedEnemies[i].IsValid() || SpawnedEnemies[i]->IsPendingKillPending())
        {
            SpawnedEnemies.RemoveAt(i);
        }
    }
}

bool AEnemySpawner::CanSpawn() const
{
    // 检查最大存活数量
    if (MaxAliveEnemies > 0)
    {
        if (GetAliveEnemyCount() >= MaxAliveEnemies)
        {
            return false;
        }
    }

    // 检查波次配置
    if (CurrentWaveIndex >= WaveConfigs.Num())
    {
        return false;
    }

    const FSpawnWaveInfo& CurrentWave = WaveConfigs[CurrentWaveIndex];
    
    // 检查是否生成完本波次所有敌人
    if (CurrentWaveSpawnedCount >= CurrentWave.EnemyCount)
    {
        return false;
    }

    return true;
}

void AEnemySpawner::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 检查是否是玩家
    if (!OtherActor || !OtherActor->ActorHasTag(FName(TEXT("Player"))))
    {
        return;
    }

    // 开始生成
    if (!bIsSpawning)
    {
        StartSpawning();
        
        UE_LOG(LogEnemySpawner, Log, TEXT("[EnemySpawner] 玩家触发生成器 %s"), *GetName());
    }
}
