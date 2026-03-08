// Fill in your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_Patrol.generated.h"

// 巡逻任务日志分类
DECLARE_LOG_CATEGORY_EXTERN(LogAIPatrol, Log, All);

/**
 * 巡逻任务节点内存结构体
 * 用于存储每个AI实例的独立状态，避免多实例间状态共享
 */
USTRUCT()
struct FPatrolTaskMemory
{
    GENERATED_BODY()

    /** 等待计时器 */
    UPROPERTY()
    float WaitTimer = 0.0f;
    
    /** 是否在等待中 */
    UPROPERTY()
    bool bIsWaiting = false;
    
    /** 是否在移动中 */
    UPROPERTY()
    bool bIsMoving = false;
};

/**
 * 巡逻任务节点
 * 在指定半径内随机移动，到达后等待一段时间
 * 
 * 注意：使用NodeMemory确保每个AI实例有独立的巡逻状态
 */
UCLASS()
class NIUNIU555_API UBTTask_Patrol : public UBTTask_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTTask_Patrol();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
    
    //~ Begin UBTTaskNode Interface
    virtual uint16 GetInstanceMemorySize() const override;
    virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
    //~ End UBTTaskNode Interface

protected:
    /** 在巡逻范围内生成随机巡逻点（考虑导航网格） */
    FVector GetRandomPatrolPoint(const FVector& Center, float Radius, APawn* AIPawn);

    /** 类型安全地获取巡逻任务内存 */
    static FPatrolTaskMemory* GetPatrolMemory(uint8* NodeMemory);

public:
    /** 巡逻中心位置Blackboard键 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "巡逻任务")
    FBlackboardKeySelector PatrolCenterKey;

    /** 巡逻半径Blackboard键 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "巡逻任务")
    FBlackboardKeySelector PatrolRadiusKey;

    /** 在巡逻点等待时间 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "巡逻任务", meta = (ClampMin = "0.0"))
    float WaitTimeAtPoint = 3.0f;

    /** 到达目标点的接受半径 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "巡逻任务", meta = (ClampMin = "10.0"))
    float AcceptanceRadius = 50.0f;
};
