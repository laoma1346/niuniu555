// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyDataAsset.h"

UEnemyDataAsset::UEnemyDataAsset()
{
    // ========== 基础信息默认值 ==========
    EnemyID = NAME_None;
    EnemyType = EEnemyType::Normal;
    Faction = EEnemyFaction::Hostile;
    EnemySize = EEnemySize::Medium;
    BaseLevel = 1;

    // ========== 基础属性默认值 ==========
    BaseHealth = 100.0f;
    BaseAttack = 10.0f;
    BaseDefense = 5.0f;
    BaseMoveSpeed = 300.0f;
    BaseRotationSpeed = 360.0f;
    AttackSpeedMultiplier = 1.0f;
    CriticalRate = 0.05f;
    CriticalDamage = 1.5f;

    // 属性成长默认配置
    StatGrowth.BaseHealth = 100.0f;
    StatGrowth.HealthPerChapter = 50.0f;
    StatGrowth.BaseAttack = 10.0f;
    StatGrowth.AttackPerChapter = 5.0f;
    StatGrowth.BaseDefense = 5.0f;
    StatGrowth.DefensePerChapter = 2.0f;

    // ========== AI配置默认值 ==========
    PatrolRadius = 500.0f;
    ChaseRadius = 2000.0f;
    AttackRange = 150.0f;
    MinAttackInterval = 1.0f;
    MaxAttackInterval = 2.0f;

    // 感知配置默认值
    SenseConfig.SenseTypes = static_cast<int32>(EAISenseType::Sight) | static_cast<int32>(EAISenseType::Hearing) | static_cast<int32>(EAISenseType::Damage);
    SenseConfig.SightRadius = 1500.0f;
    SenseConfig.SightAngle = 120.0f;
    SenseConfig.HearingRadius = 800.0f;
    SenseConfig.MemoryDuration = 5.0f;

    // ========== 攻击配置默认值 ==========
    AttackType = EEnemyAttackType::Melee;
    AttackWindUpTime = 0.3f;
    AttackRecoveryTime = 0.5f;
    SkillCooldown = 10.0f;

    // ========== 受击配置默认值 ==========
    HitStunDuration = 0.3f;
    bCanBeKnockedBack = true;
    KnockbackResistance = 0.0f;
    bCanBeStunned = true;
    StunImmunityDuration = 3.0f;

    // ========== 死亡配置默认值 ==========
    CorpseLifeTime = 5.0f;
    bUseRagdoll = true;
    RagdollDuration = 3.0f;

    // ========== 掉落配置默认值 ==========
    ExperienceReward = 10;
    BaseGoldDrop = 5;
    GoldDropVariance = 0.2f;
}

float UEnemyDataAsset::GetHealthForChapter(int32 Chapter) const
{
    // 确保章节至少为1
    int32 ValidChapter = FMath::Max(1, Chapter);
    
    // 基础值 + 成长值
    float GrowthHealth = StatGrowth.CalculateHealth(ValidChapter);
    
    // 使用配置的成长公式，或者基础值（取较大值）
    return FMath::Max(BaseHealth + StatGrowth.HealthPerChapter * (ValidChapter - 1), GrowthHealth);
}

float UEnemyDataAsset::GetAttackForChapter(int32 Chapter) const
{
    int32 ValidChapter = FMath::Max(1, Chapter);
    float GrowthAttack = StatGrowth.CalculateAttack(ValidChapter);
    return FMath::Max(BaseAttack + StatGrowth.AttackPerChapter * (ValidChapter - 1), GrowthAttack);
}

float UEnemyDataAsset::GetDefenseForChapter(int32 Chapter) const
{
    int32 ValidChapter = FMath::Max(1, Chapter);
    float GrowthDefense = StatGrowth.CalculateDefense(ValidChapter);
    return FMath::Max(BaseDefense + StatGrowth.DefensePerChapter * (ValidChapter - 1), GrowthDefense);
}

float UEnemyDataAsset::GetRandomAttackInterval() const
{
    // 在最小和最大间隔之间随机
    return FMath::FRandRange(MinAttackInterval, MaxAttackInterval);
}

int32 UEnemyDataAsset::CalculateGoldDrop() const
{
    // 基础金币 + 浮动
    float Variance = FMath::FRandRange(-GoldDropVariance, GoldDropVariance);
    float FinalGold = static_cast<float>(BaseGoldDrop) * (1.0f + Variance);
    
    // 返回整数，至少为1
    return FMath::Max(1, FMath::RoundToInt(FinalGold));
}

float UEnemyDataAsset::GetSizeScale() const
{
    // 返回尺寸对应的缩放比例
    switch (EnemySize)
    {
        case EEnemySize::Tiny:
            return 0.5f;
        case EEnemySize::Small:
            return 0.75f;
        case EEnemySize::Medium:
            return 1.0f;
        case EEnemySize::Large:
            return 1.5f;
        case EEnemySize::Huge:
            return 2.0f;
        case EEnemySize::Massive:
            return 3.0f;
        default:
            return 1.0f;
    }
}

bool UEnemyDataAsset::IsValidData() const
{
    // 检查必需的数据是否配置
    bool bHasValidID = !EnemyID.IsNone();
    bool bHasValidName = !EnemyName.IsEmpty();
    bool bHasValidStats = BaseHealth > 0 && BaseAttack > 0;
    bool bHasValidMesh = SkeletalMesh != nullptr;
    
    return bHasValidID && bHasValidName && bHasValidStats && bHasValidMesh;
}
