// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyStateMachineComponent.h"
#include "EnemyDataAsset.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UEnemyStateMachineComponent::UEnemyStateMachineComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    CurrentState = EEnemyState::None;
    PreviousState = EEnemyState::None;
    StateTimer = 0.0f;
    StateDuration = 0.0f;
    bIsTransitioning = false;
    bDebugLog = false;
}

void UEnemyStateMachineComponent::BeginPlay()
{
    Super::BeginPlay();

    // 默认进入出生状态
    if (CurrentState == EEnemyState::None)
    {
        StartSpawn();
    }
}

void UEnemyStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 更新状态计时器
    StateTimer += DeltaTime;

    // 更新当前状态
    UpdateState(DeltaTime);
}

void UEnemyStateMachineComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

bool UEnemyStateMachineComponent::IsInCombatState() const
{
    return CurrentState == EEnemyState::Chase || 
           CurrentState == EEnemyState::Attack ||
           CurrentState == EEnemyState::Skill;
}

bool UEnemyStateMachineComponent::CanAttack() const
{
    // 只有在Idle、Patrol、Chase状态下可以攻击
    return CurrentState == EEnemyState::Idle || 
           CurrentState == EEnemyState::Patrol ||
           CurrentState == EEnemyState::Chase;
}

bool UEnemyStateMachineComponent::CanMove() const
{
    // 攻击、受击、眩晕、死亡状态下不能自由移动
    return CurrentState != EEnemyState::Attack &&
           CurrentState != EEnemyState::Skill &&
           CurrentState != EEnemyState::Hit &&
           CurrentState != EEnemyState::Stun &&
           CurrentState != EEnemyState::Dead;
}

bool UEnemyStateMachineComponent::CanBeKnockedBack() const
{
    if (!EnemyData)
    {
        return true;
    }
    return EnemyData->bCanBeKnockedBack && 
           CurrentState != EEnemyState::Dead;
}

bool UEnemyStateMachineComponent::CanBeStunned() const
{
    if (!EnemyData)
    {
        return true;
    }
    return EnemyData->bCanBeStunned && 
           CurrentState != EEnemyState::Stun &&
           CurrentState != EEnemyState::Dead;
}

bool UEnemyStateMachineComponent::RequestStateChange(EEnemyState NewState, bool bForce)
{
    // 检查是否是相同状态
    if (CurrentState == NewState)
    {
        return false;
    }

    // 检查转换是否合法
    if (!CanTransitionTo(CurrentState, NewState, bForce))
    {
        if (bDebugLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("[EnemyStateMachine] 状态转换被拒绝: %s -> %s"),
                *UEnum::GetValueAsString(CurrentState),
                *UEnum::GetValueAsString(NewState));
        }
        return false;
    }

    // 执行状态转换
    ForceStateChange(NewState);
    return true;
}

void UEnemyStateMachineComponent::ForceStateChange(EEnemyState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    EEnemyState OldState = CurrentState;

    // 退出旧状态
    ExitState(OldState);

    // 记录历史
    PreviousState = OldState;
    CurrentState = NewState;
    StateTimer = 0.0f;

    // 进入新状态
    EnterState(NewState);

    // 广播状态变更事件
    OnStateChanged.Broadcast(OldState, NewState);

    // 调试日志
    if (bDebugLog)
    {
        LogStateChange(OldState, NewState);
    }
}

bool UEnemyStateMachineComponent::ReturnToPreviousState()
{
    if (PreviousState != EEnemyState::None && PreviousState != EEnemyState::Dead)
    {
        return RequestStateChange(PreviousState);
    }
    return false;
}

bool UEnemyStateMachineComponent::StartSpawn()
{
    return RequestStateChange(EEnemyState::Spawn);
}

bool UEnemyStateMachineComponent::StartIdle()
{
    return RequestStateChange(EEnemyState::Idle);
}

bool UEnemyStateMachineComponent::StartPatrol()
{
    return RequestStateChange(EEnemyState::Patrol);
}

bool UEnemyStateMachineComponent::StartChase()
{
    return RequestStateChange(EEnemyState::Chase);
}

bool UEnemyStateMachineComponent::StartAttack()
{
    return RequestStateChange(EEnemyState::Attack);
}

