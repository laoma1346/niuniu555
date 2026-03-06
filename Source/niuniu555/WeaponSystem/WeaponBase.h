// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponInterface.h"
#include "WeaponTypes.h"
#include "WeaponBase.generated.h"

// 前向声明
class USkeletalMeshComponent;
struct FHitInfo;
// 粒子系统类前向声明
class UParticleSystem;
class USoundBase;
class UAnimMontage;

/**
 * 武器基类 - 所有武器的基类
 * 实现了 IWeaponInterface 接口的基础功能
 */
UCLASS(Abstract, Blueprintable)
class NIUNIU555_API AWeaponBase : public AActor, public IWeaponInterface
{
    GENERATED_BODY()

public:
    AWeaponBase();

    // ========== AActor 接口 ==========
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ========== IWeaponInterface 实现 ==========

    // 基础查询
    virtual EWeaponType GetWeaponType_Implementation() const override { return WeaponType; }
    virtual FText GetWeaponName_Implementation() const override { return WeaponName; }
    virtual FText GetWeaponDescription_Implementation() const override { return WeaponDescription; }

    // 状态查询
    virtual EWeaponState GetWeaponState_Implementation() const override { return CurrentState; }
    virtual bool CanAttack_Implementation() const override;
    virtual bool CanEquip_Implementation() const override { return CurrentState != EWeaponState::Switching; }

    // 攻击操作（子类需要重写）
    virtual bool PerformLightAttack_Implementation(int32 ComboStage) override;
    virtual bool PerformHeavyAttack_Implementation(int32 ComboStage) override;
    virtual bool StartChargedAttack_Implementation() override;
    virtual bool ReleaseChargedAttack_Implementation(float ChargeRatio) override;
    virtual void CancelAttack_Implementation() override;

    // 连招相关
    virtual int32 GetCurrentComboStage_Implementation() const override { return CurrentComboStage; }
    virtual void ResetCombo_Implementation() override;
    virtual int32 GetMaxComboStages_Implementation() const override;

    // 装备/卸下
    virtual void OnEquipped_Implementation(class ACharacter* OwnerCharacter) override;
    virtual void OnUnequipped_Implementation() override;

    // 伤害计算
    virtual FDamageResult CalculateDamage_Implementation(float BaseDamage, EDamageType InDamageType) override;

public:
    // ========== 组件 ==========

    // 武器骨骼网格体组件 - 在蓝图中指定武器模型
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> WeaponMesh;

    // 武器根组件（用于定位）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> WeaponRoot;

public:
    // ========== 武器数据资产（推荐方式） ==========

    // 武器数据资产 - 在蓝图中指定，包含所有武器配置
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|数据资产")
    TObjectPtr<class UWeaponDataAsset> WeaponDataAsset;

    // ========== 武器基础配置（直接配置，优先级低于数据资产） ==========

    // 武器类型 - 在蓝图中指定（会被数据资产覆盖）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|基础信息")
    EWeaponType WeaponType = EWeaponType::None;

    // 武器显示名称 - 在蓝图中指定（会被数据资产覆盖）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|基础信息")
    FText WeaponName;

    // 武器描述 - 在蓝图中指定（会被数据资产覆盖）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|基础信息")
    FText WeaponDescription;

    // 武器品质 - 在蓝图中指定（会被数据资产覆盖）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|基础信息")
    EWeaponRarity WeaponRarity = EWeaponRarity::Common;

    // 武器攻击类型 - 在蓝图中指定
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|基础信息")
    EWeaponAttackType AttackType = EWeaponAttackType::Melee;

    // 武器基础属性 - 在蓝图中配置（会被数据资产覆盖）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|属性")
    FWeaponBaseStats BaseStats;

public:
    // ========== 连招配置 ==========

    // 轻击连招动画蒙太奇 - 在蓝图中指定动画资源
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|连招|轻击")
    TObjectPtr<UAnimMontage> LightAttackMontage;

    // 轻击连招阶段配置 - 在蓝图中配置每段连招参数
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|连招|轻击")
    TArray<FComboStageInfo> LightComboStages;

    // 重击连招动画蒙太奇 - 在蓝图中指定动画资源
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|连招|重击")
    TObjectPtr<UAnimMontage> HeavyAttackMontage;

    // 重击连招阶段配置 - 在蓝图中配置每段连招参数
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|连招|重击")
    TArray<FComboStageInfo> HeavyComboStages;

    // 蓄力攻击动画蒙太奇 - 在蓝图中指定动画资源
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|蓄力")
    TObjectPtr<UAnimMontage> ChargedAttackMontage;

    // 蓄力阶段Section名称 - 在动画蒙太奇中定义的Section
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|蓄力")
    FName ChargeLoopSectionName = FName("ChargeLoop");

    // 蓄力攻击阶段Section名称 - 在动画蒙太奇中定义的Section
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|蓄力")
    FName ChargeAttackSectionName = FName("ChargeAttack");

    // 最大蓄力时间（秒）- 在蓝图中配置
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|蓄力")
    float MaxChargeTime = 2.0f;

public:
    // ========== 特效与音效配置 ==========

    // 攻击命中特效 - 粒子特效
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|特效")
    TObjectPtr<UParticleSystem> HitEffect;

