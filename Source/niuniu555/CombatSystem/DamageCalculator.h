// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DamageTypes.h"
#include "DamageCalculator.generated.h"

/**
 * 伤害计算器 - 通用伤害计算工具类
 * 
 * 使用方式：
 * UDamageCalculator::CalculateDamage(Params);
 * 
 * 伤害公式：
 * 基础伤害 = 攻击力 × 技能倍率
 * 防御减免 = Max(基础伤害 × 0.1, 基础伤害 - 防御力)
 * 抗性减免 = 防御减免 × (1 - 抗性)
 * 暴击判定 = 随机值 < 暴击率
 * 最终伤害 = 抗性减免 × 暴击倍率 × 随机浮动(0.9-1.1) + 固定加成 + 百分比加成
 */
UCLASS()
class NIUNIU555_API UDamageCalculator : public UObject
{
    GENERATED_BODY()

public:
    // ========== 主要计算方法 ==========

    /**
     * 计算伤害 - 静态方法，直接调用
     * @param Params 伤害计算参数
     * @return 详细伤害计算结果
     */
    UFUNCTION(BlueprintCallable, Category = "伤害计算", meta = (WorldContext = "WorldContextObject"))
    static FDamageCalculationResult CalculateDamage(const FDamageCalculationParams& Params);

    /**
     * 计算伤害（带随机种子，用于联机同步）
     * @param Params 伤害计算参数
     * @param RandomSeed 随机种子
     * @return 详细伤害计算结果
     */
    UFUNCTION(BlueprintCallable, Category = "伤害计算")
    static FDamageCalculationResult CalculateDamageWithSeed(const FDamageCalculationParams& Params, int32 RandomSeed);

public:
    // ========== 分步计算方法（可用于调试和自定义） ==========

    /**
     * 计算基础伤害
     * 公式：攻击力 × 技能倍率
     */
    UFUNCTION(BlueprintPure, Category = "伤害计算|分步")
    static float CalculateBaseDamage(float BaseAttack, float SkillMultiplier);

    /**
     * 应用防御减免
     * 公式：Max(基础伤害 × 0.1, 基础伤害 - 防御力)
     * 确保最低造成10%伤害
     */
    UFUNCTION(BlueprintPure, Category = "伤害计算|分步")
    static float ApplyDefense(float BaseDamage, float Defense, bool bIgnoreDefense = false);

    /**
     * 应用抗性减免
     * 公式：伤害 × (1 - 抗性)
     */
    UFUNCTION(BlueprintPure, Category = "伤害计算|分步")
    static float ApplyResistance(float Damage, float Resistance, bool bIgnoreResistance = false);

    /**
     * 计算暴击
     * @param Damage 原始伤害
     * @param CriticalRate 暴击率 (0-1)
     * @param CriticalDamage 暴击伤害倍率
     * @param bForceCritical 是否强制暴击
     * @param OutIsCritical 输出是否暴击
     * @return 暴击后的伤害
     */
    UFUNCTION(BlueprintPure, Category = "伤害计算|分步")
    static float ApplyCritical(float Damage, float CriticalRate, float CriticalDamage, bool bForceCritical, bool& OutIsCritical);

    /**
     * 应用伤害浮动
     * 公式：伤害 × Random(1 - 浮动, 1 + 浮动)
     */
    UFUNCTION(BlueprintPure, Category = "伤害计算|分步")
    static float ApplyVariance(float Damage, float Variance);

    /**
     * 应用伤害加成
     * 公式：伤害 × (1 + 百分比加成) + 固定加成
     */
    UFUNCTION(BlueprintPure, Category = "伤害计算|分步")
    static float ApplyDamageBonus(float Damage, float FlatBonus, float PercentBonus);

public:
    // ========== 辅助方法 ==========

    /**
     * 检查是否暴击
     * @param CriticalRate 暴击率 (0-1)
     * @return true表示暴击
     */
    UFUNCTION(BlueprintPure, Category = "伤害计算|辅助")
    static bool CheckCritical(float CriticalRate);

    /**
     * 获取伤害浮动随机值
     * @param Variance 浮动范围 (0.1 = ±10%)
     * @return 浮动倍数 (0.9 - 1.1)
     */
    UFUNCTION(BlueprintPure, Category = "伤害计算|辅助")
    static float GetVarianceMultiplier(float Variance);

    /**
     * 从武器统计创建伤害参数
     * @param WeaponStats 武器基础属性
     * @param DamageType 伤害类型
     * @param SkillMultiplier 技能倍率
     * @return 伤害计算参数
     */
    UFUNCTION(BlueprintPure, Category = "伤害计算|便捷")
    static FDamageCalculationParams CreateParamsFromWeaponStats(const FWeaponBaseStats& WeaponStats, EDamageType DamageType, float SkillMultiplier = 1.0f);

private:
    // 获取随机数生成器（用于确定性的伤害计算）
    static float GetRandomFloat(int32& InOutSeed);
};
