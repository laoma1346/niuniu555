// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputPriorityManager.generated.h"

// 输入模式枚举
UENUM(BlueprintType)
enum class EInputMode : uint8
{
	GameOnly		UMETA(DisplayName = "仅游戏"),
	UIOnly			UMETA(DisplayName = "仅UI"),
	GameAndUI		UMETA(DisplayName = "游戏+UI"),
	None			UMETA(DisplayName = "无输入")
};

// 输入优先级层级
UENUM(BlueprintType)
enum class EInputPriority : uint8
{
	Lowest			UMETA(DisplayName = "最低"),
	Gameplay		UMETA(DisplayName = "游戏玩法"),
	Combat			UMETA(DisplayName = "战斗"),
	UI				UMETA(DisplayName = "UI界面"),
	Menu			UMETA(DisplayName = "菜单"),
	Highest			UMETA(DisplayName = "最高/模态")
};

// 输入模式改变委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInputModeChanged, EInputMode, NewMode, EInputMode, OldMode);

/**
 * 输入优先级管理器 - 管理游戏不同状态下的输入优先级
 * 例如：打开菜单时UI输入优先，战斗中战斗输入优先
 */
UCLASS()
class NIUNIU555_API UInputPriorityManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== 输入模式管理 ==========

	// 设置当前输入模式
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Priority")
	void SetInputMode(EInputMode NewMode);

	// 获取当前输入模式
	UFUNCTION(BlueprintPure, Category = "InputSystem|Priority")
	EInputMode GetCurrentInputMode() const { return CurrentInputMode; }

	// 临时覆盖输入模式（会记录之前的模式）
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Priority")
	void PushInputMode(EInputMode OverrideMode);

	// 恢复之前的输入模式
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Priority")
	void PopInputMode();

	// ========== 优先级管理 ==========

	// 设置当前输入优先级
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Priority")
	void SetInputPriority(EInputPriority Priority) { CurrentPriority = Priority; }

	// 获取当前输入优先级
	UFUNCTION(BlueprintPure, Category = "InputSystem|Priority")
	EInputPriority GetInputPriority() const { return CurrentPriority; }

	// 检查是否允许某优先级的输入
	UFUNCTION(BlueprintPure, Category = "InputSystem|Priority")
	bool IsPriorityAllowed(EInputPriority TestPriority) const;

	// ========== 功能开关 ==========

	// 是否允许游戏输入
	UFUNCTION(BlueprintPure, Category = "InputSystem|Priority")
	bool IsGameplayInputAllowed() const;

	// 是否允许UI输入
	UFUNCTION(BlueprintPure, Category = "InputSystem|Priority")
	bool IsUIInputAllowed() const;

	// 是否允许战斗输入
	UFUNCTION(BlueprintPure, Category = "InputSystem|Priority")
	bool IsCombatInputAllowed() const;

	// ========== 快捷设置 ==========

	// 进入战斗模式（战斗输入优先）
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Priority")
	void EnterCombatMode();

	// 退出战斗模式（恢复正常游戏输入）
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Priority")
	void ExitCombatMode();

	// 打开UI（UI输入优先）
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Priority")
	void OpenUI(EInputPriority Priority = EInputPriority::UI);

	// 关闭UI（恢复游戏输入）
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Priority")
	void CloseUI();

	// 打开菜单（菜单输入优先）
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Priority")
	void OpenMenu();

	// 关闭菜单
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Priority")
	void CloseMenu();

	// 暂停游戏输入
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Priority")
	void PauseGameplayInput();

	// 恢复游戏输入
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Priority")
	void ResumeGameplayInput();

	// ========== 事件委托 ==========

	// 输入模式改变时触发
	UPROPERTY(BlueprintAssignable, Category = "InputSystem|Priority|Events")
	FOnInputModeChanged OnInputModeChanged;

protected:
	// 当前输入模式
	UPROPERTY(BlueprintReadOnly, Category = "InputSystem|Priority")
	EInputMode CurrentInputMode = EInputMode::GameOnly;

	// 之前的输入模式（用于Push/Pop）
	UPROPERTY()
	TArray<EInputMode> ModeStack;

	// 当前输入优先级
	UPROPERTY(BlueprintReadOnly, Category = "InputSystem|Priority")
	EInputPriority CurrentPriority = EInputPriority::Gameplay;

	// 暂停前的输入模式（用于Pause/Resume）
	UPROPERTY()
	EInputMode PrePauseMode = EInputMode::GameOnly;

	// 是否在暂停状态
	UPROPERTY()
	bool bIsPaused = false;

	// 应用输入模式到PlayerController
	void ApplyInputMode(EInputMode Mode);
};
