// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponTypes.h"
#include "WeaponManagerComponent.generated.h"

class AWeaponBase;
class UWeaponDataAsset;

// ==================== 委托定义 ====================

// 武器切换开始委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponSwitchStarted, EWeaponType, FromWeapon, EWeaponType, ToWeapon);

// 武器切换完成委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwitchCompleted, EWeaponType, CurrentWeapon);

// 当前武器改变委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentWeaponChanged, AWeaponBase*, NewWeapon, AWeaponBase*, OldWeapon);

// 攻击输入处理委托（用于连招系统联动）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttackInputReceived, EWeaponAttackInput, AttackType, int32, ComboStage);

/**
 * 武器管理器组件 - 管理角色的所有武器
 * 负责武器切换、当前武器状态管理、攻击输入转发
 * 将此组件添加到Character上即可使用武器系统
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NIUNIU555_API UWeaponManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWeaponManagerComponent();

    // ========== ActorComponent 接口 ==========
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // ========== 武器管理 ==========

    /**
     * 注册武器 - 将武器类与数据资产关联
     * @param WeaponType 武器类型
     * @param WeaponClass 武器Actor类
     * @param WeaponData 武器数据资产（可为空）
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|注册")
    void RegisterWeapon(EWeaponType WeaponType, TSubclassOf<AWeaponBase> WeaponClass, UWeaponDataAsset* WeaponData = nullptr);

    /**
     * 注销武器
     * @param WeaponType 要注销的武器类型
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|注册")
    void UnregisterWeapon(EWeaponType WeaponType);

    /**
     * 获取已注册的武器类
     * @param WeaponType 武器类型
     * @return 武器类，未注册则返回nullptr
     */
    UFUNCTION(BlueprintPure, Category = "武器管理器|查询")
    TSubclassOf<AWeaponBase> GetWeaponClass(EWeaponType WeaponType) const;

    /**
     * 获取已注册的武器数据
     * @param WeaponType 武器类型
     * @return 武器数据资产
     */
    UFUNCTION(BlueprintPure, Category = "武器管理器|查询")
    UWeaponDataAsset* GetWeaponData(EWeaponType WeaponType) const;

    // ========== 武器装备/切换 ==========

    /**
     * 装备指定类型的武器
     * @param WeaponType 要装备的武器类型
     * @return 是否成功开始装备
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|装备")
    bool EquipWeapon(EWeaponType WeaponType);

    /**
     * 卸下当前武器
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|装备")
    void UnequipCurrentWeapon();

    /**
     * 切换到下一个武器
     * @return 是否成功开始切换
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|装备")
    bool SwitchToNextWeapon();

    /**
     * 切换到上一个武器
     * @return 是否成功开始切换
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|装备")
    bool SwitchToPreviousWeapon();

    /**
     * 通过索引直接切换武器
     * @param WeaponIndex 武器索引
     * @return 是否成功开始切换
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|装备")
    bool SwitchToWeaponByIndex(int32 WeaponIndex);

    // ========== 当前武器查询 ==========

    /**
     * 获取当前装备的武器
     * @return 当前武器Actor
     */
    UFUNCTION(BlueprintPure, Category = "武器管理器|当前武器")
    AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

    /**
     * 获取当前武器类型
     * @return 当前武器类型
     */
    UFUNCTION(BlueprintPure, Category = "武器管理器|当前武器")
    EWeaponType GetCurrentWeaponType() const;

    /**
     * 是否已装备武器
     * @return 如果有武器则返回true
     */
    UFUNCTION(BlueprintPure, Category = "武器管理器|当前武器")
    bool HasEquippedWeapon() const { return CurrentWeapon != nullptr; }

    /**
     * 当前是否正在切换武器
     * @return 如果正在切换则返回true
     */
    UFUNCTION(BlueprintPure, Category = "武器管理器|当前武器")
    bool IsSwitchingWeapon() const { return bIsSwitchingWeapon; }

    // ========== 攻击操作（转发给当前武器） ==========

    /**
     * 执行轻击
     * @return 是否成功开始攻击
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|攻击")
    bool PerformLightAttack();

    /**
     * 执行重击
     * @return 是否成功开始攻击
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|攻击")
    bool PerformHeavyAttack();

    /**
     * 开始蓄力攻击
     * @return 是否成功开始蓄力
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|攻击")
    bool StartChargedAttack();

    /**
     * 释放蓄力攻击
     * @return 是否成功释放攻击
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|攻击")
    bool ReleaseChargedAttack();

    /**
     * 取消当前攻击
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|攻击")
    void CancelAttack();

    /**
     * 检查当前是否可以攻击
     * @return 如果可以攻击则返回true
     */
    UFUNCTION(BlueprintPure, Category = "武器管理器|攻击")
    bool CanAttack() const;

    /**
     * 检查当前攻击是否可以取消
     * @return 如果在Cancel Window内则返回true
     */
    UFUNCTION(BlueprintPure, Category = "武器管理器|攻击")
    bool CanCancelAttack() const;

    // ========== 连招相关 ==========

    /**
     * 获取当前连招阶段
     * @return 当前连招阶段索引
     */
    UFUNCTION(BlueprintPure, Category = "武器管理器|连招")
    int32 GetCurrentComboStage() const;

    /**
     * 重置连招
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|连招")
    void ResetCombo();

    /**
     * 推进连招阶段
     */
    UFUNCTION(BlueprintCallable, Category = "武器管理器|连招")
    void AdvanceComboStage();

    // ========== 事件委托（Blueprint可用） ==========

    // 武器切换开始事件 - 可在蓝图中绑定
    UPROPERTY(BlueprintAssignable, Category = "武器管理器|事件")
    FOnWeaponSwitchStarted OnWeaponSwitchStarted;

    // 武器切换完成事件 - 可在蓝图中绑定
    UPROPERTY(BlueprintAssignable, Category = "武器管理器|事件")
    FOnWeaponSwitchCompleted OnWeaponSwitchCompleted;

    // 当前武器改变事件 - 可在蓝图中绑定
    UPROPERTY(BlueprintAssignable, Category = "武器管理器|事件")
    FOnCurrentWeaponChanged OnCurrentWeaponChanged;

    // 攻击输入事件 - 用于与连招系统联动
    UPROPERTY(BlueprintAssignable, Category = "武器管理器|事件")
    FOnAttackInputReceived OnAttackInputReceived;

