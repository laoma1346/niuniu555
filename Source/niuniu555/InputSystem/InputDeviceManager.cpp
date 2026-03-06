// Copyright Epic Games, Inc. All Rights Reserved.

#include "InputDeviceManager.h"

// 手柄按键前缀列表
const TArray<FName> UInputDeviceManager::GamepadKeyPrefixes = {
	TEXT("Gamepad"),
	TEXT("XboxType"),
	TEXT("PS4"),
	TEXT("PS5"),
	TEXT("Switch")
};

// 鼠标按键名称列表
const TArray<FName> UInputDeviceManager::MouseKeyNames = {
	TEXT("LeftMouseButton"),
	TEXT("RightMouseButton"),
	TEXT("MiddleMouseButton"),
	TEXT("ThumbMouseButton"),
	TEXT("ThumbMouseButton2"),
	TEXT("MouseScrollUp"),
	TEXT("MouseScrollDown"),
	TEXT("MouseX"),
	TEXT("MouseY")
};

void UInputDeviceManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentDeviceType = EInputDeviceType::KeyboardMouse;
	PreviousDeviceType = EInputDeviceType::Unknown;
	
	UE_LOG(LogTemp, Log, TEXT("InputDeviceManager: Initialized"));
}

void UInputDeviceManager::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("InputDeviceManager: Deinitialized"));
}

bool UInputDeviceManager::IsGamepadConnected() const
{
	// 简化实现：检测不到具体手柄连接状态
	// 在实际游戏中，可以通过检测Gamepad键输入来判断
	return true; // 假设可能有手柄
}

int32 UInputDeviceManager::GetConnectedGamepadCount() const
{
	// 简化实现
	return 1; // 假设有1个手柄
}

void UInputDeviceManager::ProcessInputEvent(const FKey& Key)
{
	if (!Key.IsValid())
	{
		return;
	}

	EInputDeviceType DetectedDevice = DetectInputDevice(Key);
	
	if (DetectedDevice != EInputDeviceType::Unknown && DetectedDevice != CurrentDeviceType)
	{
		SwitchInputDevice(DetectedDevice);
	}
}

EInputDeviceType UInputDeviceManager::DetectInputDevice(const FKey& Key) const
{
	const FName KeyName = Key.GetFName();
	const FString KeyNameString = KeyName.ToString();

	// 检查是否是鼠标按键
	if (MouseKeyNames.Contains(KeyName))
	{
		return EInputDeviceType::KeyboardMouse;
	}

	// 检查是否是手柄按键（通过前缀判断）
	for (const FName& Prefix : GamepadKeyPrefixes)
	{
		if (KeyNameString.StartsWith(Prefix.ToString()))
		{
			return EInputDeviceType::Gamepad;
		}
	}

	// 检查是否是游戏手柄专用按键
	static const TArray<FName> GamepadSpecificKeys = {
		TEXT("FaceButton_Bottom"), TEXT("FaceButton_Right"), TEXT("FaceButton_Left"), TEXT("FaceButton_Top"),
		TEXT("LeftShoulder"), TEXT("RightShoulder"), TEXT("LeftTrigger"), TEXT("RightTrigger"),
		TEXT("DPad_Up"), TEXT("DPad_Down"), TEXT("DPad_Left"), TEXT("DPad_Right"),
		TEXT("LeftStick_Up"), TEXT("LeftStick_Down"), TEXT("LeftStick_Left"), TEXT("LeftStick_Right"),
		TEXT("RightStick_Up"), TEXT("RightStick_Down"), TEXT("RightStick_Left"), TEXT("RightStick_Right"),
		TEXT("Special_Left"), TEXT("Special_Right"), TEXT("LeftStickDown"), TEXT("RightStickDown")
	};

	if (GamepadSpecificKeys.Contains(KeyName))
	{
		return EInputDeviceType::Gamepad;
	}

	// 默认认为是键盘（Touch类型需要特殊处理）
	return EInputDeviceType::KeyboardMouse;
}

void UInputDeviceManager::SwitchInputDevice(EInputDeviceType NewDeviceType)
{
	if (NewDeviceType == CurrentDeviceType)
	{
		return;
	}

	PreviousDeviceType = CurrentDeviceType;
	CurrentDeviceType = NewDeviceType;

	// 输出日志
	const UEnum* EnumPtr = StaticEnum<EInputDeviceType>();
	if (EnumPtr)
	{
		UE_LOG(LogTemp, Log, TEXT("InputDeviceManager: Device switched from %s to %s"),
			*EnumPtr->GetNameStringByValue((int64)PreviousDeviceType),
			*EnumPtr->GetNameStringByValue((int64)CurrentDeviceType));
	}

	// 广播事件
	OnInputDeviceChanged.Broadcast(CurrentDeviceType);
}
