// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnemyTypes.generated.h"

// ==================== 敌人系统基础类型定义 ====================
// 文件路径: Source/niuniu555/EnemySystem/EnemyTypes.h
// 功能说明: 定义敌人的类型、状态、阵营等基础枚举和数据结构
// 作者: AI Assistant
// 创建日期: 2026-03-07

/**
 * 敌人类型枚举
 * 用于区分不同类型的敌人，影响AI行为、掉落物等
 */
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
    None        UMETA(DisplayName = "无"),
    Normal      UMETA(DisplayName = "普通敌人"),      // 普通小怪
    Elite       UMETA(DisplayName = "精英敌人"),      // 精英怪，属性强化
    Boss        UMETA(DisplayName = "BOSS"),          // 首领敌人
    Summoned    UMETA(DisplayName = "召唤物"),        // 被召唤的临时敌人
    Environmental UMETA(DisplayName = "环境危险")     // 陷阱、机关等
};

/**
 * 敌人状态枚举
 * 敌人状态机的核心状态定义
 */
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    None        UMETA(DisplayName = "无"),
    Spawn       UMETA(DisplayName = "出生"),          // 出生/生成中（播放生成动画）
    Idle        UMETA(DisplayName = "待机"),          // 待机巡逻
    Patrol      UMETA(DisplayName = "巡逻"),          // 主动巡逻中
    Chase       UMETA(DisplayName = "追击"),          // 追击玩家
    Attack      UMETA(DisplayName = "攻击"),          // 攻击中
    Skill       UMETA(DisplayName = "施法"),          // 释放技能中
    Hit         UMETA(DisplayName = "受击"),          // 受击硬直
    Stun        UMETA(DisplayName = "眩晕"),          // 被控制/眩晕
    Return      UMETA(DisplayName = "返回"),          // 超出范围返回出生点
    Dead        UMETA(DisplayName = "死亡")           // 死亡
};

/**
 * 敌人阵营枚举
 * 用于区分敌我关系，支持未来可能的友方NPC或阵营战
 */
UENUM(BlueprintType)
enum class EEnemyFaction : uint8
{
    Neutral     UMETA(DisplayName = "中立"),          // 中立，不主动攻击
    Hostile     UMETA(DisplayName = "敌对"),          // 敌对玩家
    Friendly    UMETA(DisplayName = "友方"),          // 友方NPC
    Wild        UMETA(DisplayName = "野生"),          // 野生生物，可能攻击任何阵营
    Player      UMETA(DisplayName = "玩家")           // 玩家阵营
};

/**
 * AI感知类型枚举
 * 定义AI如何发现目标（支持位标志组合）
 */
UENUM(BlueprintType, meta = (Bitflags))
enum class EAISenseType : uint8
{
    None        = 0 UMETA(Hidden),
    Sight       = 1 << 0 UMETA(DisplayName = "视觉"),     // 视觉检测（锥形视野）
    Hearing     = 1 << 1 UMETA(DisplayName = "听觉"),     // 听觉检测（半径范围）
    Damage      = 1 << 2 UMETA(DisplayName = "伤害"),     // 受到伤害感知
    Team        = 1 << 3 UMETA(DisplayName = "队友"),     // 队友通讯
    Prediction  = 1 << 4 UMETA(DisplayName = "预测")      // 预测玩家位置
};
ENUM_CLASS_FLAGS(EAISenseType)

/**
 * AI行为优先级枚举
 * 用于行为树中的优先级判断
 */
UENUM(BlueprintType)
enum class EAIBehaviorPriority : uint8
{
    Lowest      UMETA(DisplayName = "最低"),
    Low         UMETA(DisplayName = "低"),
    Normal      UMETA(DisplayName = "普通"),
    High        UMETA(DisplayName = "高"),
    Critical    UMETA(DisplayName = "紧急"),          // 如：濒死逃跑
    Override    UMETA(DisplayName = "覆盖")           // 强制行为，如被控制
};

/**
 * 攻击类型枚举
 * 定义敌人的攻击方式
 */
UENUM(BlueprintType)
enum class EEnemyAttackType : uint8
{
    None        UMETA(DisplayName = "无"),
    Melee       UMETA(DisplayName = "近战"),          // 近战攻击
    Ranged      UMETA(DisplayName = "远程"),          // 远程投射
    AOE         UMETA(DisplayName = "范围"),          // 范围攻击
    Charge      UMETA(DisplayName = "冲锋"),          // 冲锋攻击
    Summon      UMETA(DisplayName = "召唤"),          // 召唤其他敌人
    Buff        UMETA(DisplayName = "增益"),          // 给自己/友方加BUFF
    Special     UMETA(DisplayName = "特殊")           // 特殊机制攻击
};

