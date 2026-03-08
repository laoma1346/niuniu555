// Fill in your copyright notice in the Description page of Project Settings.

#include "BTTask_ChasePlayer.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

// 定义追击任务日志分类
DEFINE_LOG_CATEGORY(LogAIChase);

UBTTask_ChasePlayer::UBTTask_ChasePlayer()
{
    NodeName = TEXT("追击玩家");
    bNotifyTick = true;
    
    // 添加ObjectFilter，确保在蓝图中正确选择Object类型的Key
    TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ChasePlayer, TargetActorKey), AActor::StaticClass());
}

uint16 UBTTask_ChasePlayer::GetInstanceMemorySize() const
{
    return sizeof(FChaseTaskMemory);
}

void UBTTask_ChasePlayer::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
    if (NodeMemory)
    {
        FChaseTaskMemory* ChaseMemory = new (NodeMemory) FChaseTaskMemory();
        
        if (InitType == EBTMemoryInit::Initialize)
        {
            ChaseMemory->RepathTimer = 0.0f;
            ChaseMemory->ChaseStartLocation = FVector::ZeroVector;
            ChaseMemory->LastTargetLocation = FVector::ZeroVector;
            ChaseMemory->NextDistanceCheckTime = 0.0f;
            ChaseMemory->bHasEnteredAttackRange = false;
        }
    }
    
    Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
}

EBTNodeResult::Type UBTTask_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FChaseTaskMemory* ChaseMemory = GetChaseMemory(NodeMemory);
    
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    
    if (!AIController || !Blackboard)
    {
        UE_LOG(LogAIChase, Warning, TEXT("AIController或Blackboard为空"));
        return EBTNodeResult::Failed;
    }
    
    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn)
    {
        UE_LOG(LogAIChase, Warning, TEXT("AI控制的Pawn为空"));
        return EBTNodeResult::Failed;
    }
    
    // 获取目标Actor
    AActor* TargetActor = nullptr;
    
    // 方法1：使用配置KeyName直接获取（推荐方式）
    if (!TargetActorKey.SelectedKeyName.IsNone())
    {
        TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    }
    
    // 方法2：如果方法1失败，尝试用KeyID获取
    if (!TargetActor && TargetActorKey.GetSelectedKeyID() != FBlackboard::InvalidKey)
    {
        TargetActor = Cast<AActor>(Blackboard->GetValue<UBlackboardKeyType_Object>(TargetActorKey.GetSelectedKeyID()));
    }
    
    // 方法3：最后尝试直接查找名为"TargetActor"的Key（后备方案）
    if (!TargetActor)
    {
        FName FallbackKeyName = FName(TEXT("TargetActor"));
        TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(FallbackKeyName));
        if (TargetActor)
        {
            UE_LOG(LogAIChase, Warning, TEXT("使用后备Key成功获取目标: %s"), *TargetActor->GetName());
        }
    }
    
    if (!TargetActor)
    {
        UE_LOG(LogAIChase, Warning, TEXT("目标Actor为空，所有获取方法均失败"));
        return EBTNodeResult::Failed;
    }
    
    UE_LOG(LogAIChase, Log, TEXT("成功获取目标: %s"), *TargetActor->GetName());
    
    // 记录追击起始位置
    if (ChaseStartLocationKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
    {
        FVector SavedLocation = Blackboard->GetValueAsVector(ChaseStartLocationKey.SelectedKeyName);
        if (SavedLocation.IsNearlyZero())
        {
            // 黑板中没有有效位置，使用当前位置并保存
            ChaseMemory->ChaseStartLocation = AIPawn->GetActorLocation();
            Blackboard->SetValueAsVector(ChaseStartLocationKey.SelectedKeyName, ChaseMemory->ChaseStartLocation);
            UE_LOG(LogAIChase, Log, TEXT("设置追击起始位置: %s"), *ChaseMemory->ChaseStartLocation.ToString());
        }
        else
        {
            // 使用黑板中保存的位置
            ChaseMemory->ChaseStartLocation = SavedLocation;
        }
    }
    else
    {
        // 没有配置黑板键，使用内存中存储的位置
        ChaseMemory->ChaseStartLocation = AIPawn->GetActorLocation();
    }
    
    // 立即开始第一次移动
    EPathFollowingRequestResult::Type MoveResult = AIController->MoveToActor(
        TargetActor,
        AcceptanceRadius,
        true,   // bStopOnOverlap
        true,   // bUsePathfinding
        false,  // bCanStrafe
        NULL,   // FilterClass
        true    // bAllowPartialPath
    );
    
    if (MoveResult == EPathFollowingRequestResult::Type::Failed)
    {
        UE_LOG(LogAIChase, Warning, TEXT("初始移动请求失败"));
        return EBTNodeResult::Failed;
    }
    
    ChaseMemory->RepathTimer = 0.0f;
    ChaseMemory->NextDistanceCheckTime = 0.0f;
    ChaseMemory->bHasEnteredAttackRange = false;
    
    // 初始化目标位置记录
    ChaseMemory->LastTargetLocation = TargetActor->GetActorLocation();
    
    UE_LOG(LogAIChase, Log, TEXT("开始追击目标: %s"), *TargetActor->GetName());
    
    return EBTNodeResult::InProgress;
}

