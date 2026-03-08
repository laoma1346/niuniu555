// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_Investigate.generated.h"

// 声明日志分类
DECLARE_LOG_CATEGORY_EXTERN(LogAIInvestigate, Log, All);

/**
 * 侦查任务 - 前往感知到的位置进行侦查
 * 融合蓝图AI系统的侦查逻辑
 */
UCLASS()
class NIUNIU555_API UBTTask_Investigate : public UBTTask_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTTask_Investigate();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
    /** 侦查位置Key */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Investigate")
    FBlackboardKeySelector InvestigateLocationKey;

    /** 到达后的侦查时间 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Investigate", meta = (ClampMin = "0.5"))
    float InvestigateTime = 2.0f;

    /** 接受半径 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Investigate", meta = (ClampMin = "10.0"))
    float AcceptanceRadius = 100.0f;

    /** 侦查时是否旋转观察 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Investigate")
    bool bRotateWhileWaiting = true;

    /** 旋转速度 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Investigate", meta = (ClampMin = "0.0"))
    float RotationSpeed = 90.0f;

private:
    // 是否已到达
    bool bHasArrived = false;
    
    // 侦查计时器
    float InvestigateTimer = 0.0f;
    
    // 当前旋转角度
    float CurrentRotation = 0.0f;
};