bool UEnemyStateMachineComponent::StartHit()
{
    return RequestStateChange(EEnemyState::Hit);
}

bool UEnemyStateMachineComponent::StartStun()
{
    return RequestStateChange(EEnemyState::Stun);
}

bool UEnemyStateMachineComponent::StartReturn()
{
    return RequestStateChange(EEnemyState::Return);
}

bool UEnemyStateMachineComponent::StartDeath()
{
    return RequestStateChange(EEnemyState::Dead);
}

void UEnemyStateMachineComponent::EnterState(EEnemyState NewState)
{
    // 广播特定状态事件
    BroadcastStateEvent(NewState);

    // 根据状态设置持续时间
    if (EnemyData)
    {
        switch (NewState)
        {
            case EEnemyState::Hit:
                StateDuration = EnemyData->HitStunDuration;
                break;
            case EEnemyState::Spawn:
                StateDuration = 1.0f; // 出生动画默认1秒
                break;
            default:
                StateDuration = 0.0f;
                break;
        }
    }
}

void UEnemyStateMachineComponent::ExitState(EEnemyState OldState)
{
    // 子类可重写进行清理工作
}

void UEnemyStateMachineComponent::UpdateState(float DeltaTime)
{
    switch (CurrentState)
    {
        case EEnemyState::Spawn:
            UpdateSpawn(DeltaTime);
            break;
        case EEnemyState::Idle:
            UpdateIdle(DeltaTime);
            break;
        case EEnemyState::Patrol:
            UpdatePatrol(DeltaTime);
            break;
        case EEnemyState::Chase:
            UpdateChase(DeltaTime);
            break;
        case EEnemyState::Attack:
            UpdateAttack(DeltaTime);
            break;
        case EEnemyState::Hit:
            UpdateHit(DeltaTime);
            break;
        case EEnemyState::Stun:
            UpdateStun(DeltaTime);
            break;
        case EEnemyState::Return:
            UpdateReturn(DeltaTime);
            break;
        case EEnemyState::Dead:
            UpdateDead(DeltaTime);
            break;
        default:
            break;
    }
}

void UEnemyStateMachineComponent::UpdateSpawn(float DeltaTime)
{
    // 出生动画完成后进入待机
    if (StateTimer >= StateDuration)
    {
        StartIdle();
    }
}

void UEnemyStateMachineComponent::UpdateIdle(float DeltaTime)
{
    // 待机状态由AI行为树控制转换
}

void UEnemyStateMachineComponent::UpdatePatrol(float DeltaTime)
{
    // 巡逻状态由AI行为树控制转换
}

void UEnemyStateMachineComponent::UpdateChase(float DeltaTime)
{
    // 追击状态由AI行为树控制转换
}

void UEnemyStateMachineComponent::UpdateAttack(float DeltaTime)
{
    // 攻击状态由攻击组件控制转换
    // 攻击动画完成后应自动返回Chase或Idle
}

void UEnemyStateMachineComponent::UpdateHit(float DeltaTime)
{
    // 受击硬直时间结束后返回合适的状态
    if (StateTimer >= StateDuration)
    {
        // 根据上一个状态决定返回什么状态
        // 如果之前正在攻击或技能，回到待机（让玩家有喘息机会）
        // 如果之前在追击，检查是否还有目标
        if (PreviousState == EEnemyState::Attack || PreviousState == EEnemyState::Skill)
        {
            StartIdle();
        }
        else if (PreviousState == EEnemyState::Chase)
        {
            // 如果还有目标，继续追击，否则待机
            AActor* OwnerActor = GetOwner();
            if (OwnerActor)
            {
                AAIController* AIController = Cast<AAIController>(OwnerActor->GetInstigatorController());
                if (!AIController)
                {
                    AIController = Cast<AAIController>(OwnerActor->GetOwner());
                }
                if (AIController)
                {
                    UBlackboardComponent* BB = AIController->GetBlackboardComponent();
                    bool bHasTarget = false;
                    if (BB)
                    {
                        // 尝试获取常见的目标键（与AI行为树任务兼容）
                        static const TArray<FName> CommonTargetKeys = {
                            FName(TEXT("TargetActor")),
                            FName(TEXT("Target")),
                            FName(TEXT("Player")),
                            FName(TEXT("Enemy"))
                        };
                        for (const FName& KeyName : CommonTargetKeys)
                        {
                            UObject* Value = BB->GetValueAsObject(KeyName);
                            if (Value != nullptr)
                            {
                                bHasTarget = true;
                                break;
                            }
                        }
                    }
                    if (bHasTarget)
                    {
                        StartChase();
                    }
                    else
                    {
                        StartIdle();
                    }
                }
                else
                {
                    StartIdle();
                }
            }
            else
            {
                StartIdle();
            }
        }
        else
        {
            ReturnToPreviousState();
        }
    }
}

