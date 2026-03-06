// Copyright Epic Games, Inc. All Rights Reserved.

#include "DamageCalculator.h"

FDamageCalculationResult UDamageCalculator::CalculateDamage(const FDamageCalculationParams& Params)
{
    FDamageCalculationResult Result;
    Result.DamageType = Params.DamageType;

    // 1. 计算基础伤害
    Result.BaseDamage = CalculateBaseDamage(Params.BaseAttack, Params.SkillMultiplier);

    // 2. 应用防御减免
    Result.DamageAfterDefense = ApplyDefense(Result.BaseDamage, Params.TargetDefense, Params.bIgnoreDefense);
    Result.AppliedDefenseReduction = (Result.BaseDamage > 0.0f) ? 
        (1.0f - Result.DamageAfterDefense / Result.BaseDamage) : 0.0f;

    // 3. 应用抗性减免
    float TargetResistance = Params.GetResistanceForType(Params.DamageType);
    Result.DamageAfterResistance = ApplyResistance(Result.DamageAfterDefense, TargetResistance, Params.bIgnoreResistance);
    Result.AppliedResistanceReduction = Params.bIgnoreResistance ? 0.0f : TargetResistance;

    // 4. 计算暴击
    bool bIsCritical = false;
    float DamageAfterCritical = ApplyCritical(
        Result.DamageAfterResistance, 
        Params.CriticalRate, 
        Params.CriticalDamage, 
        Params.bForceCritical, 
        bIsCritical
    );
    Result.bCritical = bIsCritical;
    Result.CriticalMultiplier = bIsCritical ? Params.CriticalDamage : 1.0f;

    // 5. 应用伤害浮动
    Result.DamageAfterVariance = ApplyVariance(DamageAfterCritical, Params.DamageVariance);

    // 6. 应用伤害加成
    Result.FinalDamage = ApplyDamageBonus(Result.DamageAfterVariance, Params.DamageBonus, Params.DamageBonusPercent);

    // 确保伤害不为负
    Result.FinalDamage = FMath::Max(0.0f, Result.FinalDamage);

    // 向下取整（整数伤害）
    Result.FinalDamage = FMath::FloorToFloat(Result.FinalDamage);

    return Result;
}

FDamageCalculationResult UDamageCalculator::CalculateDamageWithSeed(const FDamageCalculationParams& Params, int32 RandomSeed)
{
    // 使用种子创建确定性的随机结果
    // 这里简化处理，实际联机需要更复杂的同步机制
    FMath::RandInit(RandomSeed);
    
    FDamageCalculationResult Result = CalculateDamage(Params);
    
    // 恢复随机状态
    FMath::RandInit(FMath::Rand());
    
    return Result;
}

float UDamageCalculator::CalculateBaseDamage(float BaseAttack, float SkillMultiplier)
{
    return BaseAttack * FMath::Max(0.0f, SkillMultiplier);
}

float UDamageCalculator::ApplyDefense(float BaseDamage, float Defense, bool bIgnoreDefense)
{
    if (bIgnoreDefense || Defense <= 0.0f)
    {
        return BaseDamage;
    }

    // 防御公式：伤害 = Max(基础伤害 × 0.1, 基础伤害 - 防御力)
    // 确保至少造成10%伤害，避免高防御完全免疫
    float MinDamage = BaseDamage * 0.1f;
    float ReducedDamage = BaseDamage - Defense;
    
    return FMath::Max(MinDamage, ReducedDamage);
}

float UDamageCalculator::ApplyResistance(float Damage, float Resistance, bool bIgnoreResistance)
{
    if (bIgnoreResistance || Resistance <= 0.0f)
    {
        return Damage;
    }

    // 抗性上限90%，避免完全免疫
    float ClampedResistance = FMath::Clamp(Resistance, 0.0f, 0.9f);
    
    // 抗性公式：伤害 × (1 - 抗性)
    return Damage * (1.0f - ClampedResistance);
}

float UDamageCalculator::ApplyCritical(float Damage, float CriticalRate, float CriticalDamage, bool bForceCritical, bool& OutIsCritical)
{
    OutIsCritical = bForceCritical || CheckCritical(CriticalRate);
    
    if (OutIsCritical)
    {
        return Damage * FMath::Max(1.0f, CriticalDamage);
    }
    
    return Damage;
}

float UDamageCalculator::ApplyVariance(float Damage, float Variance)
{
    if (Variance <= 0.0f)
    {
        return Damage;
    }

    float Multiplier = GetVarianceMultiplier(Variance);
    return Damage * Multiplier;
}

float UDamageCalculator::ApplyDamageBonus(float Damage, float FlatBonus, float PercentBonus)
{
    // 先应用百分比加成
    float AfterPercent = Damage * (1.0f + PercentBonus);
    
    // 再应用固定加成
    return AfterPercent + FlatBonus;
}

bool UDamageCalculator::CheckCritical(float CriticalRate)
{
    if (CriticalRate <= 0.0f)
    {
        return false;
    }

    // 暴击率上限100%
    float ClampedRate = FMath::Clamp(CriticalRate, 0.0f, 1.0f);
    float RandomValue = FMath::RandRange(0.0f, 1.0f);
    
    return RandomValue < ClampedRate;
}

float UDamageCalculator::GetVarianceMultiplier(float Variance)
{
    if (Variance <= 0.0f)
    {
        return 1.0f;
    }

    // 浮动范围限制在50%以内
    float ClampedVariance = FMath::Clamp(Variance, 0.0f, 0.5f);
    
    // 随机范围：1.0 ± Variance
    return FMath::RandRange(1.0f - ClampedVariance, 1.0f + ClampedVariance);
}

FDamageCalculationParams UDamageCalculator::CreateParamsFromWeaponStats(const FWeaponBaseStats& WeaponStats, EDamageType DamageType, float SkillMultiplier)
{
    FDamageCalculationParams Params;
    Params.BaseAttack = WeaponStats.BaseAttack;
    Params.SkillMultiplier = SkillMultiplier;
    Params.DamageType = DamageType;
    Params.CriticalRate = WeaponStats.CriticalRate;
    Params.CriticalDamage = WeaponStats.CriticalDamage;
    Params.DamageVariance = 0.1f; // 默认10%浮动
    
    return Params;
}

float UDamageCalculator::GetRandomFloat(int32& InOutSeed)
{
    // 简单的线性同余生成器（LCG）
    // 用于确定性的随机数（联机同步）
    InOutSeed = (InOutSeed * 1103515245 + 12345) & 0x7fffffff;
    return static_cast<float>(InOutSeed) / 0x7fffffff;
}
