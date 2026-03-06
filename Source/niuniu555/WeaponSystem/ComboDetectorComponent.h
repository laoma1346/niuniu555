// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComboTypes.h"
#include "ComboDetectorComponent.generated.h"

// 连招检测完成委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponComboDetected, FName, ComboID, int32, CompletedStageCount);

// 连招阶段推进委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponComboStageAdvanced, FName, ComboID, int32, NewStageIndex);

// 连招中断委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponComboBroken, FName, ComboID, FString, Reason);

// 连招输入接收委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponComboInputReceived, EWeaponComboInputType, InputType);

/**
 * 连招检测器组件
 * 负责检测和管理连招输入序列
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NIUNIU555_API UComboDetectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UComboDetectorComponent();

    // ========== 组件接口 ==========
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // ========== 连招注册 ==========

    // 注册连招序列
    UFUNCTION(BlueprintCallable, Category = "连招检测器")
    void RegisterCombo(const FWeaponComboSequence& ComboConfig);

    // 注销连招
    UFUNCTION(BlueprintCallable, Category = "连招检测器")
    void UnregisterCombo(FName ComboID);

    // 清空所有连招
    UFUNCTION(BlueprintCallable, Category = "连招检测器")
    void ClearAllCombos();

    // ========== 输入处理 ==========

    // 添加连招输入
    UFUNCTION(BlueprintCallable, Category = "连招检测器")
    void AddComboInput(EWeaponComboInputType InputType);

    // 强制中断当前连招
    UFUNCTION(BlueprintCallable, Category = "连招检测器")
    void BreakCombo(const FString& Reason);

    // 重置连招状态
    UFUNCTION(BlueprintCallable, Category = "连招检测器")
    void ResetComboState();

    // ========== 状态查询 ==========

    // 是否正在连招中
    UFUNCTION(BlueprintPure, Category = "连招检测器")
    bool IsInCombo() const { return RuntimeState.bIsInCombo; }

    // 获取当前连招ID
    UFUNCTION(BlueprintPure, Category = "连招检测器")
    FName GetCurrentComboID() const { return RuntimeState.CurrentComboID; }

    // 获取当前阶段索引
    UFUNCTION(BlueprintPure, Category = "连招检测器")
    int32 GetCurrentStageIndex() const { return RuntimeState.CurrentStageIndex; }

    // 获取当前输入序列字符串
    UFUNCTION(BlueprintPure, Category = "连招检测器")
    FString GetCurrentInputSequenceString() const;

    // 获取当前阶段配置
    UFUNCTION(BlueprintPure, Category = "连招检测器")
    FWeaponComboStageConfig GetCurrentStageConfig() const;

    // 检查指定输入是否匹配下一阶段的期望输入
    UFUNCTION(BlueprintPure, Category = "连招检测器")
    bool DoesInputMatchNextStage(EWeaponComboInputType InputType) const;

    // ========== 事件委托 ==========

    // 连招检测完成事件
    UPROPERTY(BlueprintAssignable, Category = "连招检测器|事件")
    FOnWeaponComboDetected OnComboDetected;

    // 连招阶段推进事件
    UPROPERTY(BlueprintAssignable, Category = "连招检测器|事件")
    FOnWeaponComboStageAdvanced OnComboStageAdvanced;

    // 连招中断事件
    UPROPERTY(BlueprintAssignable, Category = "连招检测器|事件")
    FOnWeaponComboBroken OnComboBroken;

    // 连招输入接收事件
    UPROPERTY(BlueprintAssignable, Category = "连招检测器|事件")
    FOnWeaponComboInputReceived OnComboInputReceived;

public:
    // ========== 配置参数 ==========

    // 默认输入容忍时间（秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招检测器|配置")
    float DefaultInputToleranceTime;

    // 默认连招总时长限制（秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招检测器|配置")
    float DefaultMaxComboDuration;

    // 是否自动检测最佳匹配连招
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连招检测器|配置")
    bool bAutoDetectBestMatch;

protected:
    // ========== 运行时数据 ==========

    // 注册的连招序列表
    UPROPERTY()
    TArray<FWeaponComboSequence> RegisteredCombos;

    // 当前运行时状态
    UPROPERTY()
    FWeaponComboRuntimeState RuntimeState;

    // 当前匹配的连招配置索引（-1表示无）
    UPROPERTY()
    int32 CurrentComboIndex;

    // 冷却计时器
    UPROPERTY()
    float CooldownTimer;

protected:
    // ========== 内部方法 ==========

    // 查找匹配的连招索引
    UFUNCTION()
    int32 FindMatchingComboIndex();

    // 检查输入序列是否匹配连招前缀
    UFUNCTION()
    bool IsSequencePrefix(const TArray<EWeaponComboInputType>& Sequence, const TArray<EWeaponComboInputType>& Prefix) const;

    // 推进到下一阶段
    UFUNCTION()
    void AdvanceToNextStage();

    // 完成连招
    UFUNCTION()
    void CompleteCombo();

    // 检查连招是否超时
    UFUNCTION()
    bool IsComboTimedOut() const;

    // 检查当前阶段是否可取消
    UFUNCTION()
    bool IsCurrentStageCancellable() const;

    // 获取当前世界时间
    UFUNCTION()
    float GetCurrentWorldTime() const;
};
