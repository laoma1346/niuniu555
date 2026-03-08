// Fill in your copyright notice in the Description page of Project Settings.

#include "BTService_UpdateTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
    NodeName = TEXT("更新目标信息");
    
    // 默认间隔为0.5秒
    Interval = 0.5f;
    RandomDeviation = 0.1f;
    
    bUpdateTargetDirection = false;
    bUpdateDistance = true;
    bUse3DDistance = false;
    
    // 添加Key类型Filter
    TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, TargetActorKey), AActor::StaticClass());
    TargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, TargetLocationKey));
    TargetDirectionKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, TargetDirectionKey));
    TargetDistanceKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, TargetDistanceKey));
}

void UBTService_UpdateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return;
    }
    
    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn)
    {
        return;
    }
    
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard)
    {
        return;
    }
    
    // 获取目标Actor
    AActor* TargetActor = nullptr;
    if (TargetActorKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
    {
        TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    }
    
    if (!TargetActor)
    {
        return;
    }
    
    FVector AILocation = AIPawn->GetActorLocation();
    FVector TargetLocation = TargetActor->GetActorLocation();
    
    // 更新目标位置
    if (TargetLocationKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
    {
        Blackboard->SetValueAsVector(TargetLocationKey.SelectedKeyName, TargetLocation);
    }
    
    // 更新目标方向
    if (bUpdateTargetDirection && TargetDirectionKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
    {
        FVector Direction = (TargetLocation - AILocation).GetSafeNormal();
        Blackboard->SetValueAsVector(TargetDirectionKey.SelectedKeyName, Direction);
    }
    
    // 更新距离
    if (bUpdateDistance && TargetDistanceKey.SelectedKeyType == UBlackboardKeyType_Float::StaticClass())
    {
        float Distance;
        if (bUse3DDistance)
        {
            Distance = FVector::Dist(AILocation, TargetLocation);
        }
        else
        {
            // 只计算水平距离
            FVector AIHorizontal = AILocation;
            FVector TargetHorizontal = TargetLocation;
            AIHorizontal.Z = 0.0f;
            TargetHorizontal.Z = 0.0f;
            Distance = FVector::Dist(AIHorizontal, TargetHorizontal);
        }
        
        Blackboard->SetValueAsFloat(TargetDistanceKey.SelectedKeyName, Distance);
    }
}
