// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ComboTypes.generated.h"

// ==================== 连招输入类型 ====================

/**
 * 连招输入类型 - 轻击/重击
 * 对应武器攻击输入
 */
UENUM(BlueprintType)
enum class EWeaponComboInputType : uint8
{
    None    UMETA(DisplayName = "无"),
    Light   UMETA(DisplayName = "轻击 - L"),
    Heavy   UMETA(DisplayName = "重击 - H")
};

// ==================== 连招阶段信息 ====================

/**
 * 连招阶段配置
 * 定义每个连招阶段的具体参数
 */
USTRUCT(BlueprintType)
struct FWeaponComboStageConfig
{
    GENERATED_BODY()

    // 阶段索引（从0开始）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招阶段")
    int32 StageIndex;

    // 阶段名称（用于调试和日志）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招阶段")
    FString StageName;

    // 此阶段对应的动画蒙太奇Section名称
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招阶段")
    FName AnimationSectionName;

    // 此阶段的伤害倍率（相对于基础伤害）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招阶段|伤害", meta = (ClampMin = 0.1))
    float DamageMultiplier;

    // 此阶段的攻击范围倍率
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招阶段|攻击", meta = (ClampMin = 0.1))
    float RangeMultiplier;

    // 此阶段的击退力度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招阶段|攻击")
    float KnockbackForce;

    // 此阶段是否触发特殊效果（派生攻击）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招阶段|特效")
    bool bTriggersSpecialEffect;

    // 特殊效果ID（用于在蓝图中识别）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招阶段|特效", meta = (EditCondition = "bTriggersSpecialEffect"))
    FName SpecialEffectID;

    // 是否可以取消（Cancel Window）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招阶段|取消窗口")
    bool bCanCancel;

    // 取消窗口开始时间（相对于动画开始，秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招阶段|取消窗口", meta = (EditCondition = "bCanCancel", ClampMin = 0))
    float CancelWindowStart;

    // 取消窗口结束时间（相对于动画开始，秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招阶段|取消窗口", meta = (EditCondition = "bCanCancel", ClampMin = 0))
    float CancelWindowEnd;

    // 到下一阶段的输入容忍时间（秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招阶段|输入", meta = (ClampMin = 0.1))
    float InputToleranceTime;

    FWeaponComboStageConfig()
        : StageIndex(0)
        , StageName(TEXT("阶段0"))
        , AnimationSectionName(NAME_None)
        , DamageMultiplier(1.0f)
        , RangeMultiplier(1.0f)
        , KnockbackForce(300.0f)
        , bTriggersSpecialEffect(false)
        , SpecialEffectID(NAME_None)
        , bCanCancel(true)
        , CancelWindowStart(0.3f)
        , CancelWindowEnd(0.5f)
        , InputToleranceTime(0.5f)
    {}
};

// ==================== 连招序列定义 ====================

/**
 * 连招序列配置
 * 定义一个完整的连招（如 LLH, LHH 等）
 */
USTRUCT(BlueprintType)
struct FWeaponComboSequence
{
    GENERATED_BODY()

    // 连招ID（唯一标识）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招基础")
    FName ComboID;

    // 连招显示名称
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招基础")
    FText ComboName;

    // 连招描述
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招基础")
    FText Description;

    // 输入序列（如 [L, L, H]）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招基础")
    TArray<EWeaponComboInputType> InputSequence;

    // 各阶段配置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招阶段")
    TArray<FWeaponComboStageConfig> StageConfigs;

    // 连招总时长限制（从第一击开始计算，秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招基础", meta = (ClampMin = 0.5))
    float MaxTotalDuration;

    // 连招完成后是否重置（false则保持最后一击可继续派生）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招基础")
    bool bResetAfterComplete;

    // 连招完成后的冷却时间（秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招基础", meta = (ClampMin = 0))
    float CooldownAfterComplete;

    FWeaponComboSequence()
        : ComboID(NAME_None)
        , ComboName(FText::GetEmpty())
        , Description(FText::GetEmpty())
        , MaxTotalDuration(3.0f)
        , bResetAfterComplete(true)
        , CooldownAfterComplete(0.5f)
    {}

    // 获取连招阶段数
    int32 GetStageCount() const { return StageConfigs.Num(); }

    // 获取输入序列的字符串表示（如 "L-L-H"）
    FString GetSequenceString() const;

    // 检查配置是否有效
    bool IsValidConfig() const;
};

// ==================== 连招运行时状态 ====================

/**
 * 连招运行时状态
 */
USTRUCT(BlueprintType)
struct FWeaponComboRuntimeState
{
    GENERATED_BODY()

    // 当前连招ID
    UPROPERTY(BlueprintReadOnly, Category = "连招状态")
    FName CurrentComboID;

    // 当前阶段索引
    UPROPERTY(BlueprintReadOnly, Category = "连招状态")
    int32 CurrentStageIndex;

    // 当前输入序列
    UPROPERTY(BlueprintReadOnly, Category = "连招状态")
    TArray<EWeaponComboInputType> CurrentInputSequence;

    // 连招开始时间
    UPROPERTY(BlueprintReadOnly, Category = "连招状态")
    float ComboStartTime;

    // 最后输入时间
    UPROPERTY(BlueprintReadOnly, Category = "连招状态")
    float LastInputTime;

    // 是否正在连招中
    UPROPERTY(BlueprintReadOnly, Category = "连招状态")
    bool bIsInCombo;

    // 当前阶段是否已完成
    UPROPERTY(BlueprintReadOnly, Category = "连招状态")
    bool bCurrentStageCompleted;

    // 连招已持续时间
    float GetElapsedTime(float CurrentWorldTime) const
    {
        return CurrentWorldTime - ComboStartTime;
    }

    // 距离最后输入的间隔
    float GetTimeSinceLastInput(float CurrentWorldTime) const
    {
        return CurrentWorldTime - LastInputTime;
    }

    FWeaponComboRuntimeState()
        : CurrentComboID(NAME_None)
        , CurrentStageIndex(0)
        , ComboStartTime(0.0f)
        , LastInputTime(0.0f)
        , bIsInCombo(false)
        , bCurrentStageCompleted(false)
    {}

    // 重置状态
    void Reset()
    {
        CurrentComboID = NAME_None;
        CurrentStageIndex = 0;
        CurrentInputSequence.Empty();
        ComboStartTime = 0.0f;
        LastInputTime = 0.0f;
        bIsInCombo = false;
        bCurrentStageCompleted = false;
    }
};

// ==================== 连招输入事件 ====================

/**
 * 连招输入事件结构
 */
USTRUCT(BlueprintType)
struct FWeaponComboInputEvent
{
    GENERATED_BODY()

    // 输入类型
    UPROPERTY(BlueprintReadOnly, Category = "输入事件")
    EWeaponComboInputType InputType;

    // 输入时间
    UPROPERTY(BlueprintReadOnly, Category = "输入事件")
    float InputTime;

    // 当前连招阶段（输入时的阶段）
    UPROPERTY(BlueprintReadOnly, Category = "输入事件")
    int32 CurrentStage;

    // 是否触发阶段推进
    UPROPERTY(BlueprintReadOnly, Category = "输入事件")
    bool bTriggersStageAdvance;

    FWeaponComboInputEvent()
        : InputType(EWeaponComboInputType::None)
        , InputTime(0.0f)
        , CurrentStage(0)
        , bTriggersStageAdvance(false)
    {}
};
