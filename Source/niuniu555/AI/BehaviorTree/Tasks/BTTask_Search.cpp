// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_Search.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

// 定义搜索任务日志分类
DEFINE_LOG_CATEGORY(LogAISearch);

UBTTask_Search::UBTTask_Search()
{
    NodeName = TEXT("Search Target");
    bNotifyTick = true;
    
    // 添加Key类型Filter
    SearchCenterKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Search, SearchCenterKey));
}

EBTNodeResult::Type UBTTask_Search::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    
    if (!AIController || !Blackboard)
    {
        UE_LOG(LogAISearch, Warning, TEXT("AIController or Blackboard is null"));
        return EBTNodeResult::Failed;
    }
    
    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn)
    {
        UE_LOG(LogAISearch, Warning, TEXT("AI Pawn is null"));
        return EBTNodeResult::Failed;
    }
    
    // 获取搜索中心位置
    FVector SearchCenter = FVector::ZeroVector;
    if (SearchCenterKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
    {
        SearchCenter = Blackboard->GetValueAsVector(SearchCenterKey.SelectedKeyName);
    }
    else
    {
        UE_LOG(LogAISearch, Warning, TEXT("Search center key type error"));
        return EBTNodeResult::Failed;
    }
    
    // 初始化搜索
    CurrentPointIndex = 0;
    WaitTimer = 0.0f;
    TotalSearchTime = 0.0f;
    bIsWaiting = false;
    SearchPoints = GenerateSearchPoints(SearchCenter);
    
    if (SearchPoints.Num() == 0)
    {
        UE_LOG(LogAISearch, Warning, TEXT("Cannot generate valid search points"));
        return EBTNodeResult::Failed;
    }
    
    UE_LOG(LogAISearch, Log, TEXT("Start searching, center: %s, generated %d points"), 
        *SearchCenter.ToString(), SearchPoints.Num());
    
    // 移动到第一个搜索点
    FVector FirstPoint = SearchPoints[0];
    AIController->MoveToLocation(FirstPoint, AcceptanceRadius, true, true, false, false, NULL, true);
    
    return EBTNodeResult::InProgress;
}

void UBTTask_Search::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
    
    // 更新总搜索时间
    TotalSearchTime += DeltaSeconds;
    
    // 检查是否超时
    if (TotalSearchTime >= SearchDuration)
    {
        UE_LOG(LogAISearch, Log, TEXT("Search time ended, target not found"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }
    
    if (bIsWaiting)
    {
        // 等待计时
        WaitTimer += DeltaSeconds;
        if (WaitTimer >= WaitTimeAtPoint)
        {
            // 等待结束，移动到下一个搜索点
            CurrentPointIndex++;
            if (CurrentPointIndex >= SearchPoints.Num())
            {
                // 所有搜索点都已访问
                UE_LOG(LogAISearch, Log, TEXT("Completed all search points"));
                FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
                return;
            }
            
            // 移动到下一个搜索点
            FVector NextPoint = SearchPoints[CurrentPointIndex];
            AIController->MoveToLocation(NextPoint, AcceptanceRadius, true, true, false, false, NULL, true);
            
            UE_LOG(LogAISearch, Log, TEXT("Move to search point %d/%d"), CurrentPointIndex + 1, SearchPoints.Num());
            
            bIsWaiting = false;
            WaitTimer = 0.0f;
        }
    }
    else
    {
        // 检查是否到达当前搜索点
        if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle)
        {
            UE_LOG(LogAISearch, Log, TEXT("Arrived at search point %d/%d, wait %.1f seconds"), 
                CurrentPointIndex + 1, SearchPoints.Num(), WaitTimeAtPoint);
            
            // 到达搜索点，开始等待
            bIsWaiting = true;
            WaitTimer = 0.0f;
        }
    }
}

void UBTTask_Search::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        // 停止移动
        AIController->StopMovement();
    }
    
    // 重置状态
    CurrentPointIndex = 0;
    WaitTimer = 0.0f;
    TotalSearchTime = 0.0f;
    bIsWaiting = false;
    SearchPoints.Empty();
    
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

TArray<FVector> UBTTask_Search::GenerateSearchPoints(const FVector& Center) const
{
    TArray<FVector> Points;
    
    UWorld* World = nullptr;
    if (AActor* OuterActor = Cast<AActor>(GetOuter()))
    {
        World = OuterActor->GetWorld();
    }
    
    UNavigationSystemV1* NavSystem = nullptr;
    if (World)
    {
        NavSystem = UNavigationSystemV1::GetCurrent(World);
    }
    
    // 生成扇形分布的搜索点
    for (int32 i = 0; i < NumSearchPoints; ++i)
    {
        float Angle = (360.0f / NumSearchPoints) * i;
        float Radian = FMath::DegreesToRadians(Angle);
        
        FVector Offset(
            FMath::Cos(Radian) * SearchRadius,
            FMath::Sin(Radian) * SearchRadius,
            0.0f
        );
        
        FVector CandidateLocation = Center + Offset;
        
        // 尝试投影到导航网格
        if (NavSystem)
        {
            FNavLocation ProjectedLocation;
            if (NavSystem->ProjectPointToNavigation(CandidateLocation, ProjectedLocation, FVector(500.0f, 500.0f, 500.0f)))
            {
                Points.Add(ProjectedLocation.Location);
            }
        }
        else
        {
            Points.Add(CandidateLocation);
        }
    }
    
    return Points;
}
