// Fill in your copyright notice in the Description page of Project Settings.

#include "BTDecorator_DistanceCheck.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

UBTDecorator_DistanceCheck::UBTDecorator_DistanceCheck()
{
    NodeName = TEXT("距离检查");
    TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_DistanceCheck, TargetActorKey), AActor::StaticClass());
}

bool UBTDecorator_DistanceCheck::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return false;
    }
    
    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn)
    {
        return false;
    }
    
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard)
    {
        return false;
    }
    
    // 获取目标Actor
    AActor* TargetActor = nullptr;
    
    // 方法1：使用配置的TargetActorKey
    if (TargetActorKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
    {
        TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    }
    
    // 方法2：如果方法1失败，尝试直接查找名为"TargetActor"的Key（后备方案）
    if (!TargetActor)
    {
        FName FallbackKeyName = FName(TEXT("TargetActor"));
        TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(FallbackKeyName));
    }
    
    if (!TargetActor)
    {
        return false;
    }
    
    // 计算距离 - 与攻击任务保持一致，使用平方距离
    FVector AILocation = AIPawn->GetActorLocation();
    FVector TargetLocation = TargetActor->GetActorLocation();
    
    float DistanceSq;
    if (bUse3DDistance)
    {
        DistanceSq = FVector::DistSquared(AILocation, TargetLocation);
    }
    else
    {
        // 使用 DistSquared2D 计算水平距离（与任务类保持一致）
        DistanceSq = FVector::DistSquared2D(AILocation, TargetLocation);
    }
    
    // 根据模式返回结果
    switch (DistanceMode)
    {
    case EDistanceCheckMode::LessThan:
        return DistanceSq < FMath::Square(CheckDistance);
        
    case EDistanceCheckMode::GreaterThan:
        return DistanceSq > FMath::Square(CheckDistance);
        
    case EDistanceCheckMode::WithinRange:
        return DistanceSq >= FMath::Square(MinDistance) && DistanceSq <= FMath::Square(MaxDistance);
        
    case EDistanceCheckMode::OutsideRange:
        return DistanceSq < FMath::Square(MinDistance) || DistanceSq > FMath::Square(MaxDistance);
        
    default:
        return false;
    }
}

FString UBTDecorator_DistanceCheck::GetStaticDescription() const
{
    FString ModeStr;
    switch (DistanceMode)
    {
    case EDistanceCheckMode::LessThan:
        ModeStr = FString::Printf(TEXT("距离 < %.1f"), CheckDistance);
        break;
    case EDistanceCheckMode::GreaterThan:
        ModeStr = FString::Printf(TEXT("距离 > %.1f"), CheckDistance);
        break;
    case EDistanceCheckMode::WithinRange:
        ModeStr = FString::Printf(TEXT("%.1f <= 距离 <= %.1f"), MinDistance, MaxDistance);
        break;
    case EDistanceCheckMode::OutsideRange:
        ModeStr = FString::Printf(TEXT("距离 < %.1f 或 距离 > %.1f"), MinDistance, MaxDistance);
        break;
    }
    
    return FString::Printf(TEXT("%s: %s (%s)"), 
        *Super::GetStaticDescription(),
        *ModeStr,
        bUse3DDistance ? TEXT("3D") : TEXT("2D"));
}
