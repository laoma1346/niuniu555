// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyAIController.h"
#include "EnemyBase.h"
#include "EnemyDataAsset.h"
#include "EnemyStateMachineComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"

// 静态常量定义
const FName AEnemyAIController::TargetActorKey = FName("TargetActor");
const FName AEnemyAIController::HomeLocationKey = FName("HomeLocation");
const FName AEnemyAIController::PatrolCenterKey = FName("PatrolCenter");
const FName AEnemyAIController::PatrolRadiusKey = FName("PatrolRadius");
const FName AEnemyAIController::AttackRangeKey = FName("AttackRange");
const FName AEnemyAIController::ChaseRangeKey = FName("ChaseRange");
const FName AEnemyAIController::CanAttackKey = FName("CanAttack");
const FName AEnemyAIController::IsInCombatKey = FName("IsInCombat");

AEnemyAIController::AEnemyAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 【修复】显式创建感知组件，确保正确注册到AI系统
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // 创建视觉感知配置
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 3000.0f;          // 视觉半径（使用数据资产中的值）
    SightConfig->LoseSightRadius = 3500.0f;      // 失去视觉半径
    SightConfig->PeripheralVisionAngleDegrees = 200.0f;  // 视野角度（广角）
    SightConfig->SetMaxAge(5.0f);                // 记忆时间
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;  // 测试时全开
    
    // 配置到组件
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());
    
    // 【关键】设置为BrainComponent，确保AI系统识别
    SetPerceptionComponent(*AIPerceptionComponent);
    
    UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] 构造函数：感知组件创建完成"));
    
    // 默认不运行行为树（等待配置）
    bInitialized = false;
}

void AEnemyAIController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] %s BeginPlay"), *GetName());
    
    // 【修复】绑定感知更新回调并激活组件
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnPerceptionUpdated);
        AIPerceptionComponent->Activate();
        UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] 感知组件绑定成功，状态：%s"), 
            AIPerceptionComponent->IsActive() ? TEXT("Active") : TEXT("Inactive"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[EnemyAIController] 感知组件为NULL！"));
    }
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] %s OnPossess -> %s"), *GetName(), *InPawn->GetName());
    
    // 获取控制的敌人
    ControlledEnemy = Cast<AEnemyBase>(InPawn);
    if (!ControlledEnemy)
    {
        UE_LOG(LogTemp, Error, TEXT("[EnemyAIController] %s 控制的Pawn不是AEnemyBase！"), *GetName());
        return;
    }
    
    // 【修复】1. 先加载配置
    LoadAIConfigFromDataAsset();
    
    // 【修复】2. 启动行为树（这会创建Blackboard）
    StartBehaviorTree();
    
    // 【修复】3. 现在Blackboard已存在，初始化Key
    InitializeBlackboardKeys();
    
    bInitialized = true;
}

void AEnemyAIController::OnUnPossess()
{
    // 停止行为树
    StopBehaviorTree();
    
    // 清除引用
    ControlledEnemy = nullptr;
    bInitialized = false;
    
    Super::OnUnPossess();
}

void AEnemyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 更新黑板状态
    if (Blackboard && ControlledEnemy)
    {
        // 更新是否可以攻击
        if (UEnemyStateMachineComponent* StateMachine = ControlledEnemy->GetStateMachineComponent())
        {
            Blackboard->SetValueAsBool(CanAttackKey, StateMachine->CanAttack());
            Blackboard->SetValueAsBool(IsInCombatKey, StateMachine->IsInCombatState());
        }
    }
}

void AEnemyAIController::LoadAIConfigFromDataAsset()
{
    if (!ControlledEnemy)
    {
        return;
    }
    
    UEnemyDataAsset* EnemyData = ControlledEnemy->GetEnemyData();
    if (!EnemyData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EnemyAIController] %s 没有EnemyData，使用默认配置"), *GetName());
        return;
    }
    
    // 从数据资产加载行为树（如果未手动设置）
    if (!BehaviorTree && EnemyData->BehaviorTree)
    {
        BehaviorTree = EnemyData->BehaviorTree;
        UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] %s 从EnemyData加载BehaviorTree"), *GetName());
    }
    
    // 从数据资产加载黑板（如果未手动设置）
    if (!BlackboardAsset && EnemyData->BlackboardData)
    {
        BlackboardAsset = EnemyData->BlackboardData;
        UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] %s 从EnemyData加载Blackboard"), *GetName());
    }
    
    UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] %s AI配置加载完成"), *GetName());
}

