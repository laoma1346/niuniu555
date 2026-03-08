// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "DropItemBase.generated.h"

// 日志分类声明
DECLARE_LOG_CATEGORY_EXTERN(LogDropItem, Log, All);

// 前置声明
class APawn;

/** 拾取事件委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDropItemPicked, class ADropItemBase*, DropItem, APawn*, Player);

/**
 * 掉落物基类 - 所有掉落物的基类（金币、装备、神格碎片等）
 * 支持物理掉落和自动吸附功能
 */
UCLASS(Abstract, Blueprintable)
class NIUNIU555_API ADropItemBase : public AActor
{
    GENERATED_BODY()

public:
    ADropItemBase(const FObjectInitializer& ObjectInitializer);

    //~ Begin AActor Interface
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    //~ End AActor Interface

    /** 初始化掉落物（由生成器调用） */
    UFUNCTION(BlueprintCallable, Category = "掉落物")
    virtual void InitializeDrop(const FVector& SpawnLocation, const FVector& SpawnImpulse);

    /** 设置目标玩家（用于自动吸附） */
    UFUNCTION(BlueprintCallable, Category = "掉落物")
    void SetTargetPlayer(APawn* Player);

    /** 获取掉落物价值（金币数、装备品质等） */
    UFUNCTION(BlueprintPure, Category = "掉落物")
    virtual int32 GetValue() const { return 0; }

protected:
    /** 静态网格体组件 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "组件")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    /** 碰撞检测球体（用于玩家拾取） */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "组件")
    TObjectPtr<USphereComponent> PickupRadius;

    /** 是否启用自动吸附 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "吸附配置")
    bool bEnableAutoMagnet = true;

    /** 吸附触发距离 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "吸附配置", meta = (ClampMin = "50.0"))
    float MagnetRange = 300.0f;

    /** 吸附速度 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "吸附配置", meta = (ClampMin = "1.0"))
    float MagnetSpeed = 800.0f;

    /** 吸附加速度 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "吸附配置", meta = (ClampMin = "0.0"))
    float MagnetAcceleration = 2000.0f;

    /** 存在时间（秒）- 0表示永久存在 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生命周期", meta = (ClampMin = "0.0"))
    float LifeTime = 30.0f;

    /** 闪烁开始时间（秒）- 从此时开始闪烁提示即将消失 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生命周期", meta = (ClampMin = "0.0"))
    float BlinkStartTime = 5.0f;

    /** 是否正在吸附 */
    UPROPERTY(BlueprintReadOnly, Category = "状态")
    bool bIsMagneting = false;

    /** 当前吸附速度 */
    UPROPERTY()
    float CurrentMagnetSpeed = 0.0f;

    /** 目标玩家 */
    UPROPERTY()
    TWeakObjectPtr<APawn> TargetPlayer;

    /** 已存在时间 */
    UPROPERTY()
    float AliveTime = 0.0f;

    /** 是否正在闪烁 */
    UPROPERTY()
    bool bIsBlinking = false;

    /** 是否已被拾取（防止重复触发） */
    UPROPERTY()
    bool bAlreadyPicked = false;

    /** 生成后冷却时间（秒）- 此期间不触发吸附，让玩家看到掉落物 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "吸附配置", meta = (ClampMin = "0.0"))
    float SpawnCooldown = 1.0f;

protected:
    /** 检测玩家是否在吸附范围内 */
    UFUNCTION()
    void CheckPlayerInRange();

    /** 执行吸附移动 */
    UFUNCTION()
    void UpdateMagnetMovement(float DeltaTime);

    /** 执行拾取（由子类实现具体逻辑） */
    UFUNCTION(BlueprintNativeEvent, Category = "掉落物")
    void OnPickup(APawn* Player);
    virtual void OnPickup_Implementation(APawn* Player);

    /** 拾取半径重叠回调 */
    UFUNCTION()
    void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /** 开始闪烁 */
    UFUNCTION()
    void StartBlinking();

    /** 更新闪烁效果 */
    UFUNCTION()
    void UpdateBlink(float DeltaTime);

    /** 生命周期结束，销毁自身 */
    UFUNCTION()
    void ExpireAndDestroy();

public:
    /** 拾取事件委托 */
    UPROPERTY(BlueprintAssignable, Category = "事件")
    FOnDropItemPicked OnDropItemPicked;
};
