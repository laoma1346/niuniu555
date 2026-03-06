// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponComboIntegration.h"
#include "WeaponManagerComponent.h"
#include "ComboDetectorComponent.h"
#include "WeaponBase.h"
#include "WeaponInterface.h"

UWeaponComboIntegration::UWeaponComboIntegration()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(false);

    LightAttackComboID = FName("LightAttackCombo");
    HeavyAttackComboID = FName("HeavyAttackCombo");
    bAutoRegisterOnWeaponChange = true;
}

void UWeaponComboIntegration::BeginPlay()
{
    Super::BeginPlay();

    // 自动查找组件
    if (WeaponManager == nullptr)
    {
        WeaponManager = Cast<UWeaponManagerComponent>(GetOwner()->GetComponentByClass(UWeaponManagerComponent::StaticClass()));
    }

    if (ComboDetector == nullptr)
    {
        ComboDetector = Cast<UComboDetectorComponent>(GetOwner()->GetComponentByClass(UComboDetectorComponent::StaticClass()));
    }

    // 绑定事件
    if (ComboDetector != nullptr)
    {
        ComboDetector->OnComboStageAdvanced.AddDynamic(this, &UWeaponComboIntegration::HandleComboStageAdvanced);
        ComboDetector->OnComboDetected.AddDynamic(this, &UWeaponComboIntegration::HandleComboCompleted);
        ComboDetector->OnComboBroken.AddDynamic(this, &UWeaponComboIntegration::HandleComboBroken);
    }

    if (WeaponManager != nullptr)
    {
        WeaponManager->OnCurrentWeaponChanged.AddDynamic(this, &UWeaponComboIntegration::HandleWeaponChanged);
    }

    // 注册默认连招
    if (IsComponentsValid())
    {
        RegisterLightAttackCombo();
        RegisterHeavyAttackCombo();
    }
}

void UWeaponComboIntegration::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

// ========== 初始化 ==========

void UWeaponComboIntegration::SetWeaponManager(UWeaponManagerComponent* InWeaponManager)
{
    WeaponManager = InWeaponManager;
}

void UWeaponComboIntegration::SetComboDetector(UComboDetectorComponent* InComboDetector)
{
    ComboDetector = InComboDetector;
}

// ========== 连招注册 ==========

void UWeaponComboIntegration::RegisterLightAttackCombo()
{
    if (!IsComponentsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 注册轻击连招失败 - 组件无效"), *GetName());
        return;
    }

    FWeaponComboSequence LightCombo = GenerateLightAttackComboFromWeapon();
    if (LightCombo.IsValidConfig())
    {
        ComboDetector->RegisterCombo(LightCombo);
        UE_LOG(LogTemp, Log, TEXT("[%s] 注册轻击连招: %s"), *GetName(), *LightCombo.GetSequenceString());
    }
}

void UWeaponComboIntegration::RegisterHeavyAttackCombo()
{
    if (!IsComponentsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 注册重击连招失败 - 组件无效"), *GetName());
        return;
    }

    FWeaponComboSequence HeavyCombo = GenerateHeavyAttackComboFromWeapon();
    if (HeavyCombo.IsValidConfig())
    {
        ComboDetector->RegisterCombo(HeavyCombo);
        UE_LOG(LogTemp, Log, TEXT("[%s] 注册重击连招: %s"), *GetName(), *HeavyCombo.GetSequenceString());
    }
}

void UWeaponComboIntegration::RegisterCustomCombo(const FWeaponComboSequence& ComboConfig)
{
    if (!IsComponentsValid())
    {
        return;
    }

    ComboDetector->RegisterCombo(ComboConfig);
}

// ========== 输入处理 ==========

void UWeaponComboIntegration::ProcessLightAttackInput()
{
    if (!IsComponentsValid())
    {
        return;
    }

    // 直接调用连招检测器的输入处理
    ComboDetector->AddComboInput(EWeaponComboInputType::Light);
}

void UWeaponComboIntegration::ProcessHeavyAttackInput()
{
    if (!IsComponentsValid())
    {
        return;
    }

    // 直接调用连招检测器的输入处理
    ComboDetector->AddComboInput(EWeaponComboInputType::Heavy);
}

