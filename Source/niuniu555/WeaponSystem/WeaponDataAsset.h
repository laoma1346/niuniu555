// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponTypes.h"
#include "WeaponDataAsset.generated.h"

class USkeletalMesh;
class UAnimMontage;
class UParticleSystem;
class USoundBase;
class UCurveFloat;
class UTexture2D;

/**
 * 武器数据资产 - 用于在蓝图中配置武器数据
 * 数据与逻辑分离架构，设计师可以在蓝盘中独立配置武器
 */
UCLASS(Blueprintable, BlueprintType)
class NIUNIU555_API UWeaponDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UWeaponDataAsset();

    // ========== 基础信息 ==========

    // 武器唯一ID - 用于代码中识别武器
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponBasic")
    FName WeaponID;

    // 武器类型 - 决定武器分类
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponBasic")
    EWeaponType WeaponType;

    // 武器显示名称 - 游戏中显示的名字
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponBasic")
    FText WeaponName;

    // 武器描述 - 游戏中显示的描述文本
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponBasic")
    FText WeaponDescription;

    // 武器图标 - 用于UI显示
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponBasic")
    TObjectPtr<UTexture2D> WeaponIcon;

    // 武器品质/稀有度
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponBasic")
    EWeaponRarity Rarity;

    // ========== 模型资源 ==========

    // 武器骨骼网格体 - 武器的3D模型
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponMesh")
    TObjectPtr<USkeletalMesh> WeaponMesh;

    // 武器在角色身上的附加插槽名称 - 通常为"hand_r"或"WeaponSocket"
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponMesh")
    FName AttachSocketName;

    // 武器相对位置和旋转（相对于附加点）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponMesh")
    FTransform RelativeTransform;

    // ========== 基础属性 ==========

    // 基础攻击力 - 影响最终伤害
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponStats", meta = (ClampMin = 1))
    float BaseAttack;

    // 攻击速度 - 影响动画播放速度，1.0为正常速度
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponStats", meta = (ClampMin = 0.1))
    float AttackSpeed;

    // 暴击率 (0-1) - 暴击概率
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponStats", meta = (ClampMin = 0, ClampMax = 1))
    float CriticalRate;

    // 暴击伤害倍率 - 暴击时的伤害倍数
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponStats", meta = (ClampMin = 1))
    float CriticalDamage;

    // 攻击范围 - 攻击检测距离（厘米）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponStats", meta = (ClampMin = 1))
    float AttackRange;

    // 攻击角度 - 扇形检测角度（近战武器使用）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponStats", meta = (ClampMin = 1, ClampMax = 360))
    float AttackAngle;

    // 耐力消耗 - 每次攻击消耗的耐力
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponStats", meta = (ClampMin = 0))
    float StaminaCost;

    // 技能冷却时间 - 攻击间隔（秒）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponStats", meta = (ClampMin = 0))
    float CooldownTime;

    // ========== 轻击连招配置 ==========

    // 轻击连招动画蒙太奇 - 包含所有轻击段落的动画
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LightCombo")
    TObjectPtr<UAnimMontage> LightAttackMontage;

    // 轻击连招段落名称列表 - 对应动画蒙太奇中的Section名称
    // 例如：["LightAttack1", "LightAttack2", "LightAttack3"]
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LightCombo")
    TArray<FName> LightAttackSections;

    // 每段轻击的伤害倍率 - 与段落对应
    // 例如：[1.0, 1.2, 1.5] 表示第三段伤害最高
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LightCombo")
    TArray<float> LightAttackDamageMultipliers;

    // 每段轻击的击退力度
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LightCombo")
    TArray<float> LightAttackKnockbackForces;

    // 是否可以取消的段落索引列表 - 指定哪些段落可以Cancel
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LightCombo")
    TArray<int32> LightAttackCancellableStages;

    // Cancel窗口开始时间（相对于段落开始，秒）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LightCombo")
    float LightCancelWindowStart;

    // Cancel窗口结束时间（相对于段落开始，秒）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LightCombo")
    float LightCancelWindowEnd;

    // ========== 重击连招配置 ==========

    // 重击连招动画蒙太奇
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HeavyCombo")
    TObjectPtr<UAnimMontage> HeavyAttackMontage;

    // 重击连招段落名称列表
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HeavyCombo")
    TArray<FName> HeavyAttackSections;

    // 每段重击的伤害倍率 - 通常比重击高
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HeavyCombo")
    TArray<float> HeavyAttackDamageMultipliers;

    // 每段重击的击退力度
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HeavyCombo")
    TArray<float> HeavyAttackKnockbackForces;

    // 是否可以取消的段落索引列表
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HeavyCombo")
    TArray<int32> HeavyAttackCancellableStages;

    // Cancel窗口时间
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HeavyCombo")
    float HeavyCancelWindowStart;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HeavyCombo")
    float HeavyCancelWindowEnd;

    // ========== 蓄力攻击配置 ==========

    // 蓄力攻击动画蒙太奇 - 包含蓄力循环和攻击段落
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChargeAttack")
    TObjectPtr<UAnimMontage> ChargedAttackMontage;

    // 蓄力循环段落名称 - 按住蓄力键时循环播放的段落
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChargeAttack")
    FName ChargeLoopSection;

    // 蓄力攻击段落名称 - 释放蓄力键后播放的攻击段落
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChargeAttack")
    FName ChargeAttackSection;

    // 最大蓄力时间（秒）- 蓄满需要的时间
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChargeAttack", meta = (ClampMin = 0.1))
    float MaxChargeTime;

    // 蓄力伤害倍率曲线 - X轴为蓄力比例(0-1)，Y轴为伤害倍率
    // 不设置则使用线性增长
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChargeAttack")
    TObjectPtr<UCurveFloat> ChargeDamageCurve;

    // ========== 特效资源 ==========

    // 普通攻击命中特效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TObjectPtr<UParticleSystem> NormalHitEffect;

    // 暴击命中特效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TObjectPtr<UParticleSystem> CriticalHitEffect;

    // 武器轨迹特效 - 攻击时的刀光等
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TObjectPtr<UParticleSystem> TrailEffect;

    // 蓄力特效 - 蓄力时的特效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TObjectPtr<UParticleSystem> ChargeEffect;

    // ========== 音效资源 ==========

    // 普通攻击音效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
    TObjectPtr<USoundBase> AttackSound;

    // 攻击命中音效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
    TObjectPtr<USoundBase> HitSound;

    // 暴击音效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
    TObjectPtr<USoundBase> CriticalSound;

    // 武器切换音效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
    TObjectPtr<USoundBase> EquipSound;

    // ========== 碰撞检测配置 ==========

    // 攻击检测碰撞通道 - 在项目设置中配置
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    TEnumAsByte<ECollisionChannel> AttackTraceChannel;

    // 攻击检测起始骨骼名称 - 在武器骨骼网格体中定义的Socket
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    FName AttackStartSocket;

    // 攻击检测结束骨骼名称
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    FName AttackEndSocket;

    // 攻击检测半径 - 胶囊体检测半径（厘米）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision", meta = (ClampMin = 1))
    float AttackTraceRadius;

    // ========== 特殊效果（可选） ==========

    // 是否带有元素伤害
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special")
    bool bHasElementalDamage;

    // 元素伤害类型
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special", meta = (EditCondition = "bHasElementalDamage"))
    EDamageType ElementalDamageType;

    // 元素伤害值
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special", meta = (EditCondition = "bHasElementalDamage"))
    float ElementalDamageValue;