public:
    // ========== 配置参数 ==========

    // 默认武器类型 - 游戏开始时自动装备
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "武器管理器|配置")
    EWeaponType DefaultWeaponType;

    // 武器切换时间（秒）- 切换动画播放时间
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "武器管理器|配置", meta = (ClampMin = 0))
    float WeaponSwitchTime;

    // 是否自动装备默认武器 - 游戏开始时自动装备
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "武器管理器|配置")
    bool bAutoEquipDefaultWeapon;

    // 武器Actor生成位置偏移
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "武器管理器|配置")
    FVector WeaponSpawnOffset;

protected:
    // ========== 运行时数据 ==========

    // 当前装备的武器实例
    UPROPERTY(BlueprintReadOnly, Category = "武器管理器|运行时")
    TObjectPtr<AWeaponBase> CurrentWeapon;

    // 已注册的武器类映射表
    UPROPERTY()
    TMap<EWeaponType, TSubclassOf<AWeaponBase>> RegisteredWeaponClasses;

    // 已注册的武器数据映射表
    UPROPERTY()
    TMap<EWeaponType, TObjectPtr<UWeaponDataAsset>> RegisteredWeaponData;

    // 已创建的武器实例缓存（用于复用）
    UPROPERTY()
    TMap<EWeaponType, TObjectPtr<AWeaponBase>> WeaponInstanceCache;

    // 当前武器类型索引（用于循环切换）
    UPROPERTY()
    int32 CurrentWeaponIndex;

    // 是否正在切换武器
    UPROPERTY()
    bool bIsSwitchingWeapon;

    // 切换武器计时器
    UPROPERTY()
    float WeaponSwitchTimer;

    // 待切换的武器类型
    UPROPERTY()
    EWeaponType PendingWeaponType;

    // 连招阶段
    UPROPERTY()
    int32 CurrentComboStage;

    // 上次攻击输入类型
    UPROPERTY()
    EWeaponAttackInput LastAttackInput;

protected:
    // ========== 内部方法 ==========

    // 创建武器实例
    UFUNCTION()
    AWeaponBase* CreateWeaponInstance(EWeaponType WeaponType);

    // 销毁武器实例
    UFUNCTION()
    void DestroyWeaponInstance(EWeaponType WeaponType);

    // 执行武器切换
    UFUNCTION()
    void DoWeaponSwitch();

    // 武器切换完成
    UFUNCTION()
    void FinishWeaponSwitch();

    // 获取可用的武器类型列表
    UFUNCTION()
    TArray<EWeaponType> GetAvailableWeaponTypes() const;

    // 绑定武器事件
    UFUNCTION()
    void BindWeaponEvents(AWeaponBase* Weapon);

    // 解绑武器事件
    UFUNCTION()
    void UnbindWeaponEvents(AWeaponBase* Weapon);

    // ========== 事件回调 ==========

    // 武器攻击开始回调
    UFUNCTION()
    void HandleWeaponAttackStarted(EWeaponAttackInput AttackType, int32 ComboStage);

    // 武器攻击命中回调
    UFUNCTION()
    void HandleWeaponAttackHit(const FDamageResult& DamageResult);

    // 武器攻击结束回调
    UFUNCTION()
    void HandleWeaponAttackEnded();

    // 武器状态改变回调
    UFUNCTION()
    void HandleWeaponStateChanged(EWeaponState NewState, EWeaponState OldState);
};
