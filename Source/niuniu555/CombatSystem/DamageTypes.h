// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponSystem/WeaponTypes.h"
#include "DamageTypes.generated.h"

// 前置声明（避免循环引用）
class AActor;

// ==================== 伤害计算参数 ====================

/**
 * 伤害计算参数结构
 * 传递给伤害计算器进行伤害计算
 */
USTRUCT(BlueprintType)
struct FDamageCalculationParams
{
    GENERATED_BODY()

    // 基础攻击力（来自攻击者属性）
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float BaseAttack = 0.0f;

    // 技能/攻击倍率
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float SkillMultiplier = 1.0f;

    // 伤害类型
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    EDamageType DamageType;

    // 暴击率 (0-1)
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float CriticalRate = 0.0f;

    // 暴击伤害倍率
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float CriticalDamage = 1.5f;

    // 是否强制暴击（用于某些特殊攻击）
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    bool bForceCritical = false;

    // 伤害浮动范围 (例如 0.1 表示 ±10%)
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float DamageVariance = 0.1f;

    // 攻击者等级（用于等级压制计算）
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    int32 AttackerLevel = 1;

    // 目标防御力
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float TargetDefense = 0.0f;

    // 目标各类型抗性 (0-1)
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float PhysicalResistance = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float FireResistance = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float LightningResistance = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float HolyResistance = 0.0f;

    // 目标等级
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    int32 TargetLevel = 1;

    // 是否忽略防御（真实伤害）
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    bool bIgnoreDefense = false;

    // 是否忽略抗性
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    bool bIgnoreResistance = false;

    // 额外伤害加成
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float DamageBonus = 0.0f;

    // 伤害加成百分比
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float DamageBonusPercent = 0.0f;

    FDamageCalculationParams()
        : BaseAttack(0.0f)
        , SkillMultiplier(1.0f)
        , DamageType(EDamageType::Physical)
        , CriticalRate(0.0f)
        , CriticalDamage(1.5f)
        , bForceCritical(false)
        , DamageVariance(0.1f)
        , AttackerLevel(1)
        , TargetDefense(0.0f)
        , PhysicalResistance(0.0f)
        , FireResistance(0.0f)
        , LightningResistance(0.0f)
        , HolyResistance(0.0f)
        , TargetLevel(1)
        , bIgnoreDefense(false)
        , bIgnoreResistance(false)
        , DamageBonus(0.0f)
        , DamageBonusPercent(0.0f)
    {}

    // 获取对应类型的抗性
    float GetResistanceForType(EDamageType Type) const
    {
        switch (Type)
        {
        case EDamageType::Physical:
            return PhysicalResistance;
        case EDamageType::Fire:
            return FireResistance;
        case EDamageType::Lightning:
            return LightningResistance;
        case EDamageType::Holy:
            return HolyResistance;
        default:
            return 0.0f;
        }
    }
};

// ==================== 伤害计算结果 ====================

/**
 * 详细伤害计算结果
 * 包含伤害计算的中间过程，用于调试和显示
 */
USTRUCT(BlueprintType)
struct FDamageCalculationResult
{
    GENERATED_BODY()

    // 基础伤害（攻击力 × 技能倍率）
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    float BaseDamage = 0.0f;

    // 防御减免后的伤害
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    float DamageAfterDefense = 0.0f;

    // 抗性减免后的伤害
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    float DamageAfterResistance = 0.0f;

    // 暴击倍率（1.0表示未暴击）
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    float CriticalMultiplier = 1.0f;

    // 浮动后的伤害
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    float DamageAfterVariance = 0.0f;

    // 加成后的最终伤害
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    float FinalDamage = 0.0f;

    // 是否暴击
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    bool bCritical = false;

    // 是否命中（可用于闪避判定）
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    bool bHit = true;

    // 伤害类型
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    EDamageType DamageType;

    // 实际应用的防御减免百分比
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    float AppliedDefenseReduction = 0.0f;

    // 实际应用的抗性减免百分比
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    float AppliedResistanceReduction = 0.0f;

    FDamageCalculationResult()
        : BaseDamage(0.0f)
        , DamageAfterDefense(0.0f)
        , DamageAfterResistance(0.0f)
        , CriticalMultiplier(1.0f)
        , DamageAfterVariance(0.0f)
        , FinalDamage(0.0f)
        , bCritical(false)
        , bHit(true)
        , DamageType(EDamageType::Physical)
        , AppliedDefenseReduction(0.0f)
        , AppliedResistanceReduction(0.0f)
    {}
};