// ========== 状态查询 ==========

bool UWeaponComboIntegration::IsInCombo() const
{
    return ComboDetector != nullptr && ComboDetector->IsInCombo();
}

int32 UWeaponComboIntegration::GetCurrentComboStage() const
{
    return ComboDetector != nullptr ? ComboDetector->GetCurrentStageIndex() : 0;
}

FName UWeaponComboIntegration::GetCurrentComboID() const
{
    return ComboDetector != nullptr ? ComboDetector->GetCurrentComboID() : NAME_None;
}

bool UWeaponComboIntegration::CanAcceptInput() const
{
    if (!IsComponentsValid())
    {
        return false;
    }

    // 检查武器是否可以攻击
    if (!WeaponManager->CanAttack())
    {
        return false;
    }

    return true;
}

// ========== 事件处理 ==========

void UWeaponComboIntegration::HandleComboStageAdvanced(FName ComboID, int32 NewStageIndex)
{
    // 执行对应阶段的攻击
    ExecuteAttackForStage(ComboID, NewStageIndex);
}

void UWeaponComboIntegration::HandleComboCompleted(FName ComboID, int32 CompletedStageCount)
{
    UE_LOG(LogTemp, Log, TEXT("[%s] 连招完成: %s, 完成 %d 阶段"), 
        *GetName(), *ComboID.ToString(), CompletedStageCount);
}

void UWeaponComboIntegration::HandleComboBroken(FName ComboID, FString Reason)
{
    UE_LOG(LogTemp, Log, TEXT("[%s] 连招中断: %s, 原因: %s"), 
        *GetName(), *ComboID.ToString(), *Reason);
}

void UWeaponComboIntegration::HandleWeaponChanged(AWeaponBase* NewWeapon, AWeaponBase* OldWeapon)
{
    if (bAutoRegisterOnWeaponChange && NewWeapon != nullptr)
    {
        // 清空旧连招
        if (ComboDetector != nullptr)
        {
            ComboDetector->ClearAllCombos();
        }

        // 重新注册新武器的连招
        RegisterLightAttackCombo();
        RegisterHeavyAttackCombo();
    }
}

// ========== 内部方法 ==========

FWeaponComboSequence UWeaponComboIntegration::GenerateLightAttackComboFromWeapon() const
{
    FWeaponComboSequence Combo;
    Combo.ComboID = LightAttackComboID;
    Combo.ComboName = FText::FromString(TEXT("轻击连招"));
    Combo.bResetAfterComplete = true;
    Combo.CooldownAfterComplete = 0.5f;

    // 从当前武器获取轻击连招配置
    AWeaponBase* CurrentWeapon = WeaponManager->GetCurrentWeapon();
    if (CurrentWeapon != nullptr)
    {
        // 获取轻击连招阶段配置数组
        const TArray<FComboStageInfo>& Stages = CurrentWeapon->LightComboStages;
        
        for (int32 i = 0; i < Stages.Num(); ++i)
        {
            // 添加输入序列
            Combo.InputSequence.Add(EWeaponComboInputType::Light);

            // 从武器阶段配置创建连招阶段配置
            FWeaponComboStageConfig StageConfig;
            StageConfig.StageIndex = i;
            StageConfig.StageName = FString::Printf(TEXT("轻击%d"), i + 1);
            StageConfig.AnimationSectionName = Stages[i].StageName; // 使用武器配置中的Section名称
            StageConfig.DamageMultiplier = Stages[i].DamageMultiplier;
            StageConfig.bCanCancel = Stages[i].bCanCancel;
            StageConfig.CancelWindowStart = Stages[i].CancelWindowStart;
            StageConfig.CancelWindowEnd = Stages[i].CancelWindowEnd;
            StageConfig.InputToleranceTime = 0.5f;

            Combo.StageConfigs.Add(StageConfig);
        }

        UE_LOG(LogTemp, Log, TEXT("[%s] 生成轻击连招: %d段"), *GetName(), Stages.Num());
    }
    else
    {
        // 默认3段轻击连招（武器未装备时的回退）
        for (int32 i = 0; i < 3; ++i)
        {
            Combo.InputSequence.Add(EWeaponComboInputType::Light);

            FWeaponComboStageConfig StageConfig;
            StageConfig.StageIndex = i;
            StageConfig.StageName = FString::Printf(TEXT("轻击%d"), i + 1);
            StageConfig.AnimationSectionName = FName(*FString::Printf(TEXT("LightAttack%d"), i + 1));
            StageConfig.DamageMultiplier = 1.0f + (i * 0.2f);
            StageConfig.bCanCancel = true;
            StageConfig.InputToleranceTime = 0.5f;

            Combo.StageConfigs.Add(StageConfig);
        }
    }

    return Combo;
}

