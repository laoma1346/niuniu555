// Fill in your copyright notice in the Description page of Project Settings.

#include "BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "CombatSystem/DamageableInterface.h"
#include "CombatSystem/DamageTypes.h"
#include "EnemySystem/EnemyBase.h"
#include "EnemySystem/EnemyDataAsset.h"

// 定义日志分类
DEFINE_LOG_CATEGORY(LogAIAttack);

UBTTask_Attack::UBTTask_Attack()
{
    NodeName = TEXT("攻击目标");
    bNotifyTick = true;
}

void UBTTask_Attack::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
    if (NodeMemory)
    {
        FAttackTaskMemory* AttackMemory = new (NodeMemory) FAttackTaskMemory();
        
        if (InitType == EBTMemoryInit::Initialize)
        {
            AttackMemory->CurrentState = static_cast<uint8>(EAttackState::None);
            AttackMemory->StateTimer = 0.0f;
            AttackMemory->bHasDealtDamage = false;
            AttackMemory->OwnerBehaviorTreeComponent = nullptr;
            AttackMemory->AnimInstance = nullptr;
            AttackMemory->AnimStartTime = 0.0f;
        }
    }
    
    Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
}

void UBTTask_Attack::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);
    
    // 强制启用TickTask（防止蓝图实例化时覆盖）
    bNotifyTick = true;
    
    // 初始化Key
    TargetActorKey.ResolveSelectedKey(*Asset.BlackboardAsset);
    
    UE_LOG(LogAIAttack, Verbose, TEXT("InitializeFromAsset - 类名: %s, bNotifyTick=%d"), 
        *GetClass()->GetName(), bNotifyTick);
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UE_LOG(LogAIAttack, Log, TEXT("ExecuteTask被调用 - 实例: %p"), this);
    
    // 获取任务内存
    FAttackTaskMemory* Memory = GetAttackMemory(NodeMemory);
    
    // 初始化状态
    Memory->CurrentState = static_cast<uint8>(EAttackState::WindingUp);
    Memory->StateTimer = 0.0f;
    Memory->bHasDealtDamage = false;
    Memory->OwnerBehaviorTreeComponent = &OwnerComp;
    Memory->AnimInstance = nullptr;
    Memory->AnimStartTime = 0.0f;
    
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    
    if (!AIController || !Blackboard)
    {
        UE_LOG(LogAIAttack, Warning, TEXT("AIController或Blackboard为空"));
        return EBTNodeResult::Failed;
    }
    
    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn)
    {
        UE_LOG(LogAIAttack, Warning, TEXT("AI控制的Pawn为空"));
        return EBTNodeResult::Failed;
    }
    
    // 获取目标Actor
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
    
    if (!TargetActor)
    {
        UE_LOG(LogAIAttack, Warning, TEXT("目标Actor为空，KeyName=%s"),
            *TargetActorKey.SelectedKeyName.ToString());
        return EBTNodeResult::Failed;
    }
    
    // 【关键修复】检查攻击冷却，但不在冷却中时返回失败，而是等待
    if (AEnemyBase* Enemy = Cast<AEnemyBase>(AIPawn))
    {
        if (!Enemy->CanAttack())
        {
            float RemainingCooldown = Enemy->GetAttackCooldownRemaining();
            UE_LOG(LogAIAttack, Log, TEXT("%s 攻击冷却中，剩余%.2f秒，等待后重试"), 
                *AIPawn->GetName(), RemainingCooldown);
            
            // 冷却中返回Failed，让行为树重试（Selector会重新评估）
            return EBTNodeResult::Failed;
        }
    }
    
    // 检查目标是否在攻击范围内
    if (!IsTargetInAttackRange(AIPawn, TargetActor))
    {
        UE_LOG(LogAIAttack, Log, TEXT("目标不在攻击范围内，距离: %.1f / %.1f"),
            FVector::Dist2D(AIPawn->GetActorLocation(), TargetActor->GetActorLocation()), AttackRange);
        return EBTNodeResult::Failed;
    }
    
    // 开始攻击冷却
    if (AEnemyBase* Enemy = Cast<AEnemyBase>(AIPawn))
    {
        Enemy->StartAttackCooldown();
    }
    
    // 开始攻击
    StartAttack(AIController, AIPawn, TargetActor, Memory);
    
    return EBTNodeResult::InProgress;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    // 获取任务内存
    FAttackTaskMemory* Memory = GetAttackMemory(NodeMemory);
    
    if (!Memory->OwnerBehaviorTreeComponent.IsValid())
    {
        UE_LOG(LogAIAttack, Warning, TEXT("OwnerBehaviorTreeComponent无效"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
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
    
    // 获取目标Actor
    AActor* TargetActor = nullptr;
    if (!TargetActorKey.SelectedKeyName.IsNone())
    {
        TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    }
    if (!TargetActor && TargetActorKey.GetSelectedKeyID() != FBlackboard::InvalidKey)
    {
        TargetActor = Cast<AActor>(Blackboard->GetValue<UBlackboardKeyType_Object>(TargetActorKey.GetSelectedKeyID()));
    }
    
    if (!TargetActor)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // 状态机更新
    Memory->StateTimer += DeltaSeconds;
    
    EAttackState CurrentState = static_cast<EAttackState>(Memory->CurrentState);
    
    // 检查是否触发伤害（在switch外定义）
    bool bShouldDealDamage = false;
    
    switch (CurrentState)
    {
    case EAttackState::WindingUp:
        // 前摇阶段：面向目标
        RotateTowardsTarget(AIPawn, TargetActor, DeltaSeconds);
        
        // 检查是否触发伤害
        if (bUseAnimationProgress && AttackMontage && Memory->AnimInstance.IsValid())
        {
            // 使用动画进度
            if (Memory->AnimInstance->Montage_IsPlaying(AttackMontage))
            {
                float Position = Memory->AnimInstance->Montage_GetPosition(AttackMontage);
                float Length = AttackMontage->GetPlayLength();
                float NormalizedPosition = Length > 0.0f ? Position / Length : 0.0f;
                
                if (NormalizedPosition >= AttackDamageTriggerPoint && !Memory->bHasDealtDamage)
                {
                    bShouldDealDamage = true;
                }
            }
            else
            {
                // 动画已结束但未触发伤害，立即触发
                if (!Memory->bHasDealtDamage)
                {
                    bShouldDealDamage = true;
                    UE_LOG(LogAIAttack, Warning, TEXT("动画已结束但未触发伤害，强制触发"));
                }
            }
        }
        else
        {
            // 使用时间
            if (Memory->StateTimer >= AttackWindUpTime && !Memory->bHasDealtDamage)
            {
                bShouldDealDamage = true;
            }
        }
        
        if (bShouldDealDamage)
        {
            // 执行伤害判定
            if (IsTargetInAttackRange(AIPawn, TargetActor))
            {
                DealDamage(AIPawn, TargetActor);
            }
            Memory->bHasDealtDamage = true;
            UE_LOG(LogAIAttack, Log, TEXT("伤害已触发，进入攻击判定阶段"));
            Memory->CurrentState = static_cast<uint8>(EAttackState::Attacking);
            Memory->StateTimer = 0.0f;
        }
        break;
        
    case EAttackState::Attacking:
        // 攻击判定阶段：检查目标是否仍然在攻击范围内
        if (IsTargetInAttackRange(AIPawn, TargetActor))
        {
            if (!Memory->bHasDealtDamage)
            {
                DealDamage(AIPawn, TargetActor);
                Memory->bHasDealtDamage = true;
            }
        }
        else
        {
            UE_LOG(LogAIAttack, Log, TEXT("目标移出攻击范围，攻击判定失败"));
        }
        
        // 攻击判定完成，进入后摇阶段
        UE_LOG(LogAIAttack, Log, TEXT("攻击判定完成，进入后摇阶段"));
        Memory->CurrentState = static_cast<uint8>(EAttackState::Recovering);
        Memory->StateTimer = 0.0f;
        break;
        
    case EAttackState::Recovering:
        // 后摇阶段：等待动画结束 + 后摇时间
        {
            bool bCanFinish = true;
            bool bAnimationPlaying = false;
            float AnimPlayTime = 0.0f;
            
            // 1. 检查动画是否仍在播放
            if (Memory->AnimInstance.IsValid())
            {
                UAnimMontage* CurrentMontage = nullptr;
                
                // 首先检查是否有从敌人数据资产加载的动画
                if (AAIController* CurrentAIController = OwnerComp.GetAIOwner())
                {
                    if (APawn* CurrentAIPawn = CurrentAIController->GetPawn())
                    {
                        if (AEnemyBase* EnemyBase = Cast<AEnemyBase>(CurrentAIPawn))
                        {
                            if (EnemyBase->GetEnemyData() && EnemyBase->GetEnemyData()->AttackMontage)
                            {
                                CurrentMontage = EnemyBase->GetEnemyData()->AttackMontage;
                            }
                        }
                    }
                }
                
                // 如果没有，使用手动配置的动画
                if (!CurrentMontage && AttackMontage)
                {
                    CurrentMontage = AttackMontage;
                }
                
                if (CurrentMontage)
                {
                    if (Memory->AnimInstance->Montage_IsPlaying(CurrentMontage))
                    {
                        bAnimationPlaying = true;
                        AnimPlayTime = Memory->AnimInstance->GetCurrentActiveMontage() ? 
                            Memory->AnimInstance->Montage_GetPosition(CurrentMontage) : 0.0f;
                        
                        // 【关键修复】如果动画播放时间过长（超过动画长度+0.5秒），强制结束
                        float AnimLength = CurrentMontage->GetPlayLength();
                        if (AnimPlayTime > AnimLength + 0.5f)
                        {
                            UE_LOG(LogAIAttack, Warning, TEXT("动画播放时间过长(%.2f/%.2f)，强制结束"), 
                                AnimPlayTime, AnimLength);
                            Memory->AnimInstance->Montage_Stop(0.1f, CurrentMontage);
                            bAnimationPlaying = false;
                        }
                        else
                        {
                            bCanFinish = false;
                        }
                    }
                }
            }
            
            // 2. 检查最小后摇时间
            if (Memory->StateTimer < AttackRecoveryTime)
            {
                bCanFinish = false;
            }
            
            // 只有动画和后摇时间都结束，才完成任务
            if (bCanFinish)
            {
                UE_LOG(LogAIAttack, Log, TEXT("攻击完成（动画:%d 后摇:%.2f），返回Succeeded"), 
                    !bAnimationPlaying, Memory->StateTimer);
                FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            }
            else if (Memory->StateTimer >= 5.0f)  // 【修复】超时时间从10秒改为5秒
            {
                // 超时保护：5秒强制结束
                UE_LOG(LogAIAttack, Warning, TEXT("攻击任务超时（动画:%d 后摇:%.2f/%.2f），强制结束"), 
                    bAnimationPlaying, Memory->StateTimer, AttackRecoveryTime);
                
                // 强制停止动画
                if (Memory->AnimInstance.IsValid())
                {
                    // 停止手动配置的动画
                    if (AttackMontage)
                    {
                        Memory->AnimInstance->Montage_Stop(0.1f, AttackMontage);
                    }
                    
                    // 停止从敌人数据资产加载的动画
                    if (AAIController* CurrentAIController = OwnerComp.GetAIOwner())
                    {
                        if (APawn* CurrentAIPawn = CurrentAIController->GetPawn())
                        {
                            if (AEnemyBase* EnemyBase = Cast<AEnemyBase>(CurrentAIPawn))
                            {
                                if (EnemyBase->GetEnemyData() && EnemyBase->GetEnemyData()->AttackMontage)
                                {
                                    UAnimMontage* EnemyAttackMontage = EnemyBase->GetEnemyData()->AttackMontage;
                                    Memory->AnimInstance->Montage_Stop(0.1f, EnemyAttackMontage);
                                }
                            }
                        }
                    }
                }
                
                FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);  // 【修复】超时返回Succeeded而非Failed
            }
        }
        break;
        
    default:
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        break;
    }
}

void UBTTask_Attack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    // 获取任务内存
    FAttackTaskMemory* Memory = GetAttackMemory(NodeMemory);
    
    // 停止动画（如果还在播放）
    if (Memory->AnimInstance.IsValid())
    {
        // 停止手动配置的动画
        if (AttackMontage && Memory->AnimInstance->Montage_IsPlaying(AttackMontage))
        {
            Memory->AnimInstance->Montage_Stop(0.25f, AttackMontage);
        }
        
        // 尝试停止从敌人数据资产加载的动画
        if (AAIController* CurrentAIController = OwnerComp.GetAIOwner())
        {
            if (APawn* CurrentAIPawn = CurrentAIController->GetPawn())
            {
                if (AEnemyBase* EnemyBase = Cast<AEnemyBase>(CurrentAIPawn))
                {
                    if (EnemyBase->GetEnemyData() && EnemyBase->GetEnemyData()->AttackMontage)
                    {
                        UAnimMontage* EnemyAttackMontage = EnemyBase->GetEnemyData()->AttackMontage;
                        if (Memory->AnimInstance->Montage_IsPlaying(EnemyAttackMontage))
                        {
                            Memory->AnimInstance->Montage_Stop(0.25f, EnemyAttackMontage);
                        }
                    }
                }
            }
        }
    }
    
    // 清理内存
    Memory->CurrentState = static_cast<uint8>(EAttackState::None);
    Memory->StateTimer = 0.0f;
    Memory->bHasDealtDamage = false;
    Memory->OwnerBehaviorTreeComponent = nullptr;
    Memory->AnimInstance = nullptr;
    Memory->AnimStartTime = 0.0f;
    
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_Attack::StartAttack(AAIController* AIController, APawn* AIPawn, AActor* TargetActor, FAttackTaskMemory* Memory)
{
    // 播放攻击动画
    if (ACharacter* AICharacter = Cast<ACharacter>(AIPawn))
    {
        if (UAnimInstance* AnimInstance = AICharacter->GetMesh()->GetAnimInstance())
        {
            // 保存AnimInstance引用
            Memory->AnimInstance = AnimInstance;
            Memory->AnimStartTime = GetWorld()->GetTimeSeconds();
            
            // 尝试从敌人数据资产中获取攻击动画
            UAnimMontage* MontageToPlay = AttackMontage;
            
            // 检查是否是AEnemyBase类型
            if (AEnemyBase* EnemyBase = Cast<AEnemyBase>(AICharacter))
            {
                if (EnemyBase->GetEnemyData() && EnemyBase->GetEnemyData()->AttackMontage)
                {
                    MontageToPlay = EnemyBase->GetEnemyData()->AttackMontage;
                    UE_LOG(LogAIAttack, Log, TEXT("从敌人数据资产加载攻击动画: %s"), *MontageToPlay->GetName());
                }
            }
            
            if (MontageToPlay)
            {
                // 绑定动画完成回调
                FOnMontageEnded EndDelegate;
                EndDelegate.BindUObject(this, &UBTTask_Attack::OnAttackMontageEnded);
                AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
                
                float Duration = AICharacter->PlayAnimMontage(MontageToPlay);
                UE_LOG(LogAIAttack, Log, TEXT("播放攻击动画，时长: %.2f"), Duration);
            }
            else
            {
                UE_LOG(LogAIAttack, Warning, TEXT("攻击动画未设置"));
            }
        }
        else
        {
            UE_LOG(LogAIAttack, Warning, TEXT("无法获取AnimInstance"));
        }
    }
    
    UE_LOG(LogAIAttack, Log, TEXT("开始攻击目标: %s"), *TargetActor->GetName());
}

void UBTTask_Attack::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UE_LOG(LogAIAttack, Verbose, TEXT("攻击动画结束 - 被打断: %d"), bInterrupted);
}