void AEnemyAIController::InitializeBlackboardKeys()
{
    UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] InitializeBlackboardKeys 开始"));
    
    // 【修复】防御性检查：尝试从BrainComponent获取Blackboard
    if (!Blackboard)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EnemyAIController] Blackboard为NULL，尝试从BrainComponent获取..."));
        
        if (BrainComponent)
        {
            Blackboard = BrainComponent->GetBlackboardComponent();
        }
        
        if (!Blackboard)
        {
            UE_LOG(LogTemp, Error, TEXT("[EnemyAIController] 无法获取Blackboard，跳过初始化"));
            return;
        }
        
        UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] 成功从BrainComponent获取Blackboard"));
    }
    
    if (!ControlledEnemy)
    {
        UE_LOG(LogTemp, Error, TEXT("[EnemyAIController] ControlledEnemy为NULL！"));
        return;
    }
    
    // 设置家位置（出生点）
    FVector HomeLoc = ControlledEnemy->GetSpawnLocation();
    UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] 获取SpawnLocation: %s"), *HomeLoc.ToString());
    
    // 【修复】确保HomeLocation有效（如果为0，使用当前位置）
    if (HomeLoc.IsNearlyZero())
    {
        HomeLoc = ControlledEnemy->GetActorLocation();
        UE_LOG(LogTemp, Warning, TEXT("[EnemyAIController] SpawnLocation为零，使用当前位置: %s"), *HomeLoc.ToString());
    }
    
    Blackboard->SetValueAsVector(HomeLocationKey, HomeLoc);
    Blackboard->SetValueAsVector(PatrolCenterKey, HomeLoc);
    
    UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] 设置HomeLocation: %s, PatrolCenter: %s"), 
        *HomeLoc.ToString(), *HomeLoc.ToString());
    
    // 从数据资产加载数值
    if (UEnemyDataAsset* EnemyData = ControlledEnemy->GetEnemyData())
    {
        Blackboard->SetValueAsFloat(PatrolRadiusKey, EnemyData->PatrolRadius);
        Blackboard->SetValueAsFloat(AttackRangeKey, EnemyData->AttackRange);
        Blackboard->SetValueAsFloat(ChaseRangeKey, EnemyData->ChaseRadius);
        UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] 从EnemyData加载: PatrolRadius=%.0f, AttackRange=%.0f, ChaseRadius=%.0f"),
            EnemyData->PatrolRadius, EnemyData->AttackRange, EnemyData->ChaseRadius);
    }
    else
    {
        // 使用默认值
        Blackboard->SetValueAsFloat(PatrolRadiusKey, 500.0f);
        Blackboard->SetValueAsFloat(AttackRangeKey, 150.0f);
        Blackboard->SetValueAsFloat(ChaseRangeKey, 2000.0f);
        UE_LOG(LogTemp, Warning, TEXT("[EnemyAIController] 使用默认半径值"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] %s 黑板Key初始化完成"), *GetName());
}

void AEnemyAIController::StartBehaviorTree()
{
    if (!BehaviorTree)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EnemyAIController] %s 没有配置BehaviorTree！"), *GetName());
        return;
    }
    
    // 运行行为树
    RunBehaviorTree(BehaviorTree);
    UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] %s 行为树启动"), *GetName());
}

void AEnemyAIController::StopBehaviorTree()
{
    // 停止行为树
    if (BrainComponent)
    {
        BrainComponent->StopLogic(TEXT("AIController Stop"));
        UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] %s 行为树停止"), *GetName());
    }
}