FWeaponComboSequence UWeaponComboIntegration::GenerateHeavyAttackComboFromWeapon() const
{
    FWeaponComboSequence Combo;
    Combo.ComboID = HeavyAttackComboID;
    Combo.ComboName = FText::FromString(TEXT("重击连招"));
    Combo.bResetAfterComplete = true;
    Combo.CooldownAfterComplete = 0.8f;

    // 从当前武器获取重击连招配置
    AWeaponBase* CurrentWeapon = WeaponManager->GetCurrentWeapon();
    if (CurrentWeapon != nullptr)
    {
        // 获取重击连招阶段配置数组
        const TArray<FComboStageInfo>& Stages = CurrentWeapon->HeavyComboStages;
        
        for (int32 i = 0; i < Stages.Num(); ++i)
        {
            Combo.InputSequence.Add(EWeaponComboInputType::Heavy);

            FWeaponComboStageConfig StageConfig;
            StageConfig.StageIndex = i;
            StageConfig.StageName = FString::Printf(TEXT("重击%d"), i + 1);
            StageConfig.AnimationSectionName = Stages[i].StageName; // 使用武器配置中的Section名称
            StageConfig.DamageMultiplier = Stages[i].DamageMultiplier;
            StageConfig.bCanCancel = Stages[i].bCanCancel;
            StageConfig.CancelWindowStart = Stages[i].CancelWindowStart;
            StageConfig.CancelWindowEnd = Stages[i].CancelWindowEnd;
            StageConfig.InputToleranceTime = 0.6f;

            Combo.StageConfigs.Add(StageConfig);
        }

        UE_LOG(LogTemp, Log, TEXT("[%s] 生成重击连招: %d段"), *GetName(), Stages.Num());
    }
    else
    {
        // 默认2段重击连招
        for (int32 i = 0; i < 2; ++i)
        {
            Combo.InputSequence.Add(EWeaponComboInputType::Heavy);

            FWeaponComboStageConfig StageConfig;
            StageConfig.StageIndex = i;
            StageConfig.StageName = FString::Printf(TEXT("重击%d"), i + 1);
            StageConfig.AnimationSectionName = FName(*FString::Printf(TEXT("HeavyAttack%d"), i + 1));
            StageConfig.DamageMultiplier = 1.5f + (i * 0.3f);
            StageConfig.bCanCancel = false;
            StageConfig.InputToleranceTime = 0.6f;

            Combo.StageConfigs.Add(StageConfig);
        }
    }

    return Combo;
}

void UWeaponComboIntegration::ExecuteAttackForStage(FName ComboID, int32 StageIndex)
{
    if (!IsComponentsValid())
    {
        return;
    }

    // 根据连招ID决定攻击类型
    if (ComboID == LightAttackComboID)
    {
        WeaponManager->PerformLightAttack();
    }
    else if (ComboID == HeavyAttackComboID)
    {
        WeaponManager->PerformHeavyAttack();
    }
    else
    {
        // 自定义连招，默认使用轻击
        WeaponManager->PerformLightAttack();
    }
}

FWeaponComboStageConfig UWeaponComboIntegration::GetStageConfigFromWeapon(FName ComboID, int32 StageIndex) const
{
    // 从武器获取阶段配置
    AWeaponBase* CurrentWeapon = WeaponManager->GetCurrentWeapon();
    if (CurrentWeapon == nullptr)
    {
        return FWeaponComboStageConfig();
    }

    // TODO: 从武器数据资产读取具体配置
    return FWeaponComboStageConfig();
}

bool UWeaponComboIntegration::IsComponentsValid() const
{
    return WeaponManager != nullptr && ComboDetector != nullptr;
}
