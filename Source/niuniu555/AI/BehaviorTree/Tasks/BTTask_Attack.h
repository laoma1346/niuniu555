// Fill in your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Attack.generated.h"

// 攻击任务日志分类
DECLARE_LOG_CATEGORY_EXTERN(LogAIAttack, Log, All);

/**
 * 攻击任务内存结构体
 * 用于存储每个任务实例的独立状态
 */
USTRUCT()
struct FAttackTaskMemory
{
    GENERATED_BODY()

    /** 当前攻击状态 */
    UPROPERTY()
    uint8 CurrentState = 0; // EAttackState::None
    
    /** 状态计时器 */
    UPROPERTY()
    float StateTimer = 0.0f;
    
    /** 是否已经造成伤害 */
    UPROPERTY()
    bool bHasDealtDamage = false;
    
    /** 拥有者的BehaviorTreeComponent弱引用 */
    UPROPERTY()
    TWeakObjectPtr<UBehaviorTreeComponent> OwnerBehaviorTreeComponent;
    
    /** 动画实例弱引用（用于检查动画状态） */
    UPROPERTY()
    TWeakObjectPtr<UAnimInstance> AnimInstance;
    
    /** 动画开始时间 */
    UPROPERTY()
    float AnimStartTime = 0.0f;
};

/**
 * 攻击任务节点
 * 执行攻击动画并触发伤害判定
 * 使用NodeMemory存储状态，确保行为树实例化正确
 */
UCLASS()
class NIUNIU555_API UBTTask_Attack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_Attack();

    //~ Begin UBTTaskNode Interface
    virtual uint16 GetInstanceMemorySize() const override { return sizeof(FAttackTaskMemory); }
    virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
    //~ End UBTTaskNode Interface
    
    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
    /** 当前攻击状态 */
    enum class EAttackState : uint8
    {
        None,
        WindingUp,      // 前摇
        Attacking,      // 攻击判定
        Recovering      // 后摇
    };
    
    /** 
     * 类型安全地获取攻击任务内存 
     * @param NodeMemory 原始节点内存指针
     * @return 类型转换后的FAttackTaskMemory指针
     */
    FORCEINLINE static FAttackTaskMemory* GetAttackMemory(uint8* NodeMemory)
    {
        return reinterpret_cast<FAttackTaskMemory*>(NodeMemory);
    }

    /** 开始攻击 */
    void StartAttack(AAIController* AIController, APawn* AIPawn, AActor* TargetActor, FAttackTaskMemory* Memory);
    
    /** 攻击蒙太奇完成回调 */
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    
    /** 检查目标是否仍在攻击范围内 */
    bool IsTargetInAttackRange(APawn* AIPawn, AActor* TargetActor) const;
    
    /** 面向目标 */
    void RotateTowardsTarget(APawn* AIPawn, AActor* TargetActor, float DeltaSeconds);
    
    /** 执行伤害判定 */
    void DealDamage(APawn* AIPawn, AActor* TargetActor);

public:
    /** 目标Actor的Blackboard键 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "攻击任务")
    FBlackboardKeySelector TargetActorKey;

    /** 攻击动画蒙太奇 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "攻击任务")
    UAnimMontage* AttackMontage = nullptr;

    /** 攻击前摇时间（秒）- 用于时间判定方式 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "攻击任务", meta = (ClampMin = "0.0"))
    float AttackWindUpTime = 0.3f;

    /** 攻击后摇时间（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "攻击任务", meta = (ClampMin = "0.0"))
    float AttackRecoveryTime = 0.5f;

    /** 攻击伤害 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "攻击任务", meta = (ClampMin = "0.0"))
    float AttackDamage = 20.0f;

    /** 攻击范围 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "攻击任务", meta = (ClampMin = "50.0"))
    float AttackRange = 250.0f;

    /** 攻击角度（前方锥形范围） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "攻击任务", meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float AttackAngle = 60.0f;

    /** 面向目标旋转速度 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "攻击任务", meta = (ClampMin = "1.0"))
    float RotationSpeed = 5.0f;

    /** 攻击冷却时间 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "攻击任务", meta = (ClampMin = "0.0"))
    float AttackCooldown = 1.0f;
    
    /** 是否使用动画进度触发伤害（而不是固定时间） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "攻击任务")
    bool bUseAnimationProgress = false;
    
    /** 伤害触发点（动画播放百分比，0-1） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "攻击任务", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bUseAnimationProgress"))
    float AttackDamageTriggerPoint = 0.3f;
};
