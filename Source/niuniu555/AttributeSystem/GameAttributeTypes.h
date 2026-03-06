// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameAttributeTypes.generated.h"

// 属性类型枚举
UENUM(BlueprintType)
enum class EAttributeType : uint8
{
	None			UMETA(DisplayName = "无"),
	Health			UMETA(DisplayName = "生命值"),
	MaxHealth		UMETA(DisplayName = "最大生命值"),
	Mana			UMETA(DisplayName = "神力值"),
	MaxMana			UMETA(DisplayName = "最大神力值"),
	Attack			UMETA(DisplayName = "攻击力"),
	Defense			UMETA(DisplayName = "防御力"),
	MoveSpeed		UMETA(DisplayName = "移动速度"),
	AttackSpeed		UMETA(DisplayName = "攻击速度"),
	CriticalRate	UMETA(DisplayName = "暴击率"),
	CriticalDamage	UMETA(DisplayName = "暴击伤害"),
	GoldGainRate	UMETA(DisplayName = "金币获取率")
};

// 属性值结构（基础值 + 附加值 + 乘数）
USTRUCT(BlueprintType)
struct FAttributeValue
{
	GENERATED_BODY()

	// 基础值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float BaseValue = 0.0f;

	// 附加值（来自装备、Buff等）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float AdditiveValue = 0.0f;

	// 乘数（百分比加成）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float Multiplier = 1.0f;

	// 获取最终值
	float GetFinalValue() const
	{
		return (BaseValue + AdditiveValue) * Multiplier;
	}

	// 设置基础值
	void SetBaseValue(float Value) { BaseValue = Value; }

	// 添加附加值
	void AddAdditive(float Value) { AdditiveValue += Value; }

	// 添加乘数
	void AddMultiplier(float Value) { Multiplier += Value; }

	// 重置附加值和乘数
	void ResetModifiers()
	{
		AdditiveValue = 0.0f;
		Multiplier = 1.0f;
	}
};

// 属性集合
USTRUCT(BlueprintType)
struct FAttributeSet
{
	GENERATED_BODY()

	// 生命值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FAttributeValue Health;

	// 最大生命值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FAttributeValue MaxHealth;

	// 神力值（魔法值）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FAttributeValue Mana;

	// 最大神力值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FAttributeValue MaxMana;

	// 攻击力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FAttributeValue Attack;

	// 防御力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FAttributeValue Defense;

	// 移动速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FAttributeValue MoveSpeed;

	// 攻击速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FAttributeValue AttackSpeed;

	// 暴击率（0-1）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FAttributeValue CriticalRate;

	// 暴击伤害（默认1.5）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FAttributeValue CriticalDamage;

	// 金币获取率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FAttributeValue GoldGainRate;

	// 初始化默认值
	void InitializeDefaults()
	{
		MaxHealth.BaseValue = 100.0f;
		Health.BaseValue = 100.0f;
		MaxMana.BaseValue = 50.0f;
		Mana.BaseValue = 50.0f;
		Attack.BaseValue = 10.0f;
		Defense.BaseValue = 5.0f;
		MoveSpeed.BaseValue = 600.0f;
		AttackSpeed.BaseValue = 1.0f;
		CriticalRate.BaseValue = 0.05f;
		CriticalDamage.BaseValue = 1.5f;
		GoldGainRate.BaseValue = 1.0f;
	}

	// 获取指定属性的最终值
	float GetAttributeValue(EAttributeType Type) const;

	// 获取指定属性的引用
	FAttributeValue* GetAttribute(EAttributeType Type);
};
