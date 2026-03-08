// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatSystem/DamageableInterface.h"
#include "EnemyTypes.h"
#include "EnemyBase.generated.h"

// 日志分类声明
DECLARE_LOG_CATEGORY_EXTERN(LogEnemy, Log, All);

// 前置声明组件类
class UAttributeComponent;
class UHitReactionComponent;
class UEnemyStateMachineComponent;
class UEnemyDataAsset;

/**
 * 敌人基类 - 所有敌人的基础类
 * 整合属性系统、状态机、受击反馈
 * 实现IDamageableInterface接口与战斗系统交互
 * 文件路径: Source/niuniu555/EnemySystem/EnemyBase.h
 */
UCLASS(Abstract, Blueprintable)
class NIUNIU555_API AEnemyBase : public ACharacter, public IDamageableInterface
{
    GENERATED_BODY()

public:
    AEnemyBase(const FObjectInitializer& ObjectInitializer);

    // ========== Actor生命周期 ==========
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ========== 初始化 ==========

    // 从数据资产初始化敌人
    UFUNCTION(BlueprintCallable, Category = "敌人")
    virtual void InitializeFromDataAsset(UEnemyDataAsset* DataAsset, int32 Chapter = 1);

    // 设置章节（影响属性数值）
    UFUNCTION(BlueprintCallable, Category = "敌人")
    virtual void SetChapter(int32 NewChapter);

    // ========== IDamageableInterface实现 ==========

    // ========== IDamageableInterface实现 ==========
    // 注意：接口方法不能使用override，直接实现接口方法

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "伤害接口")
    float GetCurrentHealth() const;
    virtual float GetCurrentHealth_Implementation() const override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "伤害接口")
    float GetMaxHealth() const;
    virtual float GetMaxHealth_Implementation() const override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "伤害接口")
    float GetHealthPercent() const;
    virtual float GetHealthPercent_Implementation() const override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "伤害接口")
    bool IsAlive() const;
    virtual bool IsAlive_Implementation() const override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "伤害接口")
    bool CanBeDamaged() const;
    virtual bool CanBeDamaged_Implementation() const override;

    // 伤害处理 - 实现IDamageableInterface::TakeDamage
    // 【修复】使用 _Implementation 后缀，与 BlueprintNativeEvent 接口匹配
    virtual float TakeDamage_Implementation(const FHitInfo& HitInfo) override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "伤害接口")
    float Heal(float Amount);
    virtual float Heal_Implementation(float Amount) override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "伤害接口")
    void Die();
    virtual void Die_Implementation() override;

    // 防御/抗性
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "伤害接口")
    float GetDefense() const;
    virtual float GetDefense_Implementation() const override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "伤害接口")
    float GetResistance(EDamageType DamageType) const;
    virtual float GetResistance_Implementation(EDamageType DamageType) const override;

    // 受击反馈
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "伤害接口")
    void PlayHitReaction(const FHitInfo& HitInfo);
    virtual void PlayHitReaction_Implementation(const FHitInfo& HitInfo) override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "伤害接口")
    EHitReactionType GetHitReactionType(float Damage, float KnockbackForce) const;
    virtual EHitReactionType GetHitReactionType_Implementation(float Damage, float KnockbackForce) const override;

    // ========== 组件访问 ==========

    UFUNCTION(BlueprintPure, Category = "敌人|组件")
    UAttributeComponent* GetAttributeComponent() const { return AttributeComp; }

    UFUNCTION(BlueprintPure, Category = "敌人|组件")
    UHitReactionComponent* GetHitReactionComponent() const { return HitReactionComp; }

    UFUNCTION(BlueprintPure, Category = "敌人|组件")
    UEnemyStateMachineComponent* GetStateMachineComponent() const { return StateMachineComp; }

    UFUNCTION(BlueprintPure, Category = "敌人|数据")
    UEnemyDataAsset* GetEnemyData() const { return EnemyData; }

    // ========== 敌人查询 ==========

    UFUNCTION(BlueprintPure, Category = "敌人")
    int32 GetChapter() const { return CurrentChapter; }

    UFUNCTION(BlueprintPure, Category = "敌人")
    FVector GetSpawnLocation() const { return SpawnLocation; }

    UFUNCTION(BlueprintPure, Category = "敌人")
    bool IsBoss() const;

    UFUNCTION(BlueprintPure, Category = "敌人")
    bool IsElite() const;

    UFUNCTION(BlueprintPure, Category = "敌人")
    EEnemyType GetEnemyType() const;

    UFUNCTION(BlueprintPure, Category = "敌人")
    EEnemyState GetCurrentState() const;

    // ========== AI相关 ==========

    // 获取当前攻击目标
    UFUNCTION(BlueprintPure, Category = "敌人|AI")
    AActor* GetTarget() const { return CurrentTarget.Get(); }

    // 设置攻击目标
    UFUNCTION(BlueprintCallable, Category = "敌人|AI")
    void SetTarget(AActor* NewTarget);

    // 获取出生位置
    UFUNCTION(BlueprintPure, Category = "敌人|AI")
    FVector GetHomeLocation() const { return SpawnLocation; }

    // ========== 攻击冷却管理 ==========

    // 检查是否可以攻击（冷却是否结束）
    UFUNCTION(BlueprintPure, Category = "敌人|攻击")
    bool CanAttack() const;

    // 开始攻击冷却
    UFUNCTION(BlueprintCallable, Category = "敌人|攻击")
    void StartAttackCooldown();

    // 获取攻击冷却剩余时间
    UFUNCTION(BlueprintPure, Category = "敌人|攻击")
    float GetAttackCooldownRemaining() const { return AttackCooldownTimer; }

    // 是否在攻击冷却中
    UFUNCTION(BlueprintPure, Category = "敌人|攻击")
    bool IsInAttackCooldown() const { return bIsInAttackCooldown; }

    // ========== 调试 ==========

    // 显示调试信息
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "敌人|调试")
    bool bShowDebugInfo;

