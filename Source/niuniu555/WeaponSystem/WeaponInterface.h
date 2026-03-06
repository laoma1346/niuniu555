// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponTypes.h"
#include "WeaponInterface.generated.h"

class AWeaponBase;

// ==================== 武器接口委托 ====================

// 武器攻击开始委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponAttackStarted, EWeaponAttackInput, AttackType, int32, ComboStage);

// 武器攻击命中委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponAttackHit, const FDamageResult&, DamageResult);

// 武器攻击结束委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponAttackEnded);

// 武器状态改变委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponStateChanged, EWeaponState, NewState, EWeaponState, OldState);

// ==================== 武器接口 ====================

/**
 * 武器接口 - 定义所有武器必须实现的功能
 * 所有武器类都必须实现此接口
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UWeaponInterface : public UInterface
{
    GENERATED_BODY()
};

class NIUNIU555_API IWeaponInterface
{
    GENERATED_BODY()

public:
    // ========== 基础查询 ==========

    /**
     * 获取武器类型
     * @return 武器类型枚举
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Info")
    EWeaponType GetWeaponType() const;
    virtual EWeaponType GetWeaponType_Implementation() const { return EWeaponType::None; }

    /**
     * 获取武器名称
     * @return 武器显示名称
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Info")
    FText GetWeaponName() const;
    virtual FText GetWeaponName_Implementation() const { return FText::GetEmpty(); }

    /**
     * 获取武器描述
     * @return 武器描述文本
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Info")
    FText GetWeaponDescription() const;
    virtual FText GetWeaponDescription_Implementation() const { return FText::GetEmpty(); }

    // ========== 状态查询 ==========

    /**
     * 获取当前武器状态
     * @return 武器状态
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|State")
    EWeaponState GetWeaponState() const;
    virtual EWeaponState GetWeaponState_Implementation() const { return EWeaponState::Idle; }

    /**
     * 是否可以攻击
     * @return 如果当前可以执行攻击动作则返回true
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|State")
    bool CanAttack() const;
    virtual bool CanAttack_Implementation() const { return true; }

    /**
     * 是否可以切换到此武器
     * @return 如果可以切换则返回true
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|State")
    bool CanEquip() const;
    virtual bool CanEquip_Implementation() const { return true; }

    // ========== 攻击操作 ==========

    /**
     * 执行轻击
     * @param ComboStage 当前连招阶段
     * @return 攻击是否成功开始
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Attack")
    bool PerformLightAttack(int32 ComboStage);
    virtual bool PerformLightAttack_Implementation(int32 ComboStage) { return false; }

    /**
     * 执行重击
     * @param ComboStage 当前连招阶段
     * @return 攻击是否成功开始
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Attack")
    bool PerformHeavyAttack(int32 ComboStage);
    virtual bool PerformHeavyAttack_Implementation(int32 ComboStage) { return false; }

    /**
     * 开始蓄力攻击
     * @return 蓄力是否成功开始
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Attack")
    bool StartChargedAttack();
    virtual bool StartChargedAttack_Implementation() { return false; }

    /**
     * 释放蓄力攻击
     * @param ChargeRatio 蓄力比例 (0.0 - 1.0)
     * @return 攻击是否成功执行
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Attack")
    bool ReleaseChargedAttack(float ChargeRatio);
    virtual bool ReleaseChargedAttack_Implementation(float ChargeRatio) { return false; }

    /**
     * 取消当前攻击
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Attack")
    void CancelAttack();
    virtual void CancelAttack_Implementation() {}

    // ========== 连招相关 ==========

    /**
     * 获取当前连招阶段
     * @return 当前连招阶段索引
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Combo")
    int32 GetCurrentComboStage() const;
    virtual int32 GetCurrentComboStage_Implementation() const { return 0; }

    /**
     * 重置连招
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Combo")
    void ResetCombo();
    virtual void ResetCombo_Implementation() {}

    /**
     * 获取连招最大阶段数
     * @return 最大连招阶段数
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Combo")
    int32 GetMaxComboStages() const;
    virtual int32 GetMaxComboStages_Implementation() const { return 0; }

    // ========== 装备/卸下 ==========

    /**
     * 装备武器
     * @param OwnerCharacter 装备此武器的角色
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Equip")
    void OnEquipped(class ACharacter* OwnerCharacter);
    virtual void OnEquipped_Implementation(class ACharacter* OwnerCharacter) {}

    /**
     * 卸下武器
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Equip")
    void OnUnequipped();
    virtual void OnUnequipped_Implementation() {}

    // ========== 伤害计算 ==========

    /**
     * 计算基础伤害
     * @param BaseDamage 基础伤害值
     * @param DamageType 伤害类型
     * @return 计算后的伤害结果
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Damage")
    FDamageResult CalculateDamage(float BaseDamage, EDamageType DamageType);
    virtual FDamageResult CalculateDamage_Implementation(float BaseDamage, EDamageType DamageType);
};
