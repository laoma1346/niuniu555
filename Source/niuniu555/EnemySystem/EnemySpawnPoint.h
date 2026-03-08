// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyTypes.h"
#include "EnemySpawnPoint.generated.h"

// 日志分类声明
DECLARE_LOG_CATEGORY_EXTERN(LogEnemySpawn, Log, All);

// 前置声明
class UEnemyDataAsset;
class AEnemyBase;

/** 生成事件委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemySpawned, AEnemyBase*, SpawnedEnemy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpawnCompleted);

/**
 * 敌人生成点组件
 * 标记场景中的敌人生成位置，配置生成参数
 * 可放置在场景中的任意Actor上（如SpawnVolume、Trigger等）
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NIUNIU555_API UEnemySpawnPoint : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnemySpawnPoint();

    //~ Begin UActorComponent Interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~ End UActorComponent Interface

    /** 检查是否可以生成敌人（冷却时间/最大数量检查） */
    UFUNCTION(BlueprintCallable, Category = "生成点")
    bool CanSpawn() const;

    /** 执行生成 */
    UFUNCTION(BlueprintCallable, Category = "生成点")
    AActor* SpawnEnemy();

    /** 获取生成位置 */
    UFUNCTION(BlueprintPure, Category = "生成点")
    FVector GetSpawnLocation() const;

    /** 获取生成旋转 */
    UFUNCTION(BlueprintPure, Category = "生成点")
    FRotator GetSpawnRotation() const;

    /** 设置生成冷却 */
    UFUNCTION(BlueprintCallable, Category = "生成点")
    void StartCooldown();

public:
    /** 敌人数据资产（决定生成什么敌人） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
    TSubclassOf<AEnemyBase> EnemyClass;

    /** 生成半径（随机偏移范围） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置", meta = (ClampMin = "0.0"))
    float SpawnRadius = 0.0f;

    /** 生成高度偏移（防止卡地面） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置", meta = (ClampMin = "0.0"))
    float SpawnHeightOffset = 100.0f;

    /** 生成冷却时间（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置", meta = (ClampMin = "0.0"))
    float CooldownTime = 5.0f;

    /** 最大同时存在数（0表示无限制） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置", meta = (ClampMin = "0"))
    int32 MaxAliveCount = 1;

    /** 是否启用自动生成 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "自动生成")
    bool bAutoSpawn = false;

    /** 自动生成间隔（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "自动生成", meta = (EditCondition = "bAutoSpawn", ClampMin = "0.1"))
    float AutoSpawnInterval = 10.0f;

    /** 生成视觉效果（粒子系统） */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "视觉效果")
    TSoftObjectPtr<UParticleSystem> SpawnEffect;

protected:
    /** 当前由该生成点生成的敌人列表 */
    UPROPERTY()
    TArray<TWeakObjectPtr<AEnemyBase>> SpawnedEnemies;

    /** 上次生成时间 */
    UPROPERTY()
    float LastSpawnTime = -999.0f;

    /** 自动生成计时器 */
    UPROPERTY()
    float AutoSpawnTimer = 0.0f;

    /** 清理已死亡的敌人引用 */
    UFUNCTION()
    void CleanupDeadEnemies();

    /** 获取当前存活的敌人数量 */
    UFUNCTION()
    int32 GetAliveEnemyCount() const;

public:
    /** 生成事件委托 */
    UPROPERTY(BlueprintAssignable, Category = "事件")
    FOnEnemySpawned OnEnemySpawned;

    /** 生成完成委托 */
    UPROPERTY(BlueprintAssignable, Category = "事件")
    FOnSpawnCompleted OnSpawnCompleted;
};
