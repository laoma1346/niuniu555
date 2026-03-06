// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputBufferSystem.generated.h"

// 输入动作类型（与战斗相关）
UENUM(BlueprintType)
enum class EInputActionType : uint8
{
	None			UMETA(DisplayName = "无"),
	LightAttack		UMETA(DisplayName = "轻攻击"),
	HeavyAttack		UMETA(DisplayName = "重攻击"),
	Dodge			UMETA(DisplayName = "闪避"),
	Skill1			UMETA(DisplayName = "技能1"),
	Skill2			UMETA(DisplayName = "技能2"),
	Skill3			UMETA(DisplayName = "技能3"),
	Ultimate		UMETA(DisplayName = "大招"),
	Interact		UMETA(DisplayName = "交互"),
	UseItem			UMETA(DisplayName = "使用道具"),
	WeaponSwitch	UMETA(DisplayName = "切换武器"),
	Hookshot		UMETA(DisplayName = "钩索")
};

// 缓冲的输入项
USTRUCT()
struct FBufferedInput
{
	GENERATED_BODY()

	// 输入动作类型
	UPROPERTY()
	EInputActionType ActionType = EInputActionType::None;

	// 缓冲开始时间
	UPROPERTY()
	float BufferStartTime = 0.0f;

	// 是否已消费
	UPROPERTY()
	bool bConsumed = false;

	FBufferedInput() = default;
	FBufferedInput(EInputActionType InActionType, float InTime)
		: ActionType(InActionType), BufferStartTime(InTime), bConsumed(false) {}
};

// 输入缓冲完成委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputBuffered, EInputActionType, ActionType);

/**
 * 输入缓冲系统 - 处理战斗输入的预输入缓冲
 * 允许玩家在动画播放期间提前输入，提升手感
 */
UCLASS()
class NIUNIU555_API UInputBufferSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== 缓冲配置 ==========

	// 设置缓冲窗口时间（秒）
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Buffer")
	void SetBufferWindow(float WindowTime) { BufferWindowTime = FMath::Max(0.0f, WindowTime); }

	// 获取缓冲窗口时间
	UFUNCTION(BlueprintPure, Category = "InputSystem|Buffer")
	float GetBufferWindow() const { return BufferWindowTime; }

	// ========== 输入缓冲操作 ==========

	// 添加输入到缓冲队列
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Buffer")
	void BufferInput(EInputActionType ActionType);

	// 消费下一个可用的缓冲输入
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Buffer")
	EInputActionType ConsumeNextBufferedInput();

	// 查看下一个可用的缓冲输入（不消费）
	UFUNCTION(BlueprintPure, Category = "InputSystem|Buffer")
	EInputActionType PeekNextBufferedInput() const;

	// 检查是否有缓冲的输入
	UFUNCTION(BlueprintPure, Category = "InputSystem|Buffer")
	bool HasBufferedInput() const;

	// 清空所有缓冲输入
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Buffer")
	void ClearBuffer();

	// 清理过期的缓冲输入
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Buffer")
	void CleanExpiredInputs();

	// ========== Tick更新 ==========

	// 每帧更新（由GameInstance调用或手动调用）
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Buffer")
	void Update(float DeltaTime);

	// ========== 事件委托 ==========

	// 输入被缓冲时触发
	UPROPERTY(BlueprintAssignable, Category = "InputSystem|Buffer|Events")
	FOnInputBuffered OnInputBuffered;

protected:
	// 缓冲窗口时间（秒）- 默认0.2秒（200ms）
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InputSystem|Buffer|Config")
	float BufferWindowTime = 0.2f;

	// 最大缓冲数量
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InputSystem|Buffer|Config")
	int32 MaxBufferSize = 3;

	// 当前世界时间（用于计算过期）
	UPROPERTY()
	float CurrentWorldTime = 0.0f;

	// 缓冲队列
	UPROPERTY()
	TArray<FBufferedInput> InputBuffer;

	// 检查输入是否过期
	bool IsInputExpired(const FBufferedInput& Input) const;
};