void UEnemyStateMachineComponent::UpdateStun(float DeltaTime)
{
    // 眩晕时间结束后返回待机
    // 眩晕是强控制，结束后应该重新评估状态，不能直接返回之前的状态
    if (StateTimer >= StateDuration)
    {
        StartIdle();
    }
}

void UEnemyStateMachineComponent::UpdateReturn(float DeltaTime)
{
    // 返回出生点完成后进入待机
}

void UEnemyStateMachineComponent::UpdateDead(float DeltaTime)
{
    // 死亡状态不自动转换
}

bool UEnemyStateMachineComponent::CanTransitionTo(EEnemyState FromState, EEnemyState ToState, bool bForce) const
{
    // 如果强制转换，直接允许（除了已死亡）
    if (bForce)
    {
        return FromState != EEnemyState::Dead;
    }

    // 已死亡状态不能转换
    if (FromState == EEnemyState::Dead)
    {
        return false;
    }

    // 查找转换规则
    const FStateTransitionRule* Rule = FindTransitionRule(FromState, ToState);
    if (Rule)
    {
        return !Rule->bRequireCondition || Rule->bCanBeForced;
    }

    // 没有规则时使用默认规则
    // 死亡状态可以从任何状态进入
    if (ToState == EEnemyState::Dead)
    {
        return true;
    }

    // 受击状态可以从大部分状态进入（打断）
    if (ToState == EEnemyState::Hit)
    {
        return FromState != EEnemyState::Dead;
    }

    // 眩晕状态可以从大部分状态进入
    if (ToState == EEnemyState::Stun)
    {
        return FromState != EEnemyState::Dead && FromState != EEnemyState::Hit;
    }

    // 攻击状态不能互相转换
    if (FromState == EEnemyState::Attack && ToState == EEnemyState::Attack)
    {
        return false;
    }

    return true;
}

const FStateTransitionRule* UEnemyStateMachineComponent::FindTransitionRule(EEnemyState FromState, EEnemyState ToState) const
{
    for (const FStateTransitionRule& Rule : TransitionRules)
    {
        if (Rule.FromState == FromState && Rule.ToState == ToState)
        {
            return &Rule;
        }
    }
    return nullptr;
}

void UEnemyStateMachineComponent::BroadcastStateEvent(EEnemyState State)
{
    switch (State)
    {
        case EEnemyState::Spawn:
            OnEnterSpawn.Broadcast(PreviousState, State);
            break;
        case EEnemyState::Idle:
            OnEnterIdle.Broadcast(PreviousState, State);
            break;
        case EEnemyState::Patrol:
            OnEnterPatrol.Broadcast(PreviousState, State);
            break;
        case EEnemyState::Chase:
            OnEnterChase.Broadcast(PreviousState, State);
            break;
        case EEnemyState::Attack:
            OnEnterAttack.Broadcast(PreviousState, State);
            break;
        case EEnemyState::Hit:
            OnEnterHit.Broadcast(PreviousState, State);
            break;
        case EEnemyState::Stun:
            OnEnterStun.Broadcast(PreviousState, State);
            break;
        case EEnemyState::Return:
            OnEnterReturn.Broadcast(PreviousState, State);
            break;
        case EEnemyState::Dead:
            OnEnterDead.Broadcast(PreviousState, State);
            break;
        default:
            break;
    }
}

void UEnemyStateMachineComponent::LogStateChange(EEnemyState FromState, EEnemyState ToState)
{
    UE_LOG(LogTemp, Log, TEXT("[EnemyStateMachine] %s - 状态转换: %s -> %s"),
        *GetOwner()->GetName(),
        *UEnum::GetValueAsString(FromState),
        *UEnum::GetValueAsString(ToState));
}
