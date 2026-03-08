// Fill in your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_DistanceCheck.generated.h"

/** 距离比较模式 */
UENUM(BlueprintType)
enum class EDistanceCheckMode : uint8
{
    LessThan,           // 小于指定距离
    GreaterThan,        // 大于指定距离
    WithinRange,        // 在指定范围内（MinDistance ~ MaxDistance）
    OutsideRange        // 在指定范围外
};

/**
 * 距离检查装饰器
 * 检查AI与目标Actor的距离是否在指定范围内
 */
UCLASS()
class NIUNIU555_API UBTDecorator_DistanceCheck : public UBTDecorator_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTDecorator_DistanceCheck();

    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
    virtual FString GetStaticDescription() const override;

public:
    /** 目标Actor的Blackboard键 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "距离检查")
    FBlackboardKeySelector TargetActorKey;

    /** 距离比较模式 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "距离检查")
    EDistanceCheckMode DistanceMode = EDistanceCheckMode::LessThan;

    /** 检查距离 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "距离检查", meta = (ClampMin = "0.0"))
    float CheckDistance = 1000.0f;

    /** 最小距离（仅WithinRange/OutsideRange模式使用） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "距离检查", meta = (ClampMin = "0.0", EditCondition = "DistanceMode == EDistanceCheckMode::WithinRange || DistanceMode == EDistanceCheckMode::OutsideRange"))
    float MinDistance = 0.0f;

    /** 最大距离（仅WithinRange/OutsideRange模式使用） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "距离检查", meta = (ClampMin = "0.0", EditCondition = "DistanceMode == EDistanceCheckMode::WithinRange || DistanceMode == EDistanceCheckMode::OutsideRange"))
    float MaxDistance = 1000.0f;

    /** 是否使用3D距离（否则只计算水平距离） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "距离检查")
    bool bUse3DDistance = false;
};
