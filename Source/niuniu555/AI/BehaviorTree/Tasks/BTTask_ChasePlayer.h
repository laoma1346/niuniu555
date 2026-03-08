// Fill in your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_ChasePlayer.generated.h"

// 追击任务日志分类
DECLARE_LOG_CATEGORY_EXTERN(LogAIChase, Log, All);

/**
 * 追击任务实例内存结构体
 * 用于存储每个任务实例的独立数据，确保线程安全和多实例安全
 */
USTRUCT()
struct FChaseTaskMemory
{
    GENERATED_BODY()

    /** 重新寻路计时器 */
    UPROPERTY()
    float RepathTimer = 0.0f;
    
    /** 追击起始位置（用于计算最大追击距离） */
    UPROPERTY()
    FVector ChaseStartLocation = FVector::ZeroVector;
    
    /** 上次目标位置（用于检测目标是否在移动） */
    UPROPERTY()
    FVector LastTargetLocation = FVector::ZeroVector;
    
    /** 下次距离检查时间（用于优化性能） */
    UPROPERTY()
    float NextDistanceCheckTime = 0.0f;
    
    /** 是否已进入攻击范围（防止攻击-追击震荡） */
    UPROPERTY()
    bool bHasEnteredAttackRange = false;
};

/**
 * 追击玩家任务节点
 * 持续追击目标Actor，直到进入攻击范围或丢失目标
 * 
 * 注意：此类使用实例内存（Instance Memory）来存储每个任务实例的状态，
 * 确保在多线程环境和多个AI同时使用此任务时的线程安全性
 */
UCLASS()
class NIUNIU555_API UBTTask_ChasePlayer : public UBTTask_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTTask_ChasePlayer();

    //~ Begin UBTTaskNode Interface
    virtual uint16 GetInstanceMemorySize() const override;
    virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
    //~ End UBTTaskNode Interface

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
    /** 
     * 类型安全地获取追击任务内存 
     * @param NodeMemory 原始节点内存指针
     * @return 类型转换后的FChaseTaskMemory指针
     */
    FORCEINLINE static FChaseTaskMemory* GetChaseMemory(uint8* NodeMemory)
    {
        return reinterpret_cast<FChaseTaskMemory*>(NodeMemory);
    }

    /** 检查是否应该停止追击（超出最大追击距离） */
    bool ShouldAbortChase(AAIController* AIController, APawn* AIPawn, AActor* TargetActor, const FChaseTaskMemory* ChaseMemory) const;
    
    /** 检查是否进入攻击范围 */
    bool IsInAttackRange(APawn* AIPawn, AActor* TargetActor) const;

public:
    /** 目标Actor的Blackboard键 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "追击任务")
    FBlackboardKeySelector TargetActorKey;

    /** 追击起始位置的Blackboard键（用于计算最大追击距离） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "追击任务")
    FBlackboardKeySelector ChaseStartLocationKey;

    /** 攻击距离（进入此距离返回成功） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "追击任务", meta = (ClampMin = "50.0"))
    float AttackRange = 250.0f;

    /** 最大追击距离（超出此距离返回失败） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "追击任务", meta = (ClampMin = "100.0"))
    float MaxChaseDistance = 5000.0f;

    /** 到达目标点的接受半径 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "追击任务", meta = (ClampMin = "10.0"))
    float AcceptanceRadius = 50.0f;

    /** 重新寻路间隔（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "追击任务", meta = (ClampMin = "0.1"))
    float RepathInterval = 0.5f;
    
    /** 距离检查间隔（秒）- 用于性能优化 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "追击任务|性能优化", meta = (ClampMin = "0.05"))
    float DistanceCheckInterval = 0.2f;
};
