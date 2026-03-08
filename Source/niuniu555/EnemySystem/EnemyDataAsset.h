// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyTypes.h"
#include "EnemyDataAsset.generated.h"

class USkeletalMesh;
class UAnimBlueprint;
class UAnimMontage;
class UParticleSystem;
class USoundBase;
class UBehaviorTree;
class UBlackboardData;
// 使用传统粒子系统（与武器系统保持一致）
// 如需使用Niagara，请在build.cs中添加Niagara模块依赖

// 前置声明掉落表数据资产
class UDropTableDataAsset;

/**
 * 敌人数据资产 - 用于在蓝图中配置敌人数据
 * 数据与逻辑分离架构，设计师可以在蓝图中独立配置敌人
 * 文件路径: Source/niuniu555/EnemySystem/EnemyDataAsset.h
 */
UCLASS(Blueprintable, BlueprintType)
class NIUNIU555_API UEnemyDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UEnemyDataAsset();

    // ========== 基础信息 ==========

    // 敌人唯一ID - 用于代码中识别敌人类型
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "敌人基础")
    FName EnemyID;

    // 敌人显示名称 - 游戏中显示的名字
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "敌人基础")
    FText EnemyName;

    // 敌人描述 - 游戏中显示的描述文本（图鉴用）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "敌人基础")
    FText EnemyDescription;

    // 敌人类型 - 决定AI行为和掉落
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "敌人基础")
    EEnemyType EnemyType;

    // 敌人阵营 - 默认为敌对
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "敌人基础")
    EEnemyFaction Faction;

    // 敌人尺寸 - 影响碰撞和受击框
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "敌人基础")
    EEnemySize EnemySize;

    // 敌人等级 - 基础等级，实际等级会根据章节调整
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "敌人基础", meta = (ClampMin = 1))
    int32 BaseLevel;

    // 敌人图标 - 用于小地图/锁定UI
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "敌人基础")
    TObjectPtr<UTexture2D> EnemyIcon;

    // ========== 模型资源 ==========

    // 敌人骨骼网格体 - 3D模型
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "视觉资源")
    TObjectPtr<USkeletalMesh> SkeletalMesh;

    // 动画蓝图 - 控制敌人动画状态机
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "视觉资源")
    TSubclassOf<UAnimInstance> AnimBlueprintClass;

    // 默认材质 - 用于受击闪白后恢复
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "视觉资源")
    TObjectPtr<UMaterialInterface> DefaultMaterial;

    // 受击材质 - 受击时的闪白材质（可选，不设置则动态创建）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "视觉资源")
    TObjectPtr<UMaterialInterface> HitFlashMaterial;

    // ========== 基础属性（会根据章节成长）==========

    // 基础生命值
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "基础属性", meta = (ClampMin = 1))
    float BaseHealth;

    // 基础攻击力
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "基础属性", meta = (ClampMin = 1))
    float BaseAttack;

    // 基础防御力
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "基础属性", meta = (ClampMin = 0))
    float BaseDefense;

    // 基础移动速度（厘米/秒）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "基础属性", meta = (ClampMin = 1))
    float BaseMoveSpeed;

    // 基础旋转速度（度/秒）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "基础属性", meta = (ClampMin = 1))
    float BaseRotationSpeed;

    // 攻击速度倍率 - 影响攻击动画播放速度
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "基础属性", meta = (ClampMin = 0.1))
    float AttackSpeedMultiplier;

    // 暴击率 (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "基础属性", meta = (ClampMin = 0, ClampMax = 1))
    float CriticalRate;

    // 暴击伤害倍率
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "基础属性", meta = (ClampMin = 1))
    float CriticalDamage;

    // 属性成长配置 - 每章属性增长
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "基础属性")
    FEnemyStatGrowth StatGrowth;

    // ========== AI配置 ==========

    // 行为树 - 控制AI决策
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI配置")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    // 黑板数据 - AI内存数据
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI配置")
    TObjectPtr<UBlackboardData> BlackboardData;

    // 感知配置
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI配置")
    FEnemySenseConfig SenseConfig;

    // 巡逻半径 - 出生点周围的巡逻范围
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI配置", meta = (ClampMin = 0))
    float PatrolRadius;

    // 追击半径 - 超出此范围停止追击并返回
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI配置", meta = (ClampMin = 1))
    float ChaseRadius;

    // 攻击范围 - 近战/远程攻击的起始距离
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI配置", meta = (ClampMin = 1))
    float AttackRange;

    // 最小攻击间隔（秒）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI配置", meta = (ClampMin = 0.1))
    float MinAttackInterval;

    // 最大攻击间隔（秒）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI配置", meta = (ClampMin = 0.1))
    float MaxAttackInterval;

    // ========== 攻击配置 ==========

    // 攻击类型
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "攻击配置")
    EEnemyAttackType AttackType;

    // 攻击动画蒙太奇
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "攻击配置")
    TObjectPtr<UAnimMontage> AttackMontage;

    // 技能动画蒙太奇（如果有特殊技能）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "攻击配置")
    TObjectPtr<UAnimMontage> SkillMontage;

    // 攻击前摇时间（秒）- 攻击动画开始后多久产生伤害判定
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "攻击配置", meta = (ClampMin = 0))
    float AttackWindUpTime;

    // 攻击后摇时间（秒）- 攻击判定后多久可以移动
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "攻击配置", meta = (ClampMin = 0))
    float AttackRecoveryTime;

    // 技能冷却时间（秒）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "攻击配置", meta = (ClampMin = 0))
    float SkillCooldown;

    // ========== 受击配置 ==========

    // 受击动画蒙太奇
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "受击配置")
    TObjectPtr<UAnimMontage> HitReactionMontage;

    // 受击硬直时间（秒）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "受击配置", meta = (ClampMin = 0))
    float HitStunDuration;

    // 是否可被击退
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "受击配置")
    bool bCanBeKnockedBack;

    // 击退抗性 (0-1) - 1表示完全免疫击退
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "受击配置", meta = (ClampMin = 0, ClampMax = 1))
    float KnockbackResistance;

    // 是否可被眩晕
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "受击配置")
    bool bCanBeStunned;

    // 眩晕免疫时间（秒）- 被眩晕后的一段时间内免疫
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "受击配置", meta = (ClampMin = 0))
    float StunImmunityDuration;

    // ========== 死亡配置 ==========

    // 死亡动画蒙太奇
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "死亡配置")
    TObjectPtr<UAnimMontage> DeathMontage;

    // 尸体存在时间（秒）- 0表示立即消失
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "死亡配置", meta = (ClampMin = 0))
    float CorpseLifeTime;

    // 是否启用布娃娃物理
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "死亡配置")
    bool bUseRagdoll;

    // 布娃娃持续时间（秒）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "死亡配置", meta = (ClampMin = 0))
    float RagdollDuration;

    // ========== 掉落配置 ==========

    // 掉落表数据资产 - 定义掉落物品和概率
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落配置")
    TObjectPtr<UDropTableDataAsset> DropTable;

    // 固定经验值奖励
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落配置", meta = (ClampMin = 0))
    int32 ExperienceReward;

    // 基础金币掉落
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落配置", meta = (ClampMin = 0))
    int32 BaseGoldDrop;

    // 金币掉落浮动范围 (0-1) - 0.2表示±20%浮动
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落配置", meta = (ClampMin = 0, ClampMax = 1))
    float GoldDropVariance;

    // 金币掉落物类 - 需要在蓝图中配置BP_GoldDropItem
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落配置")
    TSubclassOf<class AGoldDropItem> GoldDropClass;

    // ========== 神格碎片掉落配置 ==========

    // 神格碎片掉落物类
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "神格碎片掉落")
    TSubclassOf<class ADivineFragmentDropItem> DivineFragmentClass;

    // BOSS必定掉落碎片数量
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "神格碎片掉落", meta = (ClampMin = 0))
    int32 BossFragmentDropCount = 5;

    // 精英敌人掉落概率 (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "神格碎片掉落", meta = (ClampMin = 0, ClampMax = 1))
    float EliteFragmentDropChance = 0.5f;

    // 精英掉落碎片数量范围
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "神格碎片掉落", meta = (ClampMin = 0))
    FIntPoint EliteFragmentCountRange = FIntPoint(1, 3);

    // 普通敌人掉落概率 (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "神格碎片掉落", meta = (ClampMin = 0, ClampMax = 1))
    float NormalFragmentDropChance = 0.1f;

    // 普通掉落碎片数量范围
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "神格碎片掉落", meta = (ClampMin = 0))
    FIntPoint NormalFragmentCountRange = FIntPoint(1, 1);

    // 碎片类型权重 - 决定掉落哪种类型的碎片
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "神格碎片掉落")
    TMap<EDivineFragmentType, int32> FragmentTypeWeights;

    // ========== 特效配置 ==========

    // 出生特效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "特效配置")
    TObjectPtr<UParticleSystem> SpawnEffect;

    // 受击特效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "特效配置")
    TObjectPtr<UParticleSystem> HitEffect;

    // 死亡特效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "特效配置")
    TObjectPtr<UParticleSystem> DeathEffect;

    // 死亡特效生命周期（秒）- 0表示使用粒子系统默认生命周期
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "特效配置", meta = (ClampMin = 0))
    float DeathEffectLifeTime = 3.0f;

    // 攻击预警特效（BOSS/精英用）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "特效配置")
    TObjectPtr<UParticleSystem> AttackWarningEffect;

    // ========== 音效配置 ==========

    // 出生音效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "音效配置")
    TObjectPtr<USoundBase> SpawnSound;

    // 受击音效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "音效配置")
    TObjectPtr<USoundBase> HitSound;

    // 死亡音效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "音效配置")
    TObjectPtr<USoundBase> DeathSound;

    // 攻击音效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "音效配置")
    TObjectPtr<USoundBase> AttackSound;

    // 警觉音效（发现玩家时）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "音效配置")
    TObjectPtr<USoundBase> AlertSound;

