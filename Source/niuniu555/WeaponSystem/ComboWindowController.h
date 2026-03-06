// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComboTypes.h"
#include "ComboWindowController.generated.h"

// 窗口状态
UENUM(BlueprintType)
enum class EComboWindowState : uint8
{
    None,
    Active,         // 窗口激活（可取消）
    Locked,         // 锁定状态（不可取消）
    Closed          // 窗口关闭
};

// 窗口事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboWindowOpened);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboWindowClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboCancelAllowedChanged, bool, bAllowed);

/**
 * 连招窗口控制器
 * 管理连招取消窗口和阶段过渡时间
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NIUNIU555_API UComboWindowController : public UActorComponent
{
    GENERATED_BODY()

public:
    UComboWindowController();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // ========== 窗口控制 ==========

    // 开始连招阶段窗口
    UFUNCTION(BlueprintCallable, Category = "连招窗口")
    void StartComboStageWindow(const FWeaponComboStageConfig& StageConfig);

    // 手动打开取消窗口
    UFUNCTION(BlueprintCallable, Category = "连招窗口")
    void OpenCancelWindow(float WindowDuration);

    // 关闭取消窗口
    UFUNCTION(BlueprintCallable, Category = "连招窗口")
    void CloseCancelWindow();

    // 强制结束所有窗口
    UFUNCTION(BlueprintCallable, Category = "连招窗口")
    void ForceEndWindows();

    // ========== 状态查询 ==========

    // 获取当前窗口状态
    UFUNCTION(BlueprintPure, Category = "连招窗口")
    EComboWindowState GetWindowState() const { return CurrentWindowState; }

    // 是否允许取消（即窗口激活）
    UFUNCTION(BlueprintPure, Category = "连招窗口")
    bool IsCancelAllowed() const;

    // 是否允许连招延续
    UFUNCTION(BlueprintPure, Category = "连招窗口")
    bool CanContinueCombo() const;

    // 获取当前阶段配置
    UFUNCTION(BlueprintPure, Category = "连招窗口")
    FWeaponComboStageConfig GetCurrentStageConfig() const { return CurrentStageConfig; }

    // ========== 事件 ==========

    // 窗口打开事件
    UPROPERTY(BlueprintAssignable, Category = "连招窗口|事件")
    FOnComboWindowOpened OnWindowOpened;

    // 窗口关闭事件
    UPROPERTY(BlueprintAssignable, Category = "连招窗口|事件")
    FOnComboWindowClosed OnWindowClosed;

    // 取消允许状态改变事件
    UPROPERTY(BlueprintAssignable, Category = "连招窗口|事件")
    FOnComboCancelAllowedChanged OnCancelAllowedChanged;

protected:
    // 当前窗口状态
    UPROPERTY()
    EComboWindowState CurrentWindowState;

    // 当前阶段配置
    UPROPERTY()
    FWeaponComboStageConfig CurrentStageConfig;

    // 窗口计时器
    UPROPERTY()
    float WindowTimer;

    // 当前窗口持续时间
    UPROPERTY()
    float CurrentWindowDuration;

    // 取消窗口是否激活
    UPROPERTY()
    bool bIsCancelWindowActive;

    // 最后取消允许状态（用于检测变化）
    UPROPERTY()
    bool bLastCancelAllowedState;

protected:
    // 设置窗口状态
    UFUNCTION()
    void SetWindowState(EComboWindowState NewState);

    // 更新窗口状态
    UFUNCTION()
    void UpdateWindow(float DeltaTime);

    // 广播取消状态改变
    UFUNCTION()
    void BroadcastCancelStateChanged();
};