/**
 * 敌人尺寸类别
 * 影响受击框、碰撞体积等
 */
UENUM(BlueprintType)
enum class EEnemySize : uint8
{
    Tiny        UMETA(DisplayName = "微型"),          // 如：虫子、小鬼
    Small       UMETA(DisplayName = "小型"),          // 如：哥布林
    Medium      UMETA(DisplayName = "中型"),          // 标准人类尺寸
    Large       UMETA(DisplayName = "大型"),          // 如：巨魔
    Huge        UMETA(DisplayName = "巨型"),          // 如：BOSS
    Massive     UMETA(DisplayName = "超巨型")         // 如：世界BOSS
};

// ==================== 数据结构定义 ====================

/**
 * 敌人生成配置结构体
 * 用于配置单个敌人的生成参数
 */
USTRUCT(BlueprintType)
struct FEnemySpawnConfig
{
    GENERATED_BODY()

    // 敌人类型（关联到数据资产）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
    TSubclassOf<class ACharacter> EnemyClass;

    // 生成位置（相对于生成点）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
    FVector SpawnOffset;

    // 生成延迟（用于波次错开生成）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
    float SpawnDelay;

    // 是否立即激活
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
    bool bAutoActivate;

    // 生成后是否巡逻
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
    bool bStartPatrol;

    FEnemySpawnConfig()
        : SpawnOffset(FVector::ZeroVector)
        , SpawnDelay(0.0f)
        , bAutoActivate(true)
        , bStartPatrol(false)
    {}
};

/**
 * AI感知配置结构体
 * 配置AI的视觉、听觉等感知参数
 */
USTRUCT(BlueprintType)
struct FEnemySenseConfig
{
    GENERATED_BODY()

    // 感知类型组合（使用Bitflags）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "感知配置", meta = (Bitmask, BitmaskEnum = "EAISenseType"))
    int32 SenseTypes;

    // 视觉半径
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "感知配置")
    float SightRadius;

    // 视觉角度（锥形视野角度）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "感知配置")
    float SightAngle;

    // 听觉半径
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "感知配置")
    float HearingRadius;

    // 失去目标后的记忆时间（秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "感知配置")
    float MemoryDuration;

    FEnemySenseConfig()
        : SenseTypes(0)
        , SightRadius(1000.0f)
        , SightAngle(90.0f)
        , HearingRadius(500.0f)
        , MemoryDuration(5.0f)
    {}
};

/**
 * 敌人属性增长曲线
 * 用于根据章节/等级计算敌人属性
 */
USTRUCT(BlueprintType)
struct FEnemyStatGrowth
{
    GENERATED_BODY()

    // 基础生命值
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "属性成长")
    float BaseHealth;

    // 每章生命值增长
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "属性成长")
    float HealthPerChapter;

    // 基础攻击力
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "属性成长")
    float BaseAttack;

    // 每章攻击力增长
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "属性成长")
    float AttackPerChapter;

    // 基础防御力
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "属性成长")
    float BaseDefense;

    // 每章防御力增长
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "属性成长")
    float DefensePerChapter;

    FEnemyStatGrowth()
        : BaseHealth(100.0f)
        , HealthPerChapter(50.0f)
        , BaseAttack(10.0f)
        , AttackPerChapter(5.0f)
        , BaseDefense(5.0f)
        , DefensePerChapter(2.0f)
    {}

    // 根据章节计算属性
    float CalculateHealth(int32 Chapter) const { return BaseHealth + HealthPerChapter * (Chapter - 1); }
    float CalculateAttack(int32 Chapter) const { return BaseAttack + AttackPerChapter * (Chapter - 1); }
    float CalculateDefense(int32 Chapter) const { return BaseDefense + DefensePerChapter * (Chapter - 1); }
};

/**
 * 敌人状态变更委托（动态多播）
 * 用于通知状态机状态变化
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyStateChanged, EEnemyState, OldState, EEnemyState, NewState);

/**
 * 敌人受击委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyTakeDamage, const struct FHitInfo&, HitInfo);

/**
 * 敌人死亡委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDeath);
