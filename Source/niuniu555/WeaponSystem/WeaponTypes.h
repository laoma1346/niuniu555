// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

// ==================== 武器类型枚举 ====================

/**
 * 武器类型枚举 - 四种武器类型
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    None        UMETA(DisplayName = "无武器"),
    XuanyuanSword   UMETA(DisplayName = "轩辕剑-近战"),
    PanguAxe        UMETA(DisplayName = "盘古斧-蓄力/投掷"),
    GodOfWealthStaff UMETA(DisplayName = "财神金杖-法术"),
    HouyiBow        UMETA(DisplayName = "射日弓-远程")
};

/**
 * 武器稀有度/品质枚举
 */
UENUM(BlueprintType)
enum class EWeaponRarity : uint8
{
    Common      UMETA(DisplayName = "普通-白色"),
    Rare        UMETA(DisplayName = "稀有-蓝色"),
    Epic        UMETA(DisplayName = "史诗-金色"),
    Legendary   UMETA(DisplayName = "传说-暗金"),
    Mythic      UMETA(DisplayName = "神话-红色")
};

/**
 * 武器攻击类型
 */
UENUM(BlueprintType)
enum class EWeaponAttackType : uint8
{
    Melee       UMETA(DisplayName = "近战"),
    Ranged      UMETA(DisplayName = "远程"),
    Magic       UMETA(DisplayName = "法术"),
    Hybrid      UMETA(DisplayName = "混合")
};

/**
 * 武器状态枚举
 */
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    Idle        UMETA(DisplayName = "空闲"),
    Attacking   UMETA(DisplayName = "攻击中"),
    Charging    UMETA(DisplayName = "蓄力中"),
    Reloading   UMETA(DisplayName = "装填中"),
    Switching   UMETA(DisplayName = "切换中"),
    Cooldown    UMETA(DisplayName = "冷却中")
};

// ==================== 伤害类型 ====================

/**
 * 伤害类型枚举
 */
UENUM(BlueprintType)
enum class EDamageType : uint8
{
    Physical    UMETA(DisplayName = "物理"),
    Fire        UMETA(DisplayName = "火焰"),
    Lightning   UMETA(DisplayName = "雷电"),
    Holy        UMETA(DisplayName = "神圣")
};

/**
 * 伤害结果结构体
 */
USTRUCT(BlueprintType)
struct FDamageResult
{
    GENERATED_BODY()

    // 是否命中
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    bool bHit = false;

    // 最终伤害值
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    float FinalDamage = 0.0f;

    // 是否暴击
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    bool bCritical = false;

    // 伤害类型
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    EDamageType DamageType = EDamageType::Physical;

    // 受击位置
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    FVector HitLocation = FVector::ZeroVector;

    // 受击法线
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    FVector HitNormal = FVector::UpVector;

    // 被击中的Actor
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    TWeakObjectPtr<AActor> HitActor = nullptr;
};

// ==================== 攻击输入类型 ====================

/**
 * 攻击输入类型 - 轻击/重击
 * 与 ComboInputSystem 的 EComboInputType 保持一致
 */
UENUM(BlueprintType)
enum class EWeaponAttackInput : uint8
{
    LightAttack UMETA(DisplayName = "轻击-L"),
    HeavyAttack UMETA(DisplayName = "重击-H")
};

// ==================== 连招阶段信息 ====================

/**
 * 连招阶段信息
 */
USTRUCT(BlueprintType)
struct FComboStageInfo
{
    GENERATED_BODY()

    // 阶段索引（从0开始）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    int32 StageIndex = 0;

    // 阶段名称（用于动画蒙太奇Section）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    FName StageName;

    // 此阶段的伤害倍率
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    float DamageMultiplier = 1.0f;

    // 此阶段的攻击范围倍率
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    float RangeMultiplier = 1.0f;

    // 此阶段的击退力度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    float KnockbackForce = 300.0f;

    // 是否可以取消（用于Cancel Window）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    bool bCanCancel = false;

    // 取消窗口开始时间（相对于动画开始，秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    float CancelWindowStart = 0.3f;

    // 取消窗口结束时间（相对于动画开始，秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    float CancelWindowEnd = 0.5f;
};

// ==================== 武器基础属性 ====================

/**
 * 武器基础属性
 */
USTRUCT(BlueprintType)
struct FWeaponBaseStats
{
    GENERATED_BODY()

    // 基础攻击力
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float BaseAttack = 10.0f;

    // 攻击速度（影响动画播放速度）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackSpeed = 1.0f;

    // 暴击率 (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = 0, ClampMax = 1))
    float CriticalRate = 0.05f;

    // 暴击伤害倍率
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CriticalDamage = 1.5f;

    // 攻击范围
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 150.0f;

    // 攻击角度（扇形角度，用于近战）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackAngle = 60.0f;

    // 耐力消耗
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float StaminaCost = 10.0f;

    // 技能冷却时间（秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CooldownTime = 0.5f;
};