public:
    // 获取轻击连招阶段数
    UFUNCTION(BlueprintPure, Category = "WeaponData")
    int32 GetLightComboStageCount() const { return LightAttackSections.Num(); }

    // 获取重击连招阶段数
    UFUNCTION(BlueprintPure, Category = "WeaponData")
    int32 GetHeavyComboStageCount() const { return HeavyAttackSections.Num(); }

    // 获取指定轻击段落的伤害倍率
    UFUNCTION(BlueprintPure, Category = "WeaponData")
    float GetLightDamageMultiplier(int32 StageIndex) const;

    // 获取指定重击段落的伤害倍率
    UFUNCTION(BlueprintPure, Category = "WeaponData")
    float GetHeavyDamageMultiplier(int32 StageIndex) const;

    // 检查指定轻击段落是否可以Cancel
    UFUNCTION(BlueprintPure, Category = "WeaponData")
    bool IsLightStageCancellable(int32 StageIndex) const;

    // 检查指定重击段落是否可以Cancel
    UFUNCTION(BlueprintPure, Category = "WeaponData")
    bool IsHeavyStageCancellable(int32 StageIndex) const;

    // 获取蓄力伤害倍率（根据蓄力比例）
    UFUNCTION(BlueprintPure, Category = "WeaponData")
    float GetChargeDamageMultiplier(float ChargeRatio) const;

    // 转换为武器基础属性结构
    UFUNCTION(BlueprintPure, Category = "WeaponData")
    FWeaponBaseStats ToWeaponBaseStats() const;
};