void UBTTask_ChasePlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    FChaseTaskMemory* ChaseMemory = GetChaseMemory(NodeMemory);
    
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    
    if (!AIController || !Blackboard)
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
    
    // 获取目标Actor - 使用与ExecuteTask相同的逻辑（3种后备方案）
    AActor* TargetActor = nullptr;
    
    // 方法1：使用配置KeyName直接获取
    if (!TargetActorKey.SelectedKeyName.IsNone())
    {
        TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    }
    
    // 方法2：如果方法1失败，尝试用KeyID获取
    if (!TargetActor && TargetActorKey.GetSelectedKeyID() != FBlackboard::InvalidKey)
    {
        TargetActor = Cast<AActor>(Blackboard->GetValue<UBlackboardKeyType_Object>(TargetActorKey.GetSelectedKeyID()));
    }
    
    // 方法3：后备方案，直接查找"TargetActor"
    if (!TargetActor)
    {
        TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(FName(TEXT("TargetActor"))));
    }
    
    if (!TargetActor)
    {
        UE_LOG(LogAIChase, Warning, TEXT("目标丢失"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    FVector CurrentLocation = AIPawn->GetActorLocation();
    FVector TargetLocation = TargetActor->GetActorLocation();
    
    // 使用平方距离检查，避免开方运算
    float DistSqToTarget = FVector::DistSquared2D(CurrentLocation, TargetLocation);
    float AttackRangeSq = FMath::Square(AttackRange);
    
    // 检查是否进入攻击范围
    if (DistSqToTarget <= AttackRangeSq)
    {
        // 【关键修复】已经进入攻击范围，停止移动并返回成功
        // 不再检查目标是否静止，避免攻击和追击之间的震荡
        
        if (!ChaseMemory->bHasEnteredAttackRange)
        {
            ChaseMemory->bHasEnteredAttackRange = true;
            UE_LOG(LogAIChase, Log, TEXT("进入攻击范围(距离:%.1f)，停止追击，返回成功"), 
                FMath::Sqrt(DistSqToTarget));
        }
        
        // 停止移动
        AIController->StopMovement();
        
        // 返回成功，让攻击任务接管
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }
    else
    {
        // 离开攻击范围，重置标志
        if (ChaseMemory->bHasEnteredAttackRange)
        {
            ChaseMemory->bHasEnteredAttackRange = false;
            UE_LOG(LogAIChase, Log, TEXT("离开攻击范围(距离:%.1f)，继续追击"), 
                FMath::Sqrt(DistSqToTarget));
        }
    }
    
    // 性能优化：降低距离检查频率（最大追击距离检查）
    ChaseMemory->NextDistanceCheckTime -= DeltaSeconds;
    
    if (ChaseMemory->NextDistanceCheckTime <= 0.0f)
    {
        ChaseMemory->NextDistanceCheckTime = DistanceCheckInterval;
        
        // 检查是否超出最大追击距离（使用平方距离）
        if (MaxChaseDistance > 0.0f)
        {
            float DistSqFromStart = FVector::DistSquared2D(CurrentLocation, ChaseMemory->ChaseStartLocation);
            if (DistSqFromStart > FMath::Square(MaxChaseDistance))
            {
                UE_LOG(LogAIChase, Log, TEXT("超出最大追击距离，放弃追击"));
                FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
                return;
            }
        }
    }
    
    // 更新目标位置记录（用于目标移动检测）
    ChaseMemory->LastTargetLocation = TargetLocation;
    
    // 重新寻路计时
    ChaseMemory->RepathTimer += DeltaSeconds;
    if (ChaseMemory->RepathTimer >= RepathInterval)
    {
        ChaseMemory->RepathTimer = 0.0f;
        
        // 更新移动目标位置
        EPathFollowingRequestResult::Type MoveResult = AIController->MoveToActor(
            TargetActor,
            AcceptanceRadius,
            true,   // bStopOnOverlap
            true,   // bUsePathfinding
            false,  // bCanStrafe
            NULL,   // FilterClass
            true    // bAllowPartialPath
        );
        
        if (MoveResult == EPathFollowingRequestResult::Type::Failed)
        {
            UE_LOG(LogAIChase, Warning, TEXT("更新移动目标失败"));
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }
    }
}

void UBTTask_ChasePlayer::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    FChaseTaskMemory* ChaseMemory = GetChaseMemory(NodeMemory);
    
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        // 停止移动
        AIController->StopMovement();
    }
    
    // 清理内存状态
    ChaseMemory->RepathTimer = 0.0f;
    ChaseMemory->ChaseStartLocation = FVector::ZeroVector;
    ChaseMemory->LastTargetLocation = FVector::ZeroVector;
    ChaseMemory->NextDistanceCheckTime = 0.0f;
    ChaseMemory->bHasEnteredAttackRange = false;
    
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

bool UBTTask_ChasePlayer::ShouldAbortChase(AAIController* AIController, APawn* AIPawn, AActor* TargetActor, const FChaseTaskMemory* ChaseMemory) const
{
    if (MaxChaseDistance <= 0.0f)
    {
        return false; // 不限制追击距离
    }
    
    // 计算与追击起始位置的距离（使用平方距离，避免开方）
    float DistSqFromStart = FVector::DistSquared2D(AIPawn->GetActorLocation(), ChaseMemory->ChaseStartLocation);
    
    return DistSqFromStart > FMath::Square(MaxChaseDistance);
}

bool UBTTask_ChasePlayer::IsInAttackRange(APawn* AIPawn, AActor* TargetActor) const
{
    // 使用平方距离（忽略Z轴），与行为树装饰器保持一致
    float DistSqToTarget = FVector::DistSquared2D(AIPawn->GetActorLocation(), TargetActor->GetActorLocation());
    return DistSqToTarget <= FMath::Square(AttackRange);
}
