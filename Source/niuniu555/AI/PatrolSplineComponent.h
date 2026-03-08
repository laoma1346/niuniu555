// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PatrolSplineComponent.generated.h"

class USplineComponent;

// 委托声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPatrolPointReached, int32, PointIndex, FVector, PointLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPatrolCompleted);

/**
 * 样条巡逻组件 - 沿样条路径进行巡逻
 * 融合蓝图AI系统的样条巡逻逻辑
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NIUNIU555_API UPatrolSplineComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPatrolSplineComponent();

    // ========== 巡逻控制 ==========
    
    /** 获取下一个巡逻点 */
    UFUNCTION(BlueprintCallable, Category = "巡逻")
    FVector GetNextPatrolPoint();

    /** 获取当前巡逻点位置 */
    UFUNCTION(BlueprintPure, Category = "巡逻")
    FVector GetCurrentPointLocation() const;

    /** 获取当前巡逻点索引 */
    UFUNCTION(BlueprintPure, Category = "巡逻")
    int32 GetCurrentPointIndex() const { return CurrentPointIndex; }

    /** 设置样条组件 */
    UFUNCTION(BlueprintCallable, Category = "巡逻")
    void SetSplineComponent(USplineComponent* Spline);

    /** 获取样条组件 */
    UFUNCTION(BlueprintPure, Category = "巡逻")
    USplineComponent* GetSplineComponent() const { return SplineComponent; }

    // ========== 状态查询 ==========
    
    /** 是否到达终点 */
    UFUNCTION(BlueprintPure, Category = "巡逻")
    bool IsAtEnd() const;

    /** 反转巡逻方向 */
    UFUNCTION(BlueprintCallable, Category = "巡逻")
    void ReverseDirection();

    /** 重置巡逻到起点 */
    UFUNCTION(BlueprintCallable, Category = "巡逻")
    void ResetToStart();

    /** 获取巡逻进度（0.0 - 1.0） */
    UFUNCTION(BlueprintPure, Category = "巡逻")
    float GetPatrolProgress() const;

protected:
    // ========== 配置 ==========
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "巡逻配置")
    TObjectPtr<USplineComponent> SplineComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "巡逻配置")
    int32 CurrentPointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "巡逻配置")
    int32 Direction = 1;  // 1为正向，-1为反向

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "巡逻配置")
    bool bLoop = true;  // 是否循环

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "巡逻配置")
    bool bPingPong = true;  // 是否往返（到达终点后反向）

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "巡逻配置")
    float PointAcceptanceRadius = 100.0f;  // 到达判定半径

public:
    // ========== 委托事件 ==========
    
    UPROPERTY(BlueprintAssignable, Category = "巡逻|事件")
    FOnPatrolPointReached OnPatrolPointReached;

    UPROPERTY(BlueprintAssignable, Category = "巡逻|事件")
    FOnPatrolCompleted OnPatrolCompleted;
};