public:
    // ========== 蓝图辅助函数 ==========

    // 根据章节计算实际生命值
    UFUNCTION(BlueprintPure, Category = "敌人数据")
    float GetHealthForChapter(int32 Chapter) const;

    // 根据章节计算实际攻击力
    UFUNCTION(BlueprintPure, Category = "敌人数据")
    float GetAttackForChapter(int32 Chapter) const;

    // 根据章节计算实际防御力
    UFUNCTION(BlueprintPure, Category = "敌人数据")
    float GetDefenseForChapter(int32 Chapter) const;

    // 获取随机攻击间隔
    UFUNCTION(BlueprintPure, Category = "敌人数据")
    float GetRandomAttackInterval() const;

    // 计算金币掉落数量（含浮动）
    UFUNCTION(BlueprintPure, Category = "敌人数据")
    int32 CalculateGoldDrop() const;

    // 检查是否是BOSS类型
    UFUNCTION(BlueprintPure, Category = "敌人数据")
    bool IsBoss() const { return EnemyType == EEnemyType::Boss; }

    // 检查是否是精英类型
    UFUNCTION(BlueprintPure, Category = "敌人数据")
    bool IsElite() const { return EnemyType == EEnemyType::Elite; }

    // 检查是否是普通敌人
    UFUNCTION(BlueprintPure, Category = "敌人数据")
    bool IsNormal() const { return EnemyType == EEnemyType::Normal; }

    // 获取尺寸对应的缩放比例
    UFUNCTION(BlueprintPure, Category = "敌人数据")
    float GetSizeScale() const;

    // 验证数据资产是否有效
    UFUNCTION(BlueprintPure, Category = "敌人数据")
    bool IsValidData() const;
};
