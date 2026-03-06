// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameAttributeTypes.h"
#include "AttributeComponent.generated.h"

// 属性变化委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChanged, EAttributeType, AttributeType, float, NewValue);

// 生命值变化委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth, float, Delta);

// 死亡委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NIUNIU555_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== 属性访问 ==========

	// 获取属性集合
	UFUNCTION(BlueprintPure, Category = "Attribute")
	FAttributeSet GetAttributeSet() const { return Attributes; }

	// 获取指定属性的最终值
	UFUNCTION(BlueprintPure, Category = "Attribute")
	float GetAttributeValue(EAttributeType Type) const;

	// 获取基础值
	UFUNCTION(BlueprintPure, Category = "Attribute")
	float GetBaseValue(EAttributeType Type) const;

	// 设置基础值
	UFUNCTION(BlueprintCallable, Category = "Attribute")
	void SetBaseValue(EAttributeType Type, float Value);

	// ========== 生命值相关 ==========

	// 获取当前生命值
	UFUNCTION(BlueprintPure, Category = "Attribute|Health")
	float GetHealth() const { return Attributes.Health.GetFinalValue(); }

	// 获取最大生命值
	UFUNCTION(BlueprintPure, Category = "Attribute|Health")
	float GetMaxHealth() const { return Attributes.MaxHealth.GetFinalValue(); }

	// 获取生命值百分比
	UFUNCTION(BlueprintPure, Category = "Attribute|Health")
	float GetHealthPercent() const;

	// 修改生命值（正数为治疗，负数为伤害）
	UFUNCTION(BlueprintCallable, Category = "Attribute|Health")
	void ModifyHealth(float Delta);

	// 恢复满生命
	UFUNCTION(BlueprintCallable, Category = "Attribute|Health")
	void FullHeal();

	// 是否存活
	UFUNCTION(BlueprintPure, Category = "Attribute|Health")
	bool IsAlive() const { return GetHealth() > 0.0f; }

	// 是否死亡
	UFUNCTION(BlueprintPure, Category = "Attribute|Health")
	bool IsDead() const { return GetHealth() <= 0.0f; }

	// ========== 神力值相关 ==========

	// 获取当前神力值
	UFUNCTION(BlueprintPure, Category = "Attribute|Mana")
	float GetMana() const { return Attributes.Mana.GetFinalValue(); }

	// 获取最大神力值
	UFUNCTION(BlueprintPure, Category = "Attribute|Mana")
	float GetMaxMana() const { return Attributes.MaxMana.GetFinalValue(); }

	// 修改神力值
	UFUNCTION(BlueprintCallable, Category = "Attribute|Mana")
	void ModifyMana(float Delta);

	// 是否足够神力
	UFUNCTION(BlueprintPure, Category = "Attribute|Mana")
	bool HasEnoughMana(float Cost) const { return GetMana() >= Cost; }

	// ========== 战斗属性 ==========

	// 获取攻击力
	UFUNCTION(BlueprintPure, Category = "Attribute|Combat")
	float GetAttack() const { return Attributes.Attack.GetFinalValue(); }

	// 获取防御力
	UFUNCTION(BlueprintPure, Category = "Attribute|Combat")
	float GetDefense() const { return Attributes.Defense.GetFinalValue(); }

	// 获取移动速度
	UFUNCTION(BlueprintPure, Category = "Attribute|Combat")
	float GetMoveSpeed() const { return Attributes.MoveSpeed.GetFinalValue(); }

	// ========== 修饰器操作 ==========

	// 添加附加值
	UFUNCTION(BlueprintCallable, Category = "Attribute|Modifier")
	void AddAdditiveValue(EAttributeType Type, float Value);

	// 添加乘数
	UFUNCTION(BlueprintCallable, Category = "Attribute|Modifier")
	void AddMultiplier(EAttributeType Type, float Value);

	// 移除附加值
	UFUNCTION(BlueprintCallable, Category = "Attribute|Modifier")
	void RemoveAdditiveValue(EAttributeType Type, float Value);

	// 移除乘数
	UFUNCTION(BlueprintCallable, Category = "Attribute|Modifier")
	void RemoveMultiplier(EAttributeType Type, float Value);

	// 重置所有修饰器
	UFUNCTION(BlueprintCallable, Category = "Attribute|Modifier")
	void ResetAllModifiers();

	// ========== 事件委托 ==========

	// 任意属性变化
	UPROPERTY(BlueprintAssignable, Category = "Attribute|Events")
	FOnAttributeChanged OnAttributeChanged;

	// 生命值变化
	UPROPERTY(BlueprintAssignable, Category = "Attribute|Events")
	FOnHealthChanged OnHealthChanged;

	// 死亡事件
	UPROPERTY(BlueprintAssignable, Category = "Attribute|Events")
	FOnDeath OnDeath;

protected:
	// 属性集合
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FAttributeSet Attributes;

	// 应用伤害/治疗
	void ApplyHealthChange(float Delta);

	// 处理死亡
	void HandleDeath();
};
