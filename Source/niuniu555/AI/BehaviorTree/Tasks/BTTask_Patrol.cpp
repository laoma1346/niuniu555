// Fill in your copyright notice in the Description page of Project Settings.

#include "BTTask_Patrol.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"

// 定义巡逻任务日志分类
DEFINE_LOG_CATEGORY(LogAIPatrol);

UBTTask_Patrol::UBTTask_Patrol()
{
    NodeName = TEXT("巡逻");
    bNotifyTick = true;
    
    // 添加Key类型Filter，确保在蓝图中正确选择
    PatrolCenterKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Patrol, PatrolCenterKey));
    PatrolRadiusKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Patrol, PatrolRadiusKey));
}

uint16 UBTTask_Patrol::GetInstanceMemorySize() const
{
    return sizeof(FPatrolTaskMemory);
}

void UBTTask_Patrol::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
    if (NodeMemory)
    {
        FPatrolTaskMemory* PatrolMemory = new (NodeMemory) FPatrolTaskMemory();
        
        if (InitType == EBTMemoryInit::Initialize)
        {
            PatrolMemory->WaitTimer = 0.0f;
            PatrolMemory->bIsWaiting = false;
            PatrolMemory->bIsMoving = false;
        }
    }
    
    Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
}

FPatrolTaskMemory* UBTTask_Patrol::GetPatrolMemory(uint8* NodeMemory)
{
    return reinterpret_cast<FPatrolTaskMemory*>(NodeMemory);
}

EBTNodeResult::Type UBTTask_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FPatrolTaskMemory* PatrolMemory = GetPatrolMemory(NodeMemory);
    
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    
    if (!AIController || !Blackboard)
    {
        UE_LOG(LogAIPatrol, Warning, TEXT("AIController或Blackboard为空"));
        return EBTNodeResult::Failed;
    }
    
    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn)
    {
        UE_LOG(LogAIPatrol, Warning, TEXT("AI控制的Pawn为空"));
        return EBTNodeResult::Failed;
    }
    
    // 获取巡逻中心和半径
    FVector PatrolCenter;
    if (PatrolCenterKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
    {
        PatrolCenter = Blackboard->GetValueAsVector(PatrolCenterKey.SelectedKeyName);
    }
    else
    {
        // 默认使用AI当前位置作为中心
        PatrolCenter = AIPawn->GetActorLocation();
    }
    
    float PatrolRadius = 500.0f;
    if (PatrolRadiusKey.SelectedKeyType == UBlackboardKeyType_Float::StaticClass())
    {
        PatrolRadius = Blackboard->GetValueAsFloat(PatrolRadiusKey.SelectedKeyName);
    }
    
    // 在巡逻半径内生成随机目标点
    FVector TargetLocation = GetRandomPatrolPoint(PatrolCenter, PatrolRadius, AIPawn);
    
    UE_LOG(LogAIPatrol, Log, TEXT("从 %s 移动到 %s (半径: %.1f)"), 
        *AIPawn->GetActorLocation().ToString(), *TargetLocation.ToString(), PatrolRadius);
    
    // 使用MoveToLocation移动
    EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
        TargetLocation,
        AcceptanceRadius,
        true,  // bStopOnOverlap
        true,  // bUsePathfinding
        false, // bProjectDestinationToNavigation
        false, // bCanStrafe
        NULL,  // FilterClass
        true   // bAllowPartialPaths
    );
    
    if (MoveResult == EPathFollowingRequestResult::Type::Failed)
    {
        UE_LOG(LogAIPatrol, Warning, TEXT("移动请求失败"));
        return EBTNodeResult::Failed;
    }
    
    PatrolMemory->bIsMoving = true;
    PatrolMemory->bIsWaiting = false;
    PatrolMemory->WaitTimer = 0.0f;
    
    return EBTNodeResult::InProgress;
}

void UBTTask_Patrol::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    FPatrolTaskMemory* PatrolMemory = GetPatrolMemory(NodeMemory);
    
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    if (PatrolMemory->bIsWaiting)
    {
        // 等待计时
        PatrolMemory->WaitTimer += DeltaSeconds;
        if (PatrolMemory->WaitTimer >= WaitTimeAtPoint)
        {
            UE_LOG(LogAIPatrol, Log, TEXT("等待完成，继续下一次巡逻"));
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
    }
    else if (PatrolMemory->bIsMoving)
    {
        // 检查是否到达目标
        if (AIController->GetMoveStatus() == EPathFollowingStatus::Type::Idle)
        {
            // 到达目标，开始等待
            PatrolMemory->bIsMoving = false;
            PatrolMemory->bIsWaiting = true;
            PatrolMemory->WaitTimer = 0.0f;
            UE_LOG(LogAIPatrol, Log, TEXT("到达巡逻点，等待 %.1f 秒"), WaitTimeAtPoint);
        }
    }
}

void UBTTask_Patrol::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    FPatrolTaskMemory* PatrolMemory = GetPatrolMemory(NodeMemory);
    
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController && PatrolMemory->bIsMoving)
    {
        // 停止移动
        AIController->StopMovement();
    }
    
    // 清理内存状态
    PatrolMemory->bIsMoving = false;
    PatrolMemory->bIsWaiting = false;
    PatrolMemory->WaitTimer = 0.0f;
    
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

FVector UBTTask_Patrol::GetRandomPatrolPoint(const FVector& Center, float Radius, APawn* AIPawn)
{
    UWorld* World = AIPawn ? AIPawn->GetWorld() : nullptr;
    if (!World)
    {
        // 如果没有世界上下文，返回中心点附近随机位置
        FVector2D RandomPoint2D = FMath::RandPointInCircle(Radius);
        return Center + FVector(RandomPoint2D.X, RandomPoint2D.Y, 0.0f);
    }
    
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
    if (!NavSystem)
    {
        UE_LOG(LogAIPatrol, Warning, TEXT("Navigation System not found! Ensure NavMesh is built."));
        // 没有导航系统，返回简单随机点
        FVector2D RandomPoint2D = FMath::RandPointInCircle(Radius);
        return Center + FVector(RandomPoint2D.X, RandomPoint2D.Y, 0.0f);
    }
    
    // 尝试在导航网格上找到有效点
    for (int32 Attempt = 0; Attempt < 10; ++Attempt)
    {
        FVector2D RandomPoint2D = FMath::RandPointInCircle(Radius);
        FVector CandidateLocation = Center + FVector(RandomPoint2D.X, RandomPoint2D.Y, 0.0f);
        
        FNavLocation ProjectedLocation;
        if (NavSystem->ProjectPointToNavigation(CandidateLocation, ProjectedLocation, FVector(500.0f, 500.0f, 500.0f)))
        {
            return ProjectedLocation.Location;
        }
    }
    
    // 如果找不到导航点，尝试将中心点投影到导航网格
    UE_LOG(LogAIPatrol, Warning, TEXT("无法在导航网格上找到有效巡逻点，尝试投影中心点"));
    FNavLocation CenterLocation;
    if (NavSystem->ProjectPointToNavigation(Center, CenterLocation, FVector(1000.0f, 1000.0f, 1000.0f)))
    {
        return CenterLocation.Location;
    }
    
    // 后备：返回原始中心点（可能移动会失败，但至少有返回值）
    UE_LOG(LogAIPatrol, Warning, TEXT("中心点也无法投影到导航网格，返回原始中心点"));
    return Center;
}