void UBTTask_Attack::DealDamage(APawn* AIPawn, AActor* TargetActor)
{
    // 创建伤害信息
    FHitInfo HitInfo;
    HitInfo.Damage = AttackDamage;
    HitInfo.Attacker = AIPawn;
    HitInfo.HitLocation = TargetActor->GetActorLocation();
    HitInfo.HitDirection = (TargetActor->GetActorLocation() - AIPawn->GetActorLocation()).GetSafeNormal();
    HitInfo.bCritical = false;
    HitInfo.HitReaction = EHitReactionType::Light;
    
    // 应用伤害
    if (TargetActor->GetClass()->ImplementsInterface(UDamageableInterface::StaticClass()))
    {
        IDamageableInterface::Execute_TakeDamage(TargetActor, HitInfo);
        UE_LOG(LogAIAttack, Log, TEXT("对 %s 造成 %.1f 伤害"), *TargetActor->GetName(), AttackDamage);
    }
}

bool UBTTask_Attack::IsTargetInAttackRange(APawn* AIPawn, AActor* TargetActor) const
{
    // 使用平方距离（忽略Z轴），与行为树装饰器保持一致
    float DistSq = FVector::DistSquared2D(AIPawn->GetActorLocation(), TargetActor->GetActorLocation());
    if (DistSq > FMath::Square(AttackRange))
    {
        return false;
    }
    
    // 检查角度
    FVector DirectionToTarget = (TargetActor->GetActorLocation() - AIPawn->GetActorLocation()).GetSafeNormal();
    FVector ForwardVector = AIPawn->GetActorForwardVector();
    
    float DotProduct = FVector::DotProduct(ForwardVector, DirectionToTarget);
    float AngleDegrees = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)) * (180.0f / PI);
    
    if (AngleDegrees > AttackAngle)
    {
        return false;
    }
    
    return true;
}

void UBTTask_Attack::RotateTowardsTarget(APawn* AIPawn, AActor* TargetActor, float DeltaSeconds)
{
    FVector TargetLocation = TargetActor->GetActorLocation();
    FVector AILocation = AIPawn->GetActorLocation();
    
    FVector Direction = (TargetLocation - AILocation).GetSafeNormal();
    Direction.Z = 0.0f; // 只在水平面旋转
    
    if (Direction.IsNearlyZero())
    {
        return;
    }
    
    FRotator TargetRotation = Direction.Rotation();
    FRotator CurrentRotation = AIPawn->GetActorRotation();
    
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, RotationSpeed);
    AIPawn->SetActorRotation(NewRotation);
}
