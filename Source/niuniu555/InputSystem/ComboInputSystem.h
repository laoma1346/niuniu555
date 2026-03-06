// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ComboInputSystem.generated.h"

// 连招输入类型（轻/重攻击）
UENUM(BlueprintType)
enum class EComboInputType : uint8
{
	None	UMETA(DisplayName = "无"),
	Light	UMETA(DisplayName = "轻击 - L"),
	Heavy	UMETA(DisplayName = "重击 - H")
};

// 连招序列配置
USTRUCT(BlueprintType)
struct FComboSequence
{
	GENERATED_BODY()

	// 连招ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FName ComboID;

	// 连招名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FString ComboName;

	// 输入序列（如 L-L-H）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	TArray<EComboInputType> InputSequence;

	// 序列间最大间隔时间（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	float MaxInterval = 0.5f;

	// 连招完成触发的技能ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FName SkillToTrigger;

	FComboSequence() = default;
};

// 连招检测完成委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboDetected, FName, ComboID);

/**
 * 连招输入系统 - 检测和管理连招序列
 * 参考《哈迪斯2》的连招系统：轻击(L)和重击(H)的组合
 */
UCLASS()
class NIUNIU555_API UComboInputSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== 连招配置 ==========

	// 注册连招序列
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Combo")
	void RegisterCombo(const FComboSequence& ComboConfig);

	// 移除连招序列
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Combo")
	void UnregisterCombo(FName ComboID);

	// 清空所有连招
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Combo")
	void ClearAllCombos();

	// 获取所有已注册的连招
	UFUNCTION(BlueprintPure, Category = "InputSystem|Combo")
	TArray<FComboSequence> GetAllCombos() const;

	// ========== 输入处理 ==========

	// 添加连招输入（轻击或重击）
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Combo")
	void AddComboInput(EComboInputType InputType);

	// 检测当前输入序列是否匹配任何连招
	UFUNCTION(BlueprintPure, Category = "InputSystem|Combo")
	FName CheckComboMatch() const;

	// 检测是否是某个连招的前缀
	UFUNCTION(BlueprintPure, Category = "InputSystem|Combo")
	bool IsComboPrefix(FName ComboID) const;

	// 获取当前输入序列
	UFUNCTION(BlueprintPure, Category = "InputSystem|Combo")
	TArray<EComboInputType> GetCurrentInputSequence() const { return CurrentInputSequence; }

	// 获取当前序列的字符串表示（如 "L-L-H"）
	UFUNCTION(BlueprintPure, Category = "InputSystem|Combo")
	FString GetCurrentSequenceString() const;

	// ========== 序列管理 ==========

	// 清空当前输入序列
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Combo")
	void ClearCurrentSequence();

	// 设置序列超时时间
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Combo")
	void SetSequenceTimeout(float Timeout) { SequenceTimeout = FMath::Max(0.1f, Timeout); }

	// 获取序列超时时间
	UFUNCTION(BlueprintPure, Category = "InputSystem|Combo")
	float GetSequenceTimeout() const { return SequenceTimeout; }

	// ========== Tick更新 ==========

	// 每帧更新（检测序列超时）
	UFUNCTION(BlueprintCallable, Category = "InputSystem|Combo")
	void Update(float DeltaTime);

	// ========== 事件委托 ==========

	// 检测到完整连招时触发
	UPROPERTY(BlueprintAssignable, Category = "InputSystem|Combo|Events")
	FOnComboDetected OnComboDetected;

protected:
	// 所有注册的连招配置
	UPROPERTY()
	TArray<FComboSequence> RegisteredCombos;

	// 当前输入序列
	UPROPERTY()
	TArray<EComboInputType> CurrentInputSequence;

	// 每次输入的时间戳
	UPROPERTY()
	TArray<float> InputTimestamps;

	// 序列超时时间（秒）- 默认0.8秒
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InputSystem|Combo|Config")
	float SequenceTimeout = 0.8f;

	// 最大序列长度
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InputSystem|Combo|Config")
	int32 MaxSequenceLength = 6;

	// 当前世界时间
	UPROPERTY()
	float CurrentWorldTime = 0.0f;

	// 最后输入时间
	UPROPERTY()
	float LastInputTime = 0.0f;

	// 检查序列是否超时
	bool IsSequenceTimedOut() const;

	// 序列是否匹配
	bool DoesSequenceMatch(const TArray<EComboInputType>& Pattern) const;

	// 序列是否是前缀
	bool IsSequencePrefix(const TArray<EComboInputType>& Pattern) const;

	// 转换输入序列为字符串
	static FString SequenceToString(const TArray<EComboInputType>& Sequence);
};
