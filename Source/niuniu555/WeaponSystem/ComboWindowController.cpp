// Copyright Epic Games, Inc. All Rights Reserved.

#include "ComboWindowController.h"

UComboWindowController::UComboWindowController()
{
    PrimaryComponentTick.bCanEverTick = true;

    CurrentWindowState = EComboWindowState::None;
    WindowTimer = 0.0f;
    CurrentWindowDuration = 0.0f;
    bIsCancelWindowActive = false;
    bLastCancelAllowedState = false;
}

void UComboWindowController::BeginPlay()
{
    Super::BeginPlay();
}

void UComboWindowController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ForceEndWindows();
    Super::EndPlay(EndPlayReason);
}

void UComboWindowController::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateWindow(DeltaTime);
}

// ========== 窗口控制 ==========

void UComboWindowController::StartComboStageWindow(const FWeaponComboStageConfig& StageConfig)
{
    CurrentStageConfig = StageConfig;

    // 计算窗口持续时间（从开始到结束）
    float WindowStart = StageConfig.CancelWindowStart;
    float WindowEnd = StageConfig.CancelWindowEnd;

    // 设置初始状态为锁定，等待到达窗口开始时间
    if (WindowStart > 0.0f)
    {
        SetWindowState(EComboWindowState::Locked);
        WindowTimer = 0.0f;
        CurrentWindowDuration = WindowStart;
    }
    else
    {
        // 立即进入激活状态
        SetWindowState(EComboWindowState::Active);
        WindowTimer = 0.0f;
        CurrentWindowDuration = WindowEnd;
        OnWindowOpened.Broadcast();
    }

    UE_LOG(LogTemp, Verbose, TEXT("[ComboWindow] 开始阶段 %d 窗口, 配置: Start=%.2f, End=%.2f"),
        StageConfig.StageIndex, WindowStart, WindowEnd);
}

void UComboWindowController::OpenCancelWindow(float WindowDuration)
{
    if (WindowDuration <= 0.0f)
    {
        return;
    }

    SetWindowState(EComboWindowState::Active);
    WindowTimer = 0.0f;
    CurrentWindowDuration = WindowDuration;
    OnWindowOpened.Broadcast();

    UE_LOG(LogTemp, Verbose, TEXT("[ComboWindow] 手动打开取消窗口, 持续时间: %.2f"), WindowDuration);
}

void UComboWindowController::CloseCancelWindow()
{
    if (CurrentWindowState != EComboWindowState::Closed)
    {
        SetWindowState(EComboWindowState::Closed);
        OnWindowClosed.Broadcast();

        UE_LOG(LogTemp, Verbose, TEXT("[ComboWindow] 取消窗口已关闭"));
    }
}

void UComboWindowController::ForceEndWindows()
{
    if (CurrentWindowState != EComboWindowState::None)
    {
        SetWindowState(EComboWindowState::Closed);
        OnWindowClosed.Broadcast();

        UE_LOG(LogTemp, Verbose, TEXT("[ComboWindow] 窗口强制结束"));
    }
}

// ========== 状态查询 ==========

bool UComboWindowController::IsCancelAllowed() const
{
    return CurrentWindowState == EComboWindowState::Active;
}

bool UComboWindowController::CanContinueCombo() const
{
    // 窗口激活或在锁定状态都可以延续连招
    return CurrentWindowState == EComboWindowState::Active ||
           CurrentWindowState == EComboWindowState::Locked;
}

// ========== 内部方法 ==========

void UComboWindowController::SetWindowState(EComboWindowState NewState)
{
    if (CurrentWindowState != NewState)
    {
        CurrentWindowState = NewState;

        bool bNewCancelAllowed = IsCancelAllowed();
        if (bNewCancelAllowed != bLastCancelAllowedState)
        {
            bLastCancelAllowedState = bNewCancelAllowed;
            BroadcastCancelStateChanged();
        }
    }
}

void UComboWindowController::UpdateWindow(float DeltaTime)
{
    if (CurrentWindowState == EComboWindowState::None ||
        CurrentWindowState == EComboWindowState::Closed)
    {
        return;
    }

    WindowTimer += DeltaTime;

    if (WindowTimer >= CurrentWindowDuration)
    {
        // 检查是否需要转换状态
        if (CurrentWindowState == EComboWindowState::Locked)
        {
            // 从锁定转为激活
            float ActiveDuration = CurrentStageConfig.CancelWindowEnd - CurrentStageConfig.CancelWindowStart;
            if (ActiveDuration > 0.0f)
            {
                SetWindowState(EComboWindowState::Active);
                WindowTimer = 0.0f;
                CurrentWindowDuration = ActiveDuration;
                OnWindowOpened.Broadcast();

                UE_LOG(LogTemp, Verbose, TEXT("[ComboWindow] 窗口转为激活状态"));
            }
            else
            {
                CloseCancelWindow();
            }
        }
        else if (CurrentWindowState == EComboWindowState::Active)
        {
            // 窗口结束
            CloseCancelWindow();
        }
    }
}

void UComboWindowController::BroadcastCancelStateChanged()
{
    OnCancelAllowedChanged.Broadcast(IsCancelAllowed());
}
