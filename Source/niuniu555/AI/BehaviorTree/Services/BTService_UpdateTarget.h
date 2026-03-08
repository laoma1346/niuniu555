// Fill in your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_UpdateTarget.generated.h"

/**
 * 更新目标服务
 * 定期更新目标Actor的位置到黑板，用于追踪移动中的目标
 */
UCLASS()
class NIUNIU555_API UBTService_UpdateTarget : public UBTService_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTService_UpdateTarget();

    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    /** 获取目标Actor的Blackboard键 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "目标更新")
    FBlackboardKeySelector TargetActorKey;

    /** 目标位置的Blackboard键（输出） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "目标更新")
    FBlackboardKeySelector TargetLocationKey;

    /** 是否更新目标方向 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "目标更新")
    bool bUpdateTargetDirection = false;

    /** 目标方向的Blackboard键（输出） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "目标更新", meta = (EditCondition = "bUpdateTargetDirection"))
    FBlackboardKeySelector TargetDirectionKey;

    /** 是否更新与目标的距离 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "目标更新")
    bool bUpdateDistance = true;

    /** 目标距离的Blackboard键（输出） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "目标更新", meta = (EditCondition = "bUpdateDistance"))
    FBlackboardKeySelector TargetDistanceKey;

    /** 使用3D距离计算（否则只计算水平距离） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "目标更新", meta = (EditCondition = "bUpdateDistance"))
    bool bUse3DDistance = false;
};
