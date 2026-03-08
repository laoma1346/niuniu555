// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_Search.generated.h"

// 声明日志分类
DECLARE_LOG_CATEGORY_EXTERN(LogAISearch, Log, All);

/**
 * 搜索任务 - 在目标最后已知位置周围搜索
 * 融合蓝图AI系统的搜索逻辑
 */
UCLASS()
class NIUNIU555_API UBTTask_Search : public UBTTask_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTTask_Search();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
    /** 搜索中心位置Key */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
    FBlackboardKeySelector SearchCenterKey;

    /** 搜索半径 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (ClampMin = "100.0"))
    float SearchRadius = 500.0f;

    /** 搜索持续时间 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (ClampMin = "1.0"))
    float SearchDuration = 5.0f;

    /** 每个搜索点等待时间 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (ClampMin = "0.0"))
    float WaitTimeAtPoint = 1.0f;

    /** 搜索点数量 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (ClampMin = "1", ClampMax = "10"))
    int32 NumSearchPoints = 3;

    /** 到达判定半径 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (ClampMin = "10.0"))
    float AcceptanceRadius = 100.0f;

private:
    // 生成搜索点
    TArray<FVector> GenerateSearchPoints(const FVector& Center) const;
    
    // 当前搜索点索引
    int32 CurrentPointIndex = 0;
    
    // 搜索点列表
    TArray<FVector> SearchPoints;
    
    // 等待计时器
    float WaitTimer = 0.0f;
    
    // 总搜索时间
    float TotalSearchTime = 0.0f;
    
    // 是否正在等待
    bool bIsWaiting = false;
};