protected:
    // ========== 组件 ==========

    // 属性组件 - 管理生命值等属性
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "组件")
    TObjectPtr<UAttributeComponent> AttributeComp;

    // 受击反馈组件 - 处理闪白、顿帧、击退
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "组件")
    TObjectPtr<UHitReactionComponent> HitReactionComp;

    // 状态机组件 - 管理敌人状态
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "组件")
    TObjectPtr<UEnemyStateMachineComponent> StateMachineComp;

    // ========== 数据配置 ==========

    // 敌人数据资产
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "数据")
    TObjectPtr<UEnemyDataAsset> EnemyData;

    // 当前章节（影响属性数值）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "数据")
    int32 CurrentChapter;

    // 出生位置（用于巡逻返回）
    UPROPERTY(BlueprintReadOnly, Category = "数据")
    FVector SpawnLocation;

    // 当前攻击目标
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    TWeakObjectPtr<AActor> CurrentTarget;

    // ========== 运行时状态 ==========

    // 是否已初始化
    UPROPERTY(BlueprintReadOnly, Category = "状态")
    bool bInitialized;

    // 是否正在死亡
    UPROPERTY(BlueprintReadOnly, Category = "状态")
    bool bIsDying;

    // ========== 攻击冷却状态 ==========

    // 攻击冷却计时器
    UPROPERTY(BlueprintReadOnly, Category = "状态|攻击")
    float AttackCooldownTimer;

    // 是否在攻击冷却中
    UPROPERTY(BlueprintReadOnly, Category = "状态|攻击")
    bool bIsInAttackCooldown;

    // ========== 受保护函数 ==========

    // 计算实际伤害（考虑防御和抗性）
    UFUNCTION(BlueprintCallable, Category = "伤害")
    virtual float CalculateActualDamage(const FHitInfo& HitInfo) const;

    // 处理死亡
    UFUNCTION(BlueprintCallable, Category = "生命周期")
    virtual void HandleDeath();

    // 生成掉落物
    UFUNCTION(BlueprintCallable, Category = "掉落")
    virtual void SpawnLoot();

    // 播放死亡动画
    UFUNCTION(BlueprintCallable, Category = "动画")
    virtual void PlayDeathAnimation();

    // 启用布娃娃物理
    UFUNCTION(BlueprintCallable, Category = "物理")
    virtual void EnableRagdoll();

    // 清理尸体
    UFUNCTION(BlueprintCallable, Category = "生命周期")
    virtual void DestroyCorpse();

    // 更新属性组件数值（从数据资产）
    UFUNCTION()
    virtual void UpdateAttributeStats();

    // ========== 事件处理 ==========

    // 属性变化事件
    UFUNCTION()
    virtual void OnHealthChanged(float NewHealth, float MaxHealth, float Delta);

    // 状态机状态变更事件
    UFUNCTION()
    virtual void OnStateChanged(EEnemyState OldState, EEnemyState NewState);

    // 受击反应事件
    UFUNCTION()
    virtual void OnHitReactionReceived(const FHitInfo& HitInfo);

    // ========== 蓝图可重写事件 ==========

    // 初始化完成时调用
    UFUNCTION(BlueprintImplementableEvent, Category = "敌人|事件", meta = (DisplayName = "OnInitialized"))
    void BP_OnInitialized();

    // 受到伤害时调用
    UFUNCTION(BlueprintImplementableEvent, Category = "敌人|事件", meta = (DisplayName = "OnTakeDamage"))
    void BP_OnTakeDamage(float Damage, const FHitInfo& HitInfo);

    // 死亡时调用
    UFUNCTION(BlueprintImplementableEvent, Category = "敌人|事件", meta = (DisplayName = "OnDeath"))
    void BP_OnDeath();

    // 生成掉落物时调用
    UFUNCTION(BlueprintImplementableEvent, Category = "敌人|事件", meta = (DisplayName = "OnSpawnLoot"))
    void BP_OnSpawnLoot();

public:
    // ========== 委托事件（供外部绑定）==========

    // 敌人死亡事件
    UPROPERTY(BlueprintAssignable, Category = "敌人|事件")
    FOnCharacterDeath OnDeath;

    // 敌人受击事件
    UPROPERTY(BlueprintAssignable, Category = "敌人|事件")
    FOnTakeDamage OnTakeDamageEvent;
};
