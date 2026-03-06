// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComboTypes.h"
#include "WeaponComboIntegration.generated.h"

class UWeaponManagerComponent;
class UComboDetectorComponent;
class AWeaponBase;

/**
 * 武器连招集成组件
 * 连接武器系统和连招系统，实现连招驱动的武器攻击
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NIUNIU555_API UWeaponComboIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UWeaponComboIntegration();

    // ========== 组件接口 ==========
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // ========== 初始化 ==========

    // 设置武器管理器（必须在BeginPlay前调用）
    UFUNCTION(BlueprintCallable, Category = "武器连招集成")
    void SetWeaponManager(UWeaponManagerComponent* InWeaponManager);

    // 设置连招检测器（必须在BeginPlay前调用）
    UFUNCTION(BlueprintCallable, Category = "武器连招集成")
    void SetComboDetector(UComboDetectorComponent* InComboDetector);

    // ========== 连招注册 ==========

    // 注册轻击连招（从武器配置自动生成）
    UFUNCTION(BlueprintCallable, Category = "武器连招集成")
    void RegisterLightAttackCombo();

    // 注册重击连招（从武器配置自动生成）
    UFUNCTION(BlueprintCallable, Category = "武器连招集成")
    void RegisterHeavyAttackCombo();

    // 注册自定义连招
    UFUNCTION(BlueprintCallable, Category = "武器连招集成")
    void RegisterCustomCombo(const FWeaponComboSequence& ComboConfig);

    // ========== 输入处理 ==========

    // 处理轻击输入
    UFUNCTION(BlueprintCallable, Category = "武器连招集成")
    void ProcessLightAttackInput();

    // 处理重击输入
    UFUNCTION(BlueprintCallable, Category = "武器连招集成")
    void ProcessHeavyAttackInput();

    // ========== 状态查询 ==========

    // 是否正在连招中
    UFUNCTION(BlueprintPure, Category = "武器连招集成")
    bool IsInCombo() const;

    // 获取当前连招阶段
    UFUNCTION(BlueprintPure, Category = "武器连招集成")
    int32 GetCurrentComboStage() const;

    // 获取当前连招ID
    UFUNCTION(BlueprintPure, Category = "武器连招集成")
    FName GetCurrentComboID() const;

    // 检查当前是否可以输入
    UFUNCTION(BlueprintPure, Category = "武器连招集成")
    bool CanAcceptInput() const;

public:
    // ========== 配置参数 ==========

    // 轻击连招ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "武器连招集成|配置")
    FName LightAttackComboID;

    // 重击连招ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "武器连招集成|配置")
    FName HeavyAttackComboID;

    // 是否在武器切换时自动重新注册连招
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "武器连招集成|配置")
    bool bAutoRegisterOnWeaponChange;

protected:
    // ========== 组件引用 ==========

    // 武器管理器
    UPROPERTY()
    TObjectPtr<UWeaponManagerComponent> WeaponManager;

    // 连招检测器
    UPROPERTY()
    TObjectPtr<UComboDetectorComponent> ComboDetector;

protected:
    // ========== 事件处理 ==========

    // 连招阶段推进处理
    UFUNCTION()
    void HandleComboStageAdvanced(FName ComboID, int32 NewStageIndex);

    // 连招完成处理
    UFUNCTION()
    void HandleComboCompleted(FName ComboID, int32 CompletedStageCount);

    // 连招中断处理
    UFUNCTION()
    void HandleComboBroken(FName ComboID, FString Reason);

    // 武器切换处理
    UFUNCTION()
    void HandleWeaponChanged(AWeaponBase* NewWeapon, AWeaponBase* OldWeapon);

protected:
    // ========== 内部方法 ==========

    // 从当前武器配置生成轻击连招
    UFUNCTION()
    FWeaponComboSequence GenerateLightAttackComboFromWeapon() const;

    // 从当前武器配置生成重击连招
    UFUNCTION()
    FWeaponComboSequence GenerateHeavyAttackComboFromWeapon() const;

    // 执行指定阶段的攻击
    UFUNCTION()
    void ExecuteAttackForStage(FName ComboID, int32 StageIndex);

    // 获取当前武器的阶段配置
    UFUNCTION()
    FWeaponComboStageConfig GetStageConfigFromWeapon(FName ComboID, int32 StageIndex) const;

    // 检查组件是否有效
    UFUNCTION()
    bool IsComponentsValid() const;
};
