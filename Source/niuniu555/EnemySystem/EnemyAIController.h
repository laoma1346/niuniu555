// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class UAIPerceptionComponent;
class AEnemyBase;

/**
 * 敌人AI控制器 - 控制EnemyBase的AI行为
 * 文件路径: Source/niuniu555/EnemySystem/EnemyAIController.h
 * 功能说明: 
 *   - 运行行为树控制敌人行为
 *   - 管理AI感知（视觉/听觉）
 *   - 与EnemyStateMachineComponent交互
 *   - 支持从EnemyDataAsset加载配置
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class NIUNIU555_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // ========== AI生命周期 ==========
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ========== 行为树控制 ==========

    // 启动行为树
    UFUNCTION(BlueprintCallable, Category = "AI行为树")
    void StartBehaviorTree();

    // 停止行为树
    UFUNCTION(BlueprintCallable, Category = "AI行为树")
    void StopBehaviorTree();

    // 重启行为树（用于状态重置）
    UFUNCTION(BlueprintCallable, Category = "AI行为树")
    void RestartBehaviorTree();

    // 是否正在运行行为树
    UFUNCTION(BlueprintPure, Category = "AI行为树")
    bool IsBehaviorTreeRunning() const;

    // ========== 目标管理 ==========

    // 获取当前攻击目标（黑板Key：TargetActor）
    UFUNCTION(BlueprintPure, Category = "AI目标")
    AActor* GetTargetActor() const;

    // 设置攻击目标
    UFUNCTION(BlueprintCallable, Category = "AI目标")
    void SetTargetActor(AActor* NewTarget);

    // 清除目标
    UFUNCTION(BlueprintCallable, Category = "AI目标")
    void ClearTargetActor();

    // 获取家位置（出生点，黑板Key：HomeLocation）
    UFUNCTION(BlueprintPure, Category = "AI目标")
    FVector GetHomeLocation() const;

    // ========== 状态检查 ==========

    // 检查是否有有效目标
    UFUNCTION(BlueprintPure, Category = "AI状态")
    bool HasValidTarget() const;

    // 检查目标是否在攻击范围内
    UFUNCTION(BlueprintPure, Category = "AI状态")
    bool IsTargetInAttackRange() const;

    // 检查目标是否在追击范围内
    UFUNCTION(BlueprintPure, Category = "AI状态")
    bool IsTargetInChaseRange() const;

    // 检查是否在巡逻范围内（距离出生点）
    UFUNCTION(BlueprintPure, Category = "AI状态")
    bool IsWithinPatrolRange() const;

    // ========== 组件访问 ==========

    // 注意：GetAIPerceptionComponent() 已在父类AAIController定义，直接使用即可
    
    UFUNCTION(BlueprintPure, Category = "AI组件")
    UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

    UFUNCTION(BlueprintPure, Category = "AI组件")
    AEnemyBase* GetControlledEnemy() const { return ControlledEnemy; }

protected:
    // ========== 组件 ==========

    // 【修复】显式声明感知组件，确保正确创建和注册
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI组件")
    TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

    // ========== 配置 ==========

    // 行为树资产（可从EnemyDataAsset覆盖）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI配置")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    // 黑板数据资产
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI配置")
    TObjectPtr<UBlackboardData> BlackboardAsset;

    // ========== 运行时状态 ==========

    // 控制的敌人
    UPROPERTY(BlueprintReadOnly, Category = "AI状态")
    TObjectPtr<AEnemyBase> ControlledEnemy;

    // 是否已初始化
    UPROPERTY(BlueprintReadOnly, Category = "AI状态")
    bool bInitialized;

    // ========== 内部函数 ==========

    // 从EnemyDataAsset加载AI配置
    UFUNCTION()
    virtual void LoadAIConfigFromDataAsset();

    // 初始化黑板键
    UFUNCTION()
    virtual void InitializeBlackboardKeys();

    // 感知更新回调
    UFUNCTION()
    virtual void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // 目标改变回调
    UFUNCTION()
    virtual void OnTargetChanged(AActor* NewTarget);

    // ========== 黑板Key名称（常量）==========
public:
    // 黑板Key名称定义（方便行为树任务节点使用）
    static const FName TargetActorKey;      // 攻击目标
    static const FName HomeLocationKey;     // 出生点位置
    static const FName PatrolCenterKey;     // 巡逻中心
    static const FName PatrolRadiusKey;     // 巡逻半径
    static const FName AttackRangeKey;      // 攻击范围
    static const FName ChaseRangeKey;       // 追击范围
    static const FName CanAttackKey;        // 是否可以攻击
    static const FName IsInCombatKey;       // 是否处于战斗状态
};