    // 武器轨迹特效 - 攻击时的刀光等
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|特效")
    TObjectPtr<UParticleSystem> TrailEffect;

    // 攻击音效 - 在蓝图中指定音效资源
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|音效")
    TObjectPtr<USoundBase> AttackSound;

    // 命中音效 - 在蓝图中指定音效资源
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|音效")
    TObjectPtr<USoundBase> HitSound;

public:
    // ========== 攻击检测配置 ==========

    // 攻击检测的碰撞通道 - 在项目设置中配置
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|碰撞")
    TEnumAsByte<ECollisionChannel> AttackTraceChannel = ECC_Pawn;

    // 攻击检测起始骨骼名称 - 在骨骼网格体中定义
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|碰撞")
    FName AttackStartBoneName = FName("weapon_start");

    // 攻击检测结束骨骼名称 - 在骨骼网格体中定义
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|碰撞")
    FName AttackEndBoneName = FName("weapon_end");

    // 攻击检测半径 - 胶囊体检测半径
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "武器配置|碰撞")
    float AttackTraceRadius = 30.0f;

public:
    // ========== 事件委托（Blueprint可用） ==========

    // 攻击开始事件 - 可在蓝图中绑定
    UPROPERTY(BlueprintAssignable, Category = "武器事件")
    FOnWeaponAttackStarted OnAttackStarted;

    // 攻击命中事件 - 可在蓝图中绑定
    UPROPERTY(BlueprintAssignable, Category = "武器事件")
    FOnWeaponAttackHit OnAttackHit;

    // 攻击结束事件 - 可在蓝图中绑定
    UPROPERTY(BlueprintAssignable, Category = "武器事件")
    FOnWeaponAttackEnded OnAttackEnded;

    // 武器状态改变事件 - 可在蓝图中绑定
    UPROPERTY(BlueprintAssignable, Category = "武器事件")
    FOnWeaponStateChanged OnStateChanged;

public:
    // ========== 公共方法 ==========

    // 获取武器拥有者
    UFUNCTION(BlueprintPure, Category = "Weapon")
    ACharacter* GetWeaponOwner() const { return WeaponOwner; }

    // 获取当前连招阶段信息
    UFUNCTION(BlueprintPure, Category = "Weapon|Combo")
    FComboStageInfo GetCurrentComboStageInfo() const;

    // 设置武器状态
    UFUNCTION(BlueprintCallable, Category = "Weapon|State")
    void SetWeaponState(EWeaponState NewState);

    // 执行攻击检测（由动画通知调用）
    UFUNCTION(BlueprintCallable, Category = "Weapon|Attack")
    void PerformAttackTrace();

    // 通知攻击结束（由动画通知调用）
    UFUNCTION(BlueprintCallable, Category = "Weapon|Attack")
    void NotifyAttackEnded();

    // 检查是否可以取消当前攻击（Cancel Window）
    UFUNCTION(BlueprintPure, Category = "Weapon|Attack")
    bool CanCancelAttack() const;

protected:
    // ========== 运行时状态 ==========

    // 当前武器状态
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
    EWeaponState CurrentState = EWeaponState::Idle;

    // 武器拥有者
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    TObjectPtr<ACharacter> WeaponOwner = nullptr;

    // 当前连招阶段
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Combo")
    int32 CurrentComboStage = 0;

    // 当前蓄力比例 (0.0 - 1.0)
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Charge")
    float CurrentChargeRatio = 0.0f;

    // 蓄力计时器
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Charge")
    float ChargeTimer = 0.0f;

    // 是否正在蓄力
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Charge")
    bool bIsCharging = false;

    // 当前攻击已命中的Actor列表（防止重复伤害）
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> HitActorsThisAttack;

    // 攻击开始时间（用于计算Cancel Window）
    UPROPERTY()
    float AttackStartTime = 0.0f;

    // 冷却计时器
    UPROPERTY()
    float CooldownTimer = 0.0f;

protected:
    // ========== 数据加载 ==========

    // 从数据资产加载武器配置
    UFUNCTION()
    virtual void LoadConfigFromDataAsset();

    // ========== 虚函数（子类可重写） ==========

    // 播放攻击动画
    virtual bool PlayAttackMontage(UAnimMontage* Montage, FName SectionName = NAME_None);

    // 停止攻击动画
    virtual void StopAttackMontage();

    // 执行实际的伤害应用
    virtual void ApplyDamageToActor(AActor* TargetActor, const FDamageResult& DamageInfo);

    // 生成命中特效
    virtual void SpawnHitEffect(const FVector& Location, const FVector& Normal);

    // 播放命中音效
    virtual void PlayHitSound(const FVector& Location);

    // 进入蓄力状态
    virtual void EnterChargingState();

    // 更新蓄力
    virtual void UpdateCharging(float DeltaTime);

    // 退出蓄力状态
    virtual void ExitChargingState();

    // 连招阶段推进
    virtual void AdvanceComboStage();

    // 重置连招状态
    virtual void ResetComboState();

    // 动画蒙太奇结束回调
    UFUNCTION()
    virtual void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
    // 动画实例引用
    UPROPERTY()
    TObjectPtr<UAnimInstance> OwnerAnimInstance = nullptr;

    // 动画结束委托
    FOnMontageEnded OnMontageEndedDelegate;
};
