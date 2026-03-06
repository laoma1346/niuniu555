// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatSystem/DamageTypes.h"
#include "DamageableInterface.generated.h"

// 受击反应类型
UENUM(BlueprintType)
enum class EHitReactionType : uint8
{
    None        UMETA(DisplayName = "无反应"),
    Light       UMETA(DisplayName = "轻击受击"),
    Heavy       UMETA(DisplayName = "重击受击"),
    Knockback   UMETA(DisplayName = "击退"),
    Knockdown   UMETA(DisplayName = "击倒"),
    Stun        UMETA(DisplayName = "眩晕")
};

// 受击信息结构
USTRUCT(BlueprintType)
struct FHitInfo
{
    GENERATED_BODY()

    // 伤害值
    UPROPERTY(BlueprintReadOnly, Category = "Hit")
    float Damage = 0.0f;

    // 是否暴击
    UPROPERTY(BlueprintReadOnly, Category = "Hit")
    bool bCritical = false;

    // 伤害类型
    UPROPERTY(BlueprintReadOnly, Category = "Hit")
    EDamageType DamageType = EDamageType::Physical;

    // 攻击者
    UPROPERTY(BlueprintReadOnly, Category = "Hit")
    TWeakObjectPtr<AActor> Attacker = nullptr;

    // 受击位置
    UPROPERTY(BlueprintReadOnly, Category = "Hit")
    FVector HitLocation = FVector::ZeroVector;

    // 受击方向（攻击来源方向）
    UPROPERTY(BlueprintReadOnly, Category = "Hit")
    FVector HitDirection = FVector::ZeroVector;

    // 击退力度
    UPROPERTY(BlueprintReadOnly, Category = "Hit")
    float KnockbackForce = 0.0f;

    // 受击反应类型
    UPROPERTY(BlueprintReadOnly, Category = "Hit")
    EHitReactionType HitReaction = EHitReactionType::None;

    // 是否触发顿帧
    UPROPERTY(BlueprintReadOnly, Category = "Hit")
    bool bTriggerHitStop = false;

    // 顿帧持续时间（秒）
    UPROPERTY(BlueprintReadOnly, Category = "Hit")
    float HitStopDuration = 0.0f;

    FHitInfo()
        : Damage(0.0f)
        , bCritical(false)
        , DamageType(EDamageType::Physical)
        , HitLocation(FVector::ZeroVector)
        , HitDirection(FVector::ZeroVector)
        , KnockbackForce(0.0f)
        , HitReaction(EHitReactionType::None)
        , bTriggerHitStop(false)
        , HitStopDuration(0.0f)
    {}
};

// 受击委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTakeDamage, const FHitInfo&, HitInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDeath);

/**
 * 可受伤接口 - 定义可被伤害的对象必须实现的功能
 * 适用于敌人、玩家、可破坏物体等
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDamageableInterface : public UInterface
{
    GENERATED_BODY()
};

class NIUNIU555_API IDamageableInterface
{
    GENERATED_BODY()

public:
    // ========== 生命/状态查询 ==========

    /**
     * 获取当前生命值
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damageable")
    float GetCurrentHealth() const;
    virtual float GetCurrentHealth_Implementation() const { return 0.0f; }

    /**
     * 获取最大生命值
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damageable")
    float GetMaxHealth() const;
    virtual float GetMaxHealth_Implementation() const { return 0.0f; }

    /**
     * 获取生命值百分比 (0-1)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damageable")
    float GetHealthPercent() const;
    virtual float GetHealthPercent_Implementation() const { return 0.0f; }

    /**
     * 是否存活
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damageable")
    bool IsAlive() const;
    virtual bool IsAlive_Implementation() const { return true; }

    /**
     * 是否可以被伤害
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damageable")
    bool CanBeDamaged() const;
    virtual bool CanBeDamaged_Implementation() const { return true; }

    // ========== 伤害处理 ==========

    /**
     * 受到伤害
     * @param HitInfo 受击信息
     * @return 实际受到的伤害值
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damageable")
    float TakeDamage(const FHitInfo& HitInfo);
    virtual float TakeDamage_Implementation(const FHitInfo& HitInfo) { return 0.0f; }

    /**
     * 治疗
     * @param Amount 治疗量
     * @return 实际治疗量
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damageable")
    float Heal(float Amount);
    virtual float Heal_Implementation(float Amount) { return 0.0f; }

    /**
     * 立即死亡
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damageable")
    void Die();
    virtual void Die_Implementation() {}

    // ========== 防御/抗性查询 ==========

    /**
     * 获取防御力
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damageable")
    float GetDefense() const;
    virtual float GetDefense_Implementation() const { return 0.0f; }

    /**
     * 获取指定类型的抗性
     * @param DamageType 伤害类型
     * @return 抗性值 (0-1)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damageable")
    float GetResistance(EDamageType DamageType) const;
    virtual float GetResistance_Implementation(EDamageType DamageType) const { return 0.0f; }

    // ========== 受击反馈 ==========

    /**
     * 播放受击反应
     * @param HitInfo 受击信息
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damageable")
    void PlayHitReaction(const FHitInfo& HitInfo);
    virtual void PlayHitReaction_Implementation(const FHitInfo& HitInfo) {}

    /**
     * 获取受击反应类型
     * @param Damage 伤害值
     * @param KnockbackForce 击退力度
     * @return 受击反应类型
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damageable")
    EHitReactionType GetHitReactionType(float Damage, float KnockbackForce) const;
    virtual EHitReactionType GetHitReactionType_Implementation(float Damage, float KnockbackForce) const { return EHitReactionType::Light; }
};
