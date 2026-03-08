// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/EnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemySystem/EnemyBase.h"
#include "EnemySystem/EnemyDataAsset.h"

// 定义EnemyAIController日志类别
DEFINE_LOG_CATEGORY(LogEnemyAI);

// 静态定义黑板Key名称
const FName AEnemyAIController::TargetActorKey = FName(TEXT("TargetActor"));
const FName AEnemyAIController::HomeLocationKey = FName(TEXT("HomeLocation"));
const FName AEnemyAIController::PatrolRadiusKey = FName(TEXT("PatrolRadius"));
const FName AEnemyAIController::PatrolCenterKey = FName(TEXT("PatrolCenter"));
const FName AEnemyAIController::ChaseStartLocationKey = FName(TEXT("ChaseStartLocation"));
const FName AEnemyAIController::AIStateKey = FName(TEXT("AIState"));
const FName AEnemyAIController::LastKnownLocationKey = FName(TEXT("LastKnownLocation"));
const FName AEnemyAIController::InvestigateLocationKey = FName(TEXT("InvestigateLocation"));
const FName AEnemyAIController::SenseTypeKey = FName(TEXT("SenseType"));

AEnemyAIController::AEnemyAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bInitialized = false;
    CurrentTarget = nullptr;
    ControlledEnemy = nullptr;
    CurrentState = EAIBehaviorState::None;
    TargetLostTimer = 0.0f;
    TargetActorTag = FName(TEXT("Player"));

    // 创建感知组件
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // 创建视觉感知配置
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 3000.0f;
    SightConfig->LoseSightRadius = 3500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 200.0f;
    SightConfig->SetMaxAge(2.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 1000.0f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    // 创建听觉感知配置
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

    // 创建伤害感知配置
    DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));

    // 配置感知组件
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());

    SetPerceptionComponent(*AIPerceptionComponent);
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ControlledEnemy = Cast<AEnemyBase>(InPawn);
    if (!ControlledEnemy.IsValid())
    {
        UE_LOG(LogEnemyAI, Warning, TEXT("[EnemyAIController] 控制的Pawn不是AEnemyBase类型"));
        return;
    }

    LoadAIConfigFromDataAsset();
    StartBehaviorTree();
    InitializeBlackboardKeys();

    // 绑定感知回调
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetPerceptionUpdated);
        AIPerceptionComponent->Activate();
    }

    // 初始状态为待机
    SetState(EAIBehaviorState::Idle);

    bInitialized = true;

    UE_LOG(LogEnemyAI, Log, TEXT("[EnemyAIController] %s 初始化完成"), *GetName());
}

void AEnemyAIController::OnUnPossess()
{
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(this, &AEnemyAIController::OnTargetPerceptionUpdated);
    }

    Super::OnUnPossess();
}

void AEnemyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 完全依赖感知系统的回调机制
    // 目标丢失通过感知系统的 Stimulus.WasSuccessfullySensed() = false 回调处理
    // SightConfig->SetMaxAge(2.0f) 确保2秒后自动触发遗忘回调
    // 不再每帧检查 CanSeeTarget，避免性能开销
}

void AEnemyAIController::LoadAIConfigFromDataAsset()
{
    if (!ControlledEnemy.IsValid())
    {
        return;
    }

    UEnemyDataAsset* EnemyData = ControlledEnemy->GetEnemyData();
    if (!EnemyData)
    {
        UE_LOG(LogEnemyAI, Warning, TEXT("[EnemyAIController] %s 没有配置EnemyData"), *GetName());
        return;
    }

    // 从数据资产加载行为树
    if (EnemyData->BehaviorTree != nullptr)
    {
        BehaviorTree = EnemyData->BehaviorTree;
    }

    UE_LOG(LogEnemyAI, Log, TEXT("[EnemyAIController] %s 从数据资产加载配置"), *GetName());
}

void AEnemyAIController::StartBehaviorTree()
{
    UBehaviorTree* BT = BehaviorTree.LoadSynchronous();
    if (BT)
    {
        RunBehaviorTree(BT);
        UE_LOG(LogEnemyAI, Log, TEXT("[EnemyAIController] %s 开始运行行为树: %s"), 
            *GetName(), *BT->GetName());
    }
    else
    {
        UE_LOG(LogEnemyAI, Warning, TEXT("[EnemyAIController] %s 没有配置行为树"), *GetName());
    }
}

void AEnemyAIController::InitializeBlackboardKeys()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB)
    {
        return;
    }

    // 初始化HomeLocation
    if (APawn* MyPawn = GetPawn())
    {
        BB->SetValueAsVector(HomeLocationKey, MyPawn->GetActorLocation());
        BB->SetValueAsVector(PatrolCenterKey, MyPawn->GetActorLocation());
    }

    // 初始化状态
    BB->SetValueAsEnum(AIStateKey, static_cast<uint8>(CurrentState));

    UE_LOG(LogEnemyAI, Log, TEXT("[EnemyAIController] %s 黑板初始化完成"), *GetName());
}

void AEnemyAIController::SetTarget(AActor* NewTarget)
{
    if (CurrentTarget != NewTarget)
    {
        AActor* OldTarget = CurrentTarget.Get();
        CurrentTarget = NewTarget;

        UBlackboardComponent* BB = GetBlackboardComponent();
        if (BB)
        {
            if (NewTarget)
            {
                BB->SetValueAsObject(TargetActorKey, NewTarget);
                BB->SetValueAsVector(ChaseStartLocationKey, GetPawn()->GetActorLocation());
                LastKnownLocation = NewTarget->GetActorLocation();

                UE_LOG(LogEnemyAI, Log, TEXT("[EnemyAIController] 设置目标: %s"), *NewTarget->GetName());
            }
            else
            {
                BB->ClearValue(TargetActorKey);
                // 不清除ChaseStartLocation，用于计算最大追击距离
            }
        }

        OnTargetChanged.Broadcast(OldTarget, NewTarget);
    }
}