void AEnemyAIController::RestartBehaviorTree()
{
    StopBehaviorTree();
    StartBehaviorTree();
    UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] %s 行为树重启"), *GetName());
}

bool AEnemyAIController::IsBehaviorTreeRunning() const
{
    return BrainComponent && BrainComponent->IsRunning();
}

AActor* AEnemyAIController::GetTargetActor() const
{
    if (Blackboard)
    {
        return Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey));
    }
    return nullptr;
}

void AEnemyAIController::SetTargetActor(AActor* NewTarget)
{
    if (Blackboard)
    {
        Blackboard->SetValueAsObject(TargetActorKey, NewTarget);
        OnTargetChanged(NewTarget);
    }
    
    // 同时设置到EnemyBase
    if (ControlledEnemy)
    {
        ControlledEnemy->SetTarget(NewTarget);
    }
}

void AEnemyAIController::ClearTargetActor()
{
    SetTargetActor(nullptr);
}

FVector AEnemyAIController::GetHomeLocation() const
{
    if (Blackboard)
    {
        return Blackboard->GetValueAsVector(HomeLocationKey);
    }
    return FVector::ZeroVector;
}

bool AEnemyAIController::HasValidTarget() const
{
    return GetTargetActor() != nullptr;
}

bool AEnemyAIController::IsTargetInAttackRange() const
{
    AActor* Target = GetTargetActor();
    if (!Target || !ControlledEnemy)
    {
        return false;
    }
    
    float AttackRange = Blackboard ? Blackboard->GetValueAsFloat(AttackRangeKey) : 150.0f;
    float Distance = FVector::Distance(ControlledEnemy->GetActorLocation(), Target->GetActorLocation());
    
    return Distance <= AttackRange;
}

bool AEnemyAIController::IsTargetInChaseRange() const
{
    AActor* Target = GetTargetActor();
    if (!Target || !ControlledEnemy)
    {
        return false;
    }
    
    float ChaseRange = Blackboard ? Blackboard->GetValueAsFloat(ChaseRangeKey) : 2000.0f;
    float Distance = FVector::Distance(ControlledEnemy->GetActorLocation(), Target->GetActorLocation());
    
    return Distance <= ChaseRange;
}

bool AEnemyAIController::IsWithinPatrolRange() const
{
    if (!ControlledEnemy)
    {
        return false;
    }
    
    FVector HomeLoc = GetHomeLocation();
    float PatrolRadius = Blackboard ? Blackboard->GetValueAsFloat(PatrolRadiusKey) : 500.0f;
    float Distance = FVector::Distance(ControlledEnemy->GetActorLocation(), HomeLoc);
    
    return Distance <= PatrolRadius;
}

void AEnemyAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    // 感知更新回调（由子类或蓝图实现具体逻辑）
    UE_LOG(LogTemp, Verbose, TEXT("[EnemyAIController] %s 感知更新，检测到 %d 个Actor"), 
        *GetName(), UpdatedActors.Num());
    
    // 如果没有目标，尝试寻找玩家
    if (!HasValidTarget())
    {
        for (AActor* Actor : UpdatedActors)
        {
            // 简单的玩家检测（根据标签或类型）
            if (Actor && Actor->ActorHasTag(FName("Player")))
            {
                SetTargetActor(Actor);
                UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] %s 发现目标: %s"), 
                    *GetName(), *Actor->GetName());
                break;
            }
        }
    }
}

void AEnemyAIController::OnTargetChanged(AActor* NewTarget)
{
    UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] %s 目标改变: %s"), 
        *GetName(), NewTarget ? *NewTarget->GetName() : TEXT("None"));
    
    // 通知状态机进入/退出追击状态
    if (ControlledEnemy && ControlledEnemy->GetStateMachineComponent())
    {
        if (NewTarget)
        {
            // 有新目标，进入追击
            ControlledEnemy->GetStateMachineComponent()->StartChase();
        }
        else
        {
            // 目标丢失，返回待机/巡逻
            ControlledEnemy->GetStateMachineComponent()->StartReturn();
        }
    }
}
