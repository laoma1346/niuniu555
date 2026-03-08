// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_Investigate.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

// 定义侦查任务日志分类
DEFINE_LOG_CATEGORY(LogAIInvestigate);

UBTTask_Investigate::UBTTask_Investigate()
{
    NodeName = TEXT("Investigate Location");
    bNotifyTick = true;
    
    // 添加Key类型Filter
    InvestigateLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Investigate, InvestigateLocationKey));
}

EBTNodeResult::Type UBTTask_Investigate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    
    if (!AIController || !Blackboard)
    {
        UE_LOG(LogAIInvestigate, Warning, TEXT("AIController or Blackboard is null"));
        return EBTNodeResult::Failed;
    }
    
    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn)
    {
        UE_LOG(LogAIInvestigate, Warning, TEXT("AI Pawn is null"));
        return EBTNodeResult::Failed;
    }
    
    // 获取侦查位置
    FVector TargetLocation = FVector::ZeroVector;
    if (InvestigateLocationKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
    {
        TargetLocation = Blackboard->GetValueAsVector(InvestigateLocationKey.SelectedKeyName);
    }
    else
    {
        UE_LOG(LogAIInvestigate, Warning, TEXT("Investigate location key type error"));
        return EBTNodeResult::Failed;
    }
    
    // 初始化状态
    bHasArrived = false;
    InvestigateTimer = 0.0f;
    CurrentRotation = 0.0f;
    
    // 检查是否已经在侦查位置
    float Distance = FVector::Dist2D(AIPawn->GetActorLocation(), TargetLocation);
    if (Distance <= AcceptanceRadius)
    {
        UE_LOG(LogAIInvestigate, Log, TEXT("Already at investigate location, start investigating"));
        bHasArrived = true;
        AIController->StopMovement();
        return EBTNodeResult::InProgress;
    }
    
    // 移动到侦查位置
    EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
        TargetLocation,
        AcceptanceRadius,
        true,   // bStopOnOverlap
        true,   // bUsePathfinding
        false,  // bProjectDestinationToNavigation
        false,  // bCanStrafe
        TSubclassOf<UNavigationQueryFilter>(),   // FilterClass
        true    // bAllowPartialPath
    );
    
    if (MoveResult == EPathFollowingRequestResult::Type::Failed)
    {
        UE_LOG(LogAIInvestigate, Warning, TEXT("Move request failed"));
        return EBTNodeResult::Failed;
    }
    
    UE_LOG(LogAIInvestigate, Log, TEXT("Start moving to investigate location: %s"), *TargetLocation.ToString());
    
    return EBTNodeResult::InProgress;
}

void UBTTask_Investigate::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    if (!bHasArrived)
    {
        // 检查是否到达侦查位置
        if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle)
        {
            UE_LOG(LogAIInvestigate, Log, TEXT("Arrived at investigate location, investigate for %.1f seconds"), InvestigateTime);
            
            bHasArrived = true;
            AIController->StopMovement();
        }
    }
    else
    {
        // 正在侦查
        InvestigateTimer += DeltaSeconds;
        
        // 侦查时旋转观察
        if (bRotateWhileWaiting)
        {
            CurrentRotation += RotationSpeed * DeltaSeconds;
            if (CurrentRotation >= 360.0f)
            {
                CurrentRotation -= 360.0f;
            }
            
            FRotator NewRotation = AIPawn->GetActorRotation();
            NewRotation.Yaw += RotationSpeed * DeltaSeconds;
            AIPawn->SetActorRotation(NewRotation);
        }
        
        // 检查侦查时间是否结束
        if (InvestigateTimer >= InvestigateTime)
        {
            UE_LOG(LogAIInvestigate, Log, TEXT("Investigation complete"));
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
    }
}

void UBTTask_Investigate::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        // 停止移动
        AIController->StopMovement();
    }
    
    // 重置状态
    bHasArrived = false;
    InvestigateTimer = 0.0f;
    CurrentRotation = 0.0f;
    
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
