// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Public/AI/AIStateTypes.h"
#include "EnemySystem/EnemyTypes.h"
#include "EnemyAIController.generated.h"

// 日志分类声明
DECLARE_LOG_CATEGORY_EXTERN(LogEnemyAI, Log, All);

// 前置声明
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
class UBehaviorTree;
class UBlackboardComponent;
class AEnemyBase;

// 委托声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorTargetChanged, AActor*, OldTarget, AActor*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIStateChanged, EAIBehaviorState, OldState, EAIBehaviorState, NewState);

/**
 * 敌人AI控制器 - 增强版（融合蓝图AI系统特性）
 * 整合感知系统、状态管理、EQS
 */
UCLASS()
class NIUNIU555_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController(const FObjectInitializer& ObjectInitializer);

    // ========== 生命周期 ==========
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void Tick(float DeltaTime) override;

    // ========== 黑板Key名称（静态常量）==========
    static const FName TargetActorKey;
    static const FName HomeLocationKey;
    static const FName PatrolRadiusKey;
    static const FName PatrolCenterKey;
    static const FName ChaseStartLocationKey;
    static const FName AIStateKey;              // 新增：AI状态枚举
    static const FName LastKnownLocationKey;    // 新增：最后已知位置
    static const FName InvestigateLocationKey;  // 新增：侦查位置
    static const FName SenseTypeKey;            // 新增：感知类型

    // ========== 目标管理 ==========
    UFUNCTION(BlueprintPure, Category = "AI|目标")
    AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

    UFUNCTION(BlueprintCallable, Category = "AI|目标")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI|目标")
    void ClearTarget();

    // ========== 感知系统 ==========
    // 使用父类的 GetAIPerceptionComponent()

    UFUNCTION(BlueprintPure, Category = "AI|感知")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "AI|感知")
    FVector GetLastKnownLocation() const { return LastKnownLocation; }

    // ========== 状态管理 ==========
    UFUNCTION(BlueprintPure, Category = "AI|状态")
    EAIBehaviorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "AI|状态")
    void SetState(EAIBehaviorState NewState);

    // ========== 状态切换函数 ==========
    UFUNCTION(BlueprintCallable, Category = "AI|状态")
    void StartIdleState();

    UFUNCTION(BlueprintCallable, Category = "AI|状态")
    void StartPatrolState();

    UFUNCTION(BlueprintCallable, Category = "AI|状态")
    void StartInvestigateState(FVector Location, EAISenseType SenseType);

    UFUNCTION(BlueprintCallable, Category = "AI|状态")
    void StartChaseState();

    UFUNCTION(BlueprintCallable, Category = "AI|状态")
    void StartAttackState();

    UFUNCTION(BlueprintCallable, Category = "AI|状态")
    void StartSearchState();

    UFUNCTION(BlueprintCallable, Category = "AI|状态")
    void StartReturnState();

    UFUNCTION(BlueprintCallable, Category = "AI|状态")
    void StartDeadState();

    // ========== 委托事件 ==========
    UPROPERTY(BlueprintAssignable, Category = "AI|事件")
    FOnActorTargetChanged OnTargetChanged;

    UPROPERTY(BlueprintAssignable, Category = "AI|事件")
    FOnAIStateChanged OnStateChanged;

protected:
    // ========== 感知组件 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "组件")
    TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Damage> DamageConfig;

    // ========== 行为树 ==========
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI配置")
    TSoftObjectPtr<UBehaviorTree> BehaviorTree;

    // 注意：父类AAIController已经有Blackboard成员，这里使用GetBlackboardComponent()获取

    // ========== 状态 ==========
    UPROPERTY(BlueprintReadOnly, Category = "状态")
    EAIBehaviorState CurrentState = EAIBehaviorState::None;

    UPROPERTY(BlueprintReadOnly, Category = "状态")
    EAIBehaviorState PreviousState = EAIBehaviorState::None;

    // ========== 目标 ==========
    UPROPERTY(BlueprintReadOnly, Category = "目标")
    TWeakObjectPtr<AActor> CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "目标")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "目标")
    FVector InvestigateLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "目标")
    EAISenseType LastSenseType = EAISenseType::None;

    // ========== 感知相关 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "感知配置")
    float TargetLostDelay = 1.5f;  // 目标丢失延迟时间（感知遗忘后备）

    /** 可配置的目标标签（替代硬编码"Player"） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "感知配置")
    FName TargetActorTag = FName(TEXT("Player"));

    float TargetLostTimer = 0.0f;

    // ========== 内部状态 ==========
    bool bInitialized = false;
    TWeakObjectPtr<AEnemyBase> ControlledEnemy;

    // ========== 回调函数 ==========
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ========== 内部函数 ==========
    void LoadAIConfigFromDataAsset();
    void StartBehaviorTree();
    void InitializeBlackboardKeys();
    void UpdateState(float DeltaTime);

    // 处理不同感知类型
    void HandleSightPerception(AActor* Actor, const FAIStimulus& Stimulus);
    void HandleHearingPerception(AActor* Actor, const FAIStimulus& Stimulus);
    void HandleDamagePerception(AActor* Actor, const FAIStimulus& Stimulus);
};
