// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIStateTypes.generated.h"

/** AI行为状态枚举 - 融合蓝图AI系统 */
UENUM(BlueprintType)
enum class EAIBehaviorState : uint8
{
    None            UMETA(DisplayName = "无"),
    Idle            UMETA(DisplayName = "待机"),
    Patrol          UMETA(DisplayName = "巡逻"),
    Investigate     UMETA(DisplayName = "侦查"),      // 前往侦查点
    Chase           UMETA(DisplayName = "追击"),
    Attack          UMETA(DisplayName = "攻击"),
    Search          UMETA(DisplayName = "搜索"),      // 搜索目标
    Return          UMETA(DisplayName = "返回"),
    Dead            UMETA(DisplayName = "死亡")
};

/** 攻击类型枚举 */
UENUM(BlueprintType)
enum class EAIAttackType : uint8
{
    None            UMETA(DisplayName = "无"),
    Melee           UMETA(DisplayName = "近战"),
    Ranged          UMETA(DisplayName = "远程"),
    Skill           UMETA(DisplayName = "技能")
};

/** AI巡逻类型 */
UENUM(BlueprintType)
enum class EAIPatrolType : uint8
{
    RandomPoint     UMETA(DisplayName = "随机点"),     // 原有：随机点巡逻
    SplinePath      UMETA(DisplayName = "样条路径"),   // 新增：样条路径巡逻
    FixedPoints     UMETA(DisplayName = "固定点")      // 固定巡逻点
};
