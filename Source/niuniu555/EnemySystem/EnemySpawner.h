// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyBase.h"
#include "EnemySpawner.generated.h"

// 日志分类声明
DECLARE_LOG_CATEGORY_EXTERN(LogEnemySpawner, Log, All);

// 前置声明
class UEnemySpawnPoint;
class UBoxComponent;

/** 生成器事件委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStarted, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveCompleted, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemySpawnedBySpawner, AEnemyBase*, SpawnedEnemy);

/**
 * 波次信息结构
 */
USTRUCT(BlueprintType)
struct FSpawnWaveInfo
{
    GENERATED_BODY()

    /** 本波次敌人类型 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "波次配置")
    TArray<TSubclassOf<AEnemyBase>> EnemyClasses;

    /** 本波次敌人数量 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "波次配置", meta = (ClampMin = "1"))
    int32 EnemyCount = 3;

    /** 生成间隔（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "波次配置", meta = (ClampMin = "0.1"))
    float SpawnInterval = 2.0f;

    /** 波次开始前延迟（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "波次配置", meta = (ClampMin = "0.0"))
    float WaveStartDelay = 3.0f;

    /** 本波次完成前是否锁定下一波 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "波次配置")
    bool bWaitForClear = true;
};

/**
 * 敌人生成器 - 管理多个生成点和波次逻辑
 * 放置在场景中，管理该区域的敌人生成
 */
UCLASS()
class NIUNIU555_API AEnemySpawner : public AActor
{
    GENERATED_BODY()

public:
    AEnemySpawner(const FObjectInitializer& ObjectInitializer);

    //~ Begin AActor Interface
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~ End AActor Interface

    /** 开始生成波次 */
    UFUNCTION(BlueprintCallable, Category = "生成器")
    void StartSpawning();

    /** 停止生成 */
    UFUNCTION(BlueprintCallable, Category = "生成器")
    void StopSpawning();

    /** 开始下一波 */
    UFUNCTION(BlueprintCallable, Category = "生成器")
    void StartNextWave();

    /** 重置生成器 */
    UFUNCTION(BlueprintCallable, Category = "生成器")
    void ResetSpawner();

    /** 获取当前波次 */
    UFUNCTION(BlueprintPure, Category = "生成器")
    int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

    /** 获取当前存活的敌人数 */
    UFUNCTION(BlueprintPure, Category = "生成器")
    int32 GetAliveEnemyCount() const;

    /** 获取总生成敌人数 */
    UFUNCTION(BlueprintPure, Category = "生成器")
    int32 GetTotalSpawnedCount() const { return TotalSpawnedCount; }

    /** 检查当前波次是否完成 */
    UFUNCTION(BlueprintPure, Category = "生成器")
    bool IsCurrentWaveComplete() const;

    /** 检查是否所有波次完成 */
    UFUNCTION(BlueprintPure, Category = "生成器")
    bool IsAllWavesComplete() const;

protected:
    /** 触发区域（玩家进入后开始生成） */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "组件")
    TObjectPtr<UBoxComponent> TriggerVolume;

    /** 波次配置列表 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "波次配置")
    TArray<FSpawnWaveInfo> WaveConfigs;

    /** 最大同时存在敌人数（0=无限制） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置", meta = (ClampMin = "0"))
    int32 MaxAliveEnemies = 10;

    /** 是否启用触发区域 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
    bool bUseTriggerVolume = true;

    /** 是否自动开始第一波 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置", meta = (EditCondition = "!bUseTriggerVolume"))
    bool bAutoStart = false;

    /** 关联的生成点列表（自动收集场景中的生成点） */
    UPROPERTY()
    TArray<TObjectPtr<UEnemySpawnPoint>> SpawnPoints;

    /** 当前波次索引 */
    UPROPERTY()
    int32 CurrentWaveIndex = 0;

    /** 本波次已生成数量 */
    UPROPERTY()
    int32 CurrentWaveSpawnedCount = 0;

    /** 总生成数量 */
    UPROPERTY()
    int32 TotalSpawnedCount = 0;

    /** 是否正在生成 */
    UPROPERTY()
    bool bIsSpawning = false;

    /** 生成计时器 */
    UPROPERTY()
    float SpawnTimer = 0.0f;

    /** 波次延迟计时器 */
    UPROPERTY()
    float WaveDelayTimer = 0.0f;

    /** 当前生成的所有敌人 */
    UPROPERTY()
    TArray<TWeakObjectPtr<AEnemyBase>> SpawnedEnemies;

protected:
    /** 收集场景中的生成点 */
    UFUNCTION()
    void CollectSpawnPoints();

    /** 执行生成 */
    UFUNCTION()
    void DoSpawn();

    /** 获取随机生成点 */
    UFUNCTION()
    UEnemySpawnPoint* GetRandomSpawnPoint() const;

    /** 获取随机敌人类型 */
    UFUNCTION()
    TSubclassOf<AEnemyBase> GetRandomEnemyClass() const;

    /** 清理死亡的敌人 */
    UFUNCTION()
    void CleanupDeadEnemies();

    /** 检查是否可以生成 */
    UFUNCTION()
    bool CanSpawn() const;

    /** 触发区域重叠回调 */
    UFUNCTION()
    void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
    /** 波次开始事件 */
    UPROPERTY(BlueprintAssignable, Category = "事件")
    FOnWaveStarted OnWaveStarted;

    /** 波次完成事件 */
    UPROPERTY(BlueprintAssignable, Category = "事件")
    FOnWaveCompleted OnWaveCompleted;

    /** 所有波次完成事件 */
    UPROPERTY(BlueprintAssignable, Category = "事件")
    FOnAllWavesCompleted OnAllWavesCompleted;

    /** 敌人生成事件 */
    UPROPERTY(BlueprintAssignable, Category = "事件")
    FOnEnemySpawnedBySpawner OnEnemySpawned;
};
