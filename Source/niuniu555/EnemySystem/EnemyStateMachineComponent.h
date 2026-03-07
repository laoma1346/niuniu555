// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyTypes.h"
#include "EnemyStateMachineComponent.generated.h"

class UEnemyDataAsset;

/**
 * 状态转换规则结构体
 * 定义哪些状态可以转换到哪些状态
 */
USTRUCT(BlueprintType)
struct FStateTransitionRule
{
    GENERATED_BODY()

    // 起始状态
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "状态规则")
    EEnemyState FromState;

    // 目标状态
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "状态规则")
    EEnemyState ToState;

    // 是否需要满足特定条件（由蓝图/代码实现）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "状态规则")
    bool bRequireCondition;

    // 转换优先级（高优先级可以打断低优先级）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "状态规则", meta = (ClampMin = 0))
    int32 Priority;

    // 是否可以被强制转换（如受击强制打断）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "状态规则")
    bool bCanBeForced;

    FStateTransitionRule()
        : FromState(EEnemyState::None)
        , ToState(EEnemyState::None)
        , bRequireCondition(false)
        , Priority(0)
        , bCanBeForced(true)
    {}
};

/**
 * 敌人状态机组件
 * 管理敌人的状态流转（Idle/Patrol/Chase/Attack/Hit/Dead）
 * 文件路径: Source/niuniu555/EnemySystem/EnemyStateMachineComponent.h
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NIUNIU555_API UEnemyStateMachineComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnemyStateMachineComponent();

    // ========== 组件生命周期 ==========
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ========== 状态查询 ==========

    // 获取当前状态
    UFUNCTION(BlueprintPure, Category = "敌人状态机")
    EEnemyState GetCurrentState() const { return CurrentState; }

    // 获取上一个状态
    UFUNCTION(BlueprintPure, Category = "敌人状态机")
    EEnemyState GetPreviousState() const { return PreviousState; }

    // 检查是否处于指定状态
    UFUNCTION(BlueprintPure, Category = "敌人状态机")
    bool IsInState(EEnemyState State) const { return CurrentState == State; }

    // 检查当前状态是否是战斗相关状态
    UFUNCTION(BlueprintPure, Category = "敌人状态机")
    bool IsInCombatState() const;

    // 检查当前是否可以攻击
    UFUNCTION(BlueprintPure, Category = "敌人状态机")
    bool CanAttack() const;

    // 检查当前是否可以移动
    UFUNCTION(BlueprintPure, Category = "敌人状态机")
    bool CanMove() const;

    // 检查当前是否可以被击退
    UFUNCTION(BlueprintPure, Category = "敌人状态机")
    bool CanBeKnockedBack() const;

    // 检查当前是否可以被眩晕
    UFUNCTION(BlueprintPure, Category = "敌人状态机")
    bool CanBeStunned() const;

    // 获取当前状态持续时间
    UFUNCTION(BlueprintPure, Category = "敌人状态机")
    float GetStateDuration() const { return StateTimer; }

    // ========== 状态转换 ==========

    // 请求状态转换（会检查转换规则）
    UFUNCTION(BlueprintCallable, Category = "敌人状态机")
    bool RequestStateChange(EEnemyState NewState, bool bForce = false);

    // 强制状态转换（无视规则）
    UFUNCTION(BlueprintCallable, Category = "敌人状态机")
    void ForceStateChange(EEnemyState NewState);

    // 返回到上一个状态
    UFUNCTION(BlueprintCallable, Category = "敌人状态机")
    bool ReturnToPreviousState();

    // ========== 便捷状态切换 ==========

    // 切换到出生状态
    UFUNCTION(BlueprintCallable, Category = "敌人状态机")
    bool StartSpawn();

    // 切换到待机状态
    UFUNCTION(BlueprintCallable, Category = "敌人状态机")
    bool StartIdle();

    // 切换到巡逻状态
    UFUNCTION(BlueprintCallable, Category = "敌人状态机")
    bool StartPatrol();

    // 切换到追击状态
    UFUNCTION(BlueprintCallable, Category = "敌人状态机")
    bool StartChase();

    // 切换到攻击状态
    UFUNCTION(BlueprintCallable, Category = "敌人状态机")
    bool StartAttack();

    // 切换到受击状态
    UFUNCTION(BlueprintCallable, Category = "敌人状态机")
    bool StartHit();

    // 切换到眩晕状态
    UFUNCTION(BlueprintCallable, Category = "敌人状态机")
    bool StartStun();

    // 切换到返回状态
    UFUNCTION(BlueprintCallable, Category = "敌人状态机")
    bool StartReturn();

    // 切换到死亡状态
    UFUNCTION(BlueprintCallable, Category = "敌人状态机")
    bool StartDeath();

    // ========== 配置 ==========

    // 关联的敌人数据资产
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "状态机配置")
    TObjectPtr<UEnemyDataAsset> EnemyData;

    // 状态转换规则列表（可选，不设置则使用默认规则）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "状态机配置")
    TArray<FStateTransitionRule> TransitionRules;

    // 是否启用调试日志
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "状态机配置")
    bool bDebugLog;

    // ========== 委托事件 ==========

    // 状态变更事件
    UPROPERTY(BlueprintAssignable, Category = "敌人状态机|事件")
    FOnEnemyStateChanged OnStateChanged;

    // 特定状态进入事件（BlueprintCallable用于绑定）
    UPROPERTY(BlueprintAssignable, Category = "敌人状态机|事件")
    FOnEnemyStateChanged OnEnterSpawn;

    UPROPERTY(BlueprintAssignable, Category = "敌人状态机|事件")
    FOnEnemyStateChanged OnEnterIdle;

    UPROPERTY(BlueprintAssignable, Category = "敌人状态机|事件")
    FOnEnemyStateChanged OnEnterPatrol;

    UPROPERTY(BlueprintAssignable, Category = "敌人状态机|事件")
    FOnEnemyStateChanged OnEnterChase;

    UPROPERTY(BlueprintAssignable, Category = "敌人状态机|事件")
    FOnEnemyStateChanged OnEnterAttack;

    UPROPERTY(BlueprintAssignable, Category = "敌人状态机|事件")
    FOnEnemyStateChanged OnEnterHit;

    UPROPERTY(BlueprintAssignable, Category = "敌人状态机|事件")
    FOnEnemyStateChanged OnEnterStun;

    UPROPERTY(BlueprintAssignable, Category = "敌人状态机|事件")
    FOnEnemyStateChanged OnEnterReturn;

    UPROPERTY(BlueprintAssignable, Category = "敌人状态机|事件")
    FOnEnemyStateChanged OnEnterDead;

protected:
    // ========== 内部状态 ==========

    // 当前状态
    UPROPERTY(BlueprintReadOnly, Category = "敌人状态机")
    EEnemyState CurrentState;

    // 上一个状态
    UPROPERTY(BlueprintReadOnly, Category = "敌人状态机")
    EEnemyState PreviousState;

    // 状态计时器
    UPROPERTY(BlueprintReadOnly, Category = "敌人状态机")
    float StateTimer;

    // 状态总持续时间（用于动画等）
    UPROPERTY(BlueprintReadOnly, Category = "敌人状态机")
    float StateDuration;

    // 是否处于转换中
    UPROPERTY(BlueprintReadOnly, Category = "敌人状态机")
    bool bIsTransitioning;

    // ========== 状态生命周期 ==========

    // 进入状态
    virtual void EnterState(EEnemyState NewState);

    // 退出状态
    virtual void ExitState(EEnemyState OldState);

    // 更新状态
    virtual void UpdateState(float DeltaTime);

    // ========== 各状态处理 ==========

    // 出生状态更新
    virtual void UpdateSpawn(float DeltaTime);

    // 待机状态更新
    virtual void UpdateIdle(float DeltaTime);

    // 巡逻状态更新
    virtual void UpdatePatrol(float DeltaTime);

    // 追击状态更新
    virtual void UpdateChase(float DeltaTime);

    // 攻击状态更新
    virtual void UpdateAttack(float DeltaTime);

    // 受击状态更新
    virtual void UpdateHit(float DeltaTime);

    // 眩晕状态更新
    virtual void UpdateStun(float DeltaTime);

    // 返回状态更新
    virtual void UpdateReturn(float DeltaTime);

    // 死亡状态更新
    virtual void UpdateDead(float DeltaTime);

    // ========== 工具函数 ==========

    // 检查状态转换是否合法
    bool CanTransitionTo(EEnemyState FromState, EEnemyState ToState, bool bForce) const;

    // 获取状态转换规则
    const FStateTransitionRule* FindTransitionRule(EEnemyState FromState, EEnemyState ToState) const;

    // 广播状态事件
    void BroadcastStateEvent(EEnemyState State);

    // 调试日志
    void LogStateChange(EEnemyState FromState, EEnemyState ToState);
};
