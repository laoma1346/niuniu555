// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputDeviceManager.generated.h"

// 输入设备类型枚举
UENUM(BlueprintType)
enum class EInputDeviceType : uint8
{
	KeyboardMouse	UMETA(DisplayName = "键盘鼠标"),
	Gamepad			UMETA(DisplayName = "手柄"),
	Touch			UMETA(DisplayName = "触摸屏"),
	Unknown			UMETA(DisplayName = "未知")
};

// 输入设备切换委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputDeviceChanged, EInputDeviceType, NewDeviceType);

/**
 * 输入设备管理器 - 管理键鼠/手柄等输入设备的检测和切换
 */
UCLASS()
class NIUNIU555_API UInputDeviceManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== 设备类型查询 ==========

	// 获取当前输入设备类型
	UFUNCTION(BlueprintPure, Category = "InputSystem|Device")
	EInputDeviceType GetCurrentInputDevice() const { return CurrentDeviceType; }

	// 是否是键鼠输入
	UFUNCTION(BlueprintPure, Category = "InputSystem|Device")
	bool IsUsingKeyboardMouse() const { return CurrentDeviceType == EInputDeviceType::KeyboardMouse; }

	// 是否是手柄输入
	UFUNCTION(BlueprintPure, Category = "InputSystem|Device")
	bool IsUsingGamepad() const { return CurrentDeviceType == EInputDeviceType::Gamepad; }

	// ========== 设备切换事件 ==========

	// 输入设备改变时触发
	UPROPERTY(BlueprintAssignable, Category = "InputSystem|Device|Events")
	FOnInputDeviceChanged OnInputDeviceChanged;

	// ========== 手柄支持查询 ==========

	// 是否有手柄连接
	UFUNCTION(BlueprintPure, Category = "InputSystem|Device")
	bool IsGamepadConnected() const;

	// 获取连接的手柄数量
	UFUNCTION(BlueprintPure, Category = "InputSystem|Device")
	int32 GetConnectedGamepadCount() const;

	// ========== 输入处理 ==========

	// 处理输入事件（由PlayerController调用）
	void ProcessInputEvent(const FKey& Key);

protected:
	// 当前输入设备类型
	UPROPERTY(BlueprintReadOnly, Category = "InputSystem|Device")
	EInputDeviceType CurrentDeviceType = EInputDeviceType::KeyboardMouse;

	// 上一个输入设备类型
	UPROPERTY(BlueprintReadOnly, Category = "InputSystem|Device")
	EInputDeviceType PreviousDeviceType = EInputDeviceType::Unknown;

	// 检测设备类型
	EInputDeviceType DetectInputDevice(const FKey& Key) const;

	// 切换设备类型
	void SwitchInputDevice(EInputDeviceType NewDeviceType);

	// 手柄按键名称前缀（用于判断是否为手柄输入）
	static const TArray<FName> GamepadKeyPrefixes;
	static const TArray<FName> MouseKeyNames;
};
