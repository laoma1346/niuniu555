// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameAttributeTypes.h"
#include "AttributeModifier.generated.h"

// 修饰器类型
UENUM(BlueprintType)
enum class EModifierType : uint8
{
	Additive		UMETA(DisplayName = "加法"),
	Multiplier		UMETA(DisplayName = "乘法"),
	Override		UMETA(DisplayName = "覆盖")
};

// 修饰器信息
USTRUCT(BlueprintType)
struct FAttributeModifier
{
	GENERATED_BODY()

	// 修饰器ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ModifierID;

	// 目标属性
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAttributeType TargetAttribute;

	// 修饰器类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EModifierType ModifierType;

	// 数值
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value;

	// 持续时间（-1表示永久）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration;

	// 剩余时间
	UPROPERTY()
	float RemainingTime;

	// 是否生效
	UPROPERTY(BlueprintReadOnly)
	bool bIsActive = true;

	FAttributeModifier()
		: TargetAttribute(EAttributeType::None)
		, ModifierType(EModifierType::Additive)
		, Value(0.0f)
		, Duration(-1.0f)
		, RemainingTime(-1.0f)
	{}

	// 更新剩余时间
	bool Update(float DeltaTime)
	{
		if (Duration > 0.0f)
		{
			RemainingTime -= DeltaTime;
			if (RemainingTime <= 0.0f)
			{
				bIsActive = false;
				return false; // 修饰器过期
			}
		}
		return true; // 修饰器仍然有效
	}

	// 初始化
	void Initialize()
	{
		RemainingTime = Duration;
		bIsActive = true;
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NIUNIU555_API UAttributeModifierComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeModifierComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== 添加修饰器 ==========

	// 添加临时修饰器（Buff/Debuff）
	UFUNCTION(BlueprintCallable, Category = "Attribute|Modifier")
	void AddModifier(const FAttributeModifier& Modifier);

	// 快捷方法：添加攻击力加成
	UFUNCTION(BlueprintCallable, Category = "Attribute|Modifier")
	void AddAttackBuff(float Value, float Duration, FName BuffID);

	// 快捷方法：添加防御力加成
	UFUNCTION(BlueprintCallable, Category = "Attribute|Modifier")
	void AddDefenseBuff(float Value, float Duration, FName BuffID);

	// 快捷方法：添加移速加成
	UFUNCTION(BlueprintCallable, Category = "Attribute|Modifier")
	void AddSpeedBuff(float Value, float Duration, FName BuffID);

	// ========== 移除修饰器 ==========

	// 移除指定ID的修饰器
	UFUNCTION(BlueprintCallable, Category = "Attribute|Modifier")
	void RemoveModifier(FName ModifierID);

	// 移除所有修饰器
	UFUNCTION(BlueprintCallable, Category = "Attribute|Modifier")
	void RemoveAllModifiers();

	// 移除指定属性的所有修饰器
	UFUNCTION(BlueprintCallable, Category = "Attribute|Modifier")
	void RemoveModifiersByAttribute(EAttributeType AttributeType);

	// ========== 查询 ==========

	// 获取活动修饰器数量
	UFUNCTION(BlueprintPure, Category = "Attribute|Modifier")
	int32 GetActiveModifierCount() const;

	// 是否有指定ID的修饰器
	UFUNCTION(BlueprintPure, Category = "Attribute|Modifier")
	bool HasModifier(FName ModifierID) const;

protected:
	// 所有修饰器
	UPROPERTY()
	TArray<FAttributeModifier> Modifiers;

	// 目标属性组件
	UPROPERTY()
	TObjectPtr<class UAttributeComponent> TargetAttributeComp;

	// 应用修饰器到属性
	void ApplyModifier(const FAttributeModifier& Modifier);

	// 移除修饰器效果
	void UnapplyModifier(const FAttributeModifier& Modifier);

	// 更新修饰器
	void UpdateModifiers(float DeltaTime);
};