void AEnemyAIController::ClearTarget()
{
    if (CurrentTarget.IsValid())
    {
        SetTarget(nullptr);
    }
}

void AEnemyAIController::SetState(EAIBehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        EAIBehaviorState OldState = CurrentState;
        CurrentState = NewState;
        PreviousState = OldState;

        UBlackboardComponent* BB = GetBlackboardComponent();
        if (BB)
        {
            BB->SetValueAsEnum(AIStateKey, static_cast<uint8>(NewState));
        }

        OnStateChanged.Broadcast(OldState, NewState);

        UE_LOG(LogEnemyAI, Log, TEXT("[EnemyAIController] 状态切换: %s -> %s"),
            *UEnum::GetValueAsString(OldState),
            *UEnum::GetValueAsString(NewState));
    }
}

void AEnemyAIController::StartIdleState()
{
    SetState(EAIBehaviorState::Idle);
}

void AEnemyAIController::StartPatrolState()
{
    SetState(EAIBehaviorState::Patrol);
}

void AEnemyAIController::StartInvestigateState(FVector Location, EAISenseType SenseType)
{
    SetState(EAIBehaviorState::Investigate);
    InvestigateLocation = Location;
    LastSenseType = SenseType;

    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsVector(InvestigateLocationKey, Location);
        BB->SetValueAsEnum(SenseTypeKey, static_cast<uint8>(SenseType));
    }

    UE_LOG(LogEnemyAI, Log, TEXT("[EnemyAIController] 开始侦查状态，位置: %s，感知类型: %s"), 
            *Location.ToString(), *UEnum::GetValueAsString(SenseType));
}

void AEnemyAIController::StartChaseState()
{
    SetState(EAIBehaviorState::Chase);
}

void AEnemyAIController::StartAttackState()
{
    SetState(EAIBehaviorState::Attack);
}

void AEnemyAIController::StartSearchState()
{
    SetState(EAIBehaviorState::Search);

    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsVector(LastKnownLocationKey, LastKnownLocation);
    }

    UE_LOG(LogEnemyAI, Log, TEXT("[EnemyAIController] 开始搜索状态，最后已知位置: %s"), *LastKnownLocation.ToString());
}

void AEnemyAIController::StartReturnState()
{
    SetState(EAIBehaviorState::Return);
}

void AEnemyAIController::StartDeadState()
{
    SetState(EAIBehaviorState::Dead);
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || !Actor->ActorHasTag(TargetActorTag))
    {
        return;
    }

    // 根据感知类型处理
    if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
    {
        HandleSightPerception(Actor, Stimulus);
    }
    else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
    {
        HandleHearingPerception(Actor, Stimulus);
    }
    else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
    {
        HandleDamagePerception(Actor, Stimulus);
    }
}

void AEnemyAIController::HandleSightPerception(AActor* Actor, const FAIStimulus& Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        // 看到目标
        UE_LOG(LogEnemyAI, Log, TEXT("[EnemyAIController] 看到目标: %s"), *Actor->GetName());
        
        if (CurrentState == EAIBehaviorState::Idle ||
            CurrentState == EAIBehaviorState::Patrol ||
            CurrentState == EAIBehaviorState::Search ||
            CurrentState == EAIBehaviorState::Investigate)
        {
            SetTarget(Actor);
            StartChaseState();
        }
        // 如果在追击或攻击状态，更新最后已知位置
        else if (CurrentState == EAIBehaviorState::Chase || CurrentState == EAIBehaviorState::Attack)
        {
            LastKnownLocation = Actor->GetActorLocation();
        }
    }
    else
    {
        // 【关键】感知系统遗忘回调 - 目标丢失
        // 这是 SetMaxAge 超时后自动触发的回调，替代了Tick中的手动检查
        UE_LOG(LogEnemyAI, Log, TEXT("[EnemyAIController] 目标丢失（感知过期）: %s"), *Actor->GetName());
        
        if (CurrentTarget == Actor)
        {
            LastKnownLocation = Actor->GetActorLocation();
            ClearTarget();
            // 切换到搜索状态
            StartSearchState();
        }
    }
}

void AEnemyAIController::HandleHearingPerception(AActor* Actor, const FAIStimulus& Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        // 听到声音，前往侦查
        if (CurrentState == EAIBehaviorState::Idle ||
            CurrentState == EAIBehaviorState::Patrol)
        {
            StartInvestigateState(Stimulus.StimulusLocation, EAISenseType::Hearing);
        }
    }
}

void AEnemyAIController::HandleDamagePerception(AActor* Actor, const FAIStimulus& Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        // 受到伤害，立即反击
        SetTarget(Actor);
        StartChaseState();
    }
}

void AEnemyAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    // 感知更新时的处理
    // 这个函数通常不需要额外实现，因为我们在OnTargetPerceptionUpdated中处理具体逻辑
}

bool AEnemyAIController::CanSeeTarget(AActor* Target) const
{
    if (!Target || !AIPerceptionComponent)
    {
        return false;
    }

    FActorPerceptionBlueprintInfo PerceptionInfo;
    AIPerceptionComponent->GetActorsPerception(Target, PerceptionInfo);

    for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
    {
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() &&
            Stimulus.WasSuccessfullySensed())
        {
            return true;
        }
    }

    return false;
}
