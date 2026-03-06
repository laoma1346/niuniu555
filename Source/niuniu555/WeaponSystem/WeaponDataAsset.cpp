// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponDataAsset.h"

UWeaponDataAsset::UWeaponDataAsset()
{
    // 设置默认值
    WeaponID = NAME_None;
    WeaponType = EWeaponType::None;
    Rarity = EWeaponRarity::Common;
    
    // 附加点默认值
    AttachSocketName = FName("hand_r");
    RelativeTransform = FTransform::Identity;
    
    // 属性默认值
    BaseAttack = 10.0f;
    AttackSpeed = 1.0f;
    CriticalRate = 0.05f;
    CriticalDamage = 1.5f;
    AttackRange = 150.0f;
    AttackAngle = 60.0f;
    StaminaCost = 10.0f;
    CooldownTime = 0.5f;
    
    // 连招默认配置
    LightCancelWindowStart = 0.3f;
    LightCancelWindowEnd = 0.5f;
    HeavyCancelWindowStart = 0.4f;
    HeavyCancelWindowEnd = 0.6f;
    
    // 蓄力默认配置
    ChargeLoopSection = FName("ChargeLoop");
    ChargeAttackSection = FName("ChargeAttack");
    MaxChargeTime = 2.0f;
    
    // 碰撞检测默认配置
    AttackTraceChannel = ECC_Pawn;
    AttackStartSocket = FName("weapon_start");
    AttackEndSocket = FName("weapon_end");
    AttackTraceRadius = 30.0f;
    
    // 特殊效果默认关闭
    bHasElementalDamage = false;
    ElementalDamageType = EDamageType::Physical;
    ElementalDamageValue = 0.0f;
}

float UWeaponDataAsset::GetLightDamageMultiplier(int32 StageIndex) const
{
    if (LightAttackDamageMultipliers.IsValidIndex(StageIndex))
    {
        return LightAttackDamageMultipliers[StageIndex];
    }
    // 默认返回1.0
    return 1.0f;
}

float UWeaponDataAsset::GetHeavyDamageMultiplier(int32 StageIndex) const
{
    if (HeavyAttackDamageMultipliers.IsValidIndex(StageIndex))
    {
        return HeavyAttackDamageMultipliers[StageIndex];
    }
    // 默认返回1.5（重击基础伤害更高）
    return 1.5f;
}

bool UWeaponDataAsset::IsLightStageCancellable(int32 StageIndex) const
{
    return LightAttackCancellableStages.Contains(StageIndex);
}

bool UWeaponDataAsset::IsHeavyStageCancellable(int32 StageIndex) const
{
    return HeavyAttackCancellableStages.Contains(StageIndex);
}

float UWeaponDataAsset::GetChargeDamageMultiplier(float ChargeRatio) const
{
    // 将蓄力比例限制在0-1范围内
    float ClampedRatio = FMath::Clamp(ChargeRatio, 0.0f, 1.0f);
    
    // 如果有配置曲线，使用曲线值
    if (ChargeDamageCurve != nullptr)
    {
        return ChargeDamageCurve->GetFloatValue(ClampedRatio);
    }
    
    // 默认线性增长：基础1.0倍，满蓄力2.0倍
    return 1.0f + ClampedRatio;
}

FWeaponBaseStats UWeaponDataAsset::ToWeaponBaseStats() const
{
    FWeaponBaseStats Stats;
    Stats.BaseAttack = BaseAttack;
    Stats.AttackSpeed = AttackSpeed;
    Stats.CriticalRate = CriticalRate;
    Stats.CriticalDamage = CriticalDamage;
    Stats.AttackRange = AttackRange;
    Stats.AttackAngle = AttackAngle;
    Stats.StaminaCost = StaminaCost;
    Stats.CooldownTime = CooldownTime;
    return Stats;
}
