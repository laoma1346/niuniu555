// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponManagerComponent.h"
#include "WeaponBase.h"
#include "WeaponDataAsset.h"
#include "GameFramework/Character.h"

UWeaponManagerComponent::UWeaponManagerComponent()
{
    // 启用每帧Tick
    PrimaryComponentTick.bCanEverTick = true;

    // 启用复制
    SetIsReplicatedByDefault(true);

    // 设置默认值
    DefaultWeaponType = EWeaponType::None;
    WeaponSwitchTime = 0.5f;
    bAutoEquipDefaultWeapon = true;
    WeaponSpawnOffset = FVector::ZeroVector;

    CurrentWeapon = nullptr;
    CurrentWeaponIndex = 0;
    bIsSwitchingWeapon = false;
    WeaponSwitchTimer = 0.0f;
    PendingWeaponType = EWeaponType::None;
    CurrentComboStage = 0;
    LastAttackInput = EWeaponAttackInput::LightAttack;
}

void UWeaponManagerComponent::BeginPlay()
    {
    Super::BeginPlay();

    // 自动装备默认武器
    if (bAutoEquipDefaultWeapon && DefaultWeaponType != EWeaponType::None)
    {
        EquipWeapon(DefaultWeaponType);
    }
}

void UWeaponManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 更新武器切换计时
    if (bIsSwitchingWeapon)
    {
        WeaponSwitchTimer -= DeltaTime;
        if (WeaponSwitchTimer <= 0.0f)
        {
            DoWeaponSwitch();  // 执行实际的武器切换逻辑
        }
    }
}

void UWeaponManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 卸下当前武器
    UnequipCurrentWeapon();

    // 销毁所有缓存的武器实例
    for (auto& Pair : WeaponInstanceCache)
    {
        if (Pair.Value != nullptr && IsValid(Pair.Value))
        {
            Pair.Value->Destroy();
        }
    }
    WeaponInstanceCache.Empty();

    Super::EndPlay(EndPlayReason);
}

// ========== 武器管理 ==========

void UWeaponManagerComponent::RegisterWeapon(EWeaponType WeaponType, TSubclassOf<AWeaponBase> WeaponClass, UWeaponDataAsset* WeaponData)
{
    if (WeaponClass == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 注册武器失败 - WeaponClass为空"), *GetName());
        return;
    }

    RegisteredWeaponClasses.Add(WeaponType, WeaponClass);
    
    if (WeaponData != nullptr)
    {
        RegisteredWeaponData.Add(WeaponType, WeaponData);
    }

    UE_LOG(LogTemp, Log, TEXT("[%s] 注册武器成功 - 类型: %s, 类: %s"), 
        *GetName(), *UEnum::GetValueAsString(WeaponType), *WeaponClass->GetName());
}

void UWeaponManagerComponent::UnregisterWeapon(EWeaponType WeaponType)
{
    // 如果当前装备的是这个武器，先卸下
    if (GetCurrentWeaponType() == WeaponType)
    {
        UnequipCurrentWeapon();
    }

    // 销毁缓存的实例
    DestroyWeaponInstance(WeaponType);

    // 从注册表中移除
    RegisteredWeaponClasses.Remove(WeaponType);
    RegisteredWeaponData.Remove(WeaponType);

    UE_LOG(LogTemp, Log, TEXT("[%s] 注销武器 - 类型: %s"), 
        *GetName(), *UEnum::GetValueAsString(WeaponType));
}

TSubclassOf<AWeaponBase> UWeaponManagerComponent::GetWeaponClass(EWeaponType WeaponType) const
{
    if (const TSubclassOf<AWeaponBase>* FoundClass = RegisteredWeaponClasses.Find(WeaponType))
    {
        return *FoundClass;
    }
    return nullptr;
}

UWeaponDataAsset* UWeaponManagerComponent::GetWeaponData(EWeaponType WeaponType) const
{
    if (const TObjectPtr<UWeaponDataAsset>* FoundData = RegisteredWeaponData.Find(WeaponType))
    {
        return *FoundData;
    }
    return nullptr;
}

// ========== 武器装备/切换 ==========

bool UWeaponManagerComponent::EquipWeapon(EWeaponType WeaponType)
{
    // 检查是否已经装备了这个武器
    if (GetCurrentWeaponType() == WeaponType)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 装备武器失败 - 已经装备了该武器"), *GetName());
        return false;
    }

    // 检查武器是否已注册
    TSubclassOf<AWeaponBase> WeaponClass = GetWeaponClass(WeaponType);
    if (WeaponClass == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 装备武器失败 - 未找到类型为 %s 的注册武器"), 
            *GetName(), *UEnum::GetValueAsString(WeaponType));
        return false;
    }

    // 如果正在切换中，不允许再次切换
    if (bIsSwitchingWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 装备武器失败 - 正在切换武器中"), *GetName());
        return false;
    }

    // 广播切换开始事件
    EWeaponType FromWeapon = GetCurrentWeaponType();
    OnWeaponSwitchStarted.Broadcast(FromWeapon, WeaponType);

    // 如果没有当前武器，立即切换
    if (CurrentWeapon == nullptr)
    {
        PendingWeaponType = WeaponType;
        DoWeaponSwitch();
        return true;
    }

    // 开始切换计时
    bIsSwitchingWeapon = true;
    WeaponSwitchTimer = WeaponSwitchTime;
    PendingWeaponType = WeaponType;

    UE_LOG(LogTemp, Log, TEXT("[%s] 开始切换武器 - 从 %s 到 %s, 切换时间: %.2f秒"), 
        *GetName(), *UEnum::GetValueAsString(FromWeapon), 
        *UEnum::GetValueAsString(WeaponType), WeaponSwitchTime);

    return true;
}

void UWeaponManagerComponent::UnequipCurrentWeapon()
{
    if (CurrentWeapon == nullptr)
    {
        return;
    }

    // 解绑事件
    UnbindWeaponEvents(CurrentWeapon);

    // 调用武器的卸下方法（使用 Execute_ 前缀）
    IWeaponInterface::Execute_OnUnequipped(CurrentWeapon);

    // 隐藏武器（不销毁，缓存起来复用）
    CurrentWeapon->SetActorHiddenInGame(true);
    CurrentWeapon->SetActorEnableCollision(false);

    // 缓存武器实例
    EWeaponType WeaponType = IWeaponInterface::Execute_GetWeaponType(CurrentWeapon);
    if (WeaponType != EWeaponType::None)
    {
        WeaponInstanceCache.Add(WeaponType, CurrentWeapon);
    }

    AWeaponBase* OldWeapon = CurrentWeapon;
    CurrentWeapon = nullptr;
    CurrentComboStage = 0;

    // 广播武器改变事件
    OnCurrentWeaponChanged.Broadcast(nullptr, OldWeapon);

    UE_LOG(LogTemp, Log, TEXT("[%s] 卸下当前武器"), *GetName());
}

bool UWeaponManagerComponent::SwitchToNextWeapon()
{
    TArray<EWeaponType> AvailableTypes = GetAvailableWeaponTypes();
    if (AvailableTypes.Num() <= 1)
    {
        return false;
    }

    // 找到当前索引
    int32 CurrentIndex = AvailableTypes.IndexOfByKey(GetCurrentWeaponType());
    if (CurrentIndex == INDEX_NONE)
    {
        CurrentIndex = -1;
    }

    // 计算下一个索引
    int32 NextIndex = (CurrentIndex + 1) % AvailableTypes.Num();
    return EquipWeapon(AvailableTypes[NextIndex]);
}

bool UWeaponManagerComponent::SwitchToPreviousWeapon()
{
    TArray<EWeaponType> AvailableTypes = GetAvailableWeaponTypes();
    if (AvailableTypes.Num() <= 1)
    {
        return false;
    }

    // 找到当前索引
    int32 CurrentIndex = AvailableTypes.IndexOfByKey(GetCurrentWeaponType());
    if (CurrentIndex == INDEX_NONE)
    {
        CurrentIndex = 0;
    }

    // 计算上一个索引
    int32 PrevIndex = (CurrentIndex - 1 + AvailableTypes.Num()) % AvailableTypes.Num();
    return EquipWeapon(AvailableTypes[PrevIndex]);
}

bool UWeaponManagerComponent::SwitchToWeaponByIndex(int32 WeaponIndex)
{
    TArray<EWeaponType> AvailableTypes = GetAvailableWeaponTypes();
    if (!AvailableTypes.IsValidIndex(WeaponIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 切换武器失败 - 索引 %d 超出范围"), 
            *GetName(), WeaponIndex);
        return false;
    }

    return EquipWeapon(AvailableTypes[WeaponIndex]);
}

// ========== 当前武器查询 ==========

EWeaponType UWeaponManagerComponent::GetCurrentWeaponType() const
{
    if (CurrentWeapon != nullptr)
    {
        return IWeaponInterface::Execute_GetWeaponType(CurrentWeapon);
    }
    return EWeaponType::None;
}

// ========== 攻击操作 ==========

bool UWeaponManagerComponent::PerformLightAttack()
    {
    if (CurrentWeapon == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 轻击失败 - 没有装备武器"), *GetName());
        return false;
    }

    // 广播攻击输入事件
    OnAttackInputReceived.Broadcast(EWeaponAttackInput::LightAttack, CurrentComboStage);
    LastAttackInput = EWeaponAttackInput::LightAttack;

    // 转发给武器
    bool bSuccess = IWeaponInterface::Execute_PerformLightAttack(CurrentWeapon, CurrentComboStage);
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("[%s] 执行轻击 - 阶段: %d"), *GetName(), CurrentComboStage);
    }

    return bSuccess;
}

bool UWeaponManagerComponent::PerformHeavyAttack()
{
    if (CurrentWeapon == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 重击失败 - 没有装备武器"), *GetName());
        return false;
    }

    // 广播攻击输入事件
    OnAttackInputReceived.Broadcast(EWeaponAttackInput::HeavyAttack, CurrentComboStage);
    LastAttackInput = EWeaponAttackInput::HeavyAttack;

    // 转发给武器
    bool bSuccess = IWeaponInterface::Execute_PerformHeavyAttack(CurrentWeapon, CurrentComboStage);
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("[%s] 执行重击 - 阶段: %d"), *GetName(), CurrentComboStage);
    }

    return bSuccess;
}

bool UWeaponManagerComponent::StartChargedAttack()
{
    if (CurrentWeapon == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 开始蓄力失败 - 没有装备武器"), *GetName());
        return false;
    }

    bool bSuccess = IWeaponInterface::Execute_StartChargedAttack(CurrentWeapon);
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("[%s] 开始蓄力攻击"), *GetName());
    }

    return bSuccess;
}

bool UWeaponManagerComponent::ReleaseChargedAttack()
{
    if (CurrentWeapon == nullptr)
    {
        return false;
    }

    bool bSuccess = IWeaponInterface::Execute_ReleaseChargedAttack(CurrentWeapon, 0.0f); // 蓄力比例由武器自己维护
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("[%s] 释放蓄力攻击"), *GetName());
    }

    return bSuccess;
}

void UWeaponManagerComponent::CancelAttack()
{
    if (CurrentWeapon == nullptr)
    {
        return;
    }

    IWeaponInterface::Execute_CancelAttack(CurrentWeapon);
    UE_LOG(LogTemp, Log, TEXT("[%s] 取消攻击"), *GetName());
}

bool UWeaponManagerComponent::CanAttack() const
{
    if (CurrentWeapon == nullptr)
    {
        return false;
    }

    return IWeaponInterface::Execute_CanAttack(CurrentWeapon);
}

bool UWeaponManagerComponent::CanCancelAttack() const
{
    if (CurrentWeapon == nullptr)
    {
        return false;
    }

    return CurrentWeapon->CanCancelAttack();
}

// ========== 连招相关 ==========

int32 UWeaponManagerComponent::GetCurrentComboStage() const
{
    if (CurrentWeapon != nullptr)
    {
        return IWeaponInterface::Execute_GetCurrentComboStage(CurrentWeapon);
    }
    return 0;
}

void UWeaponManagerComponent::ResetCombo()
{
    if (CurrentWeapon != nullptr)
    {
        IWeaponInterface::Execute_ResetCombo(CurrentWeapon);
    }
    CurrentComboStage = 0;
}

void UWeaponManagerComponent::AdvanceComboStage()
{
    if (CurrentWeapon != nullptr)
    {
        // 推进连招阶段
        int32 MaxStages = IWeaponInterface::Execute_GetMaxComboStages(CurrentWeapon);
        CurrentComboStage = (CurrentComboStage + 1) % MaxStages;
    }
}

// ========== 内部方法 ==========

AWeaponBase* UWeaponManagerComponent::CreateWeaponInstance(EWeaponType WeaponType)
{
    // 检查是否已有缓存实例
    TObjectPtr<AWeaponBase>* CachedInstance = WeaponInstanceCache.Find(WeaponType);
    if (CachedInstance != nullptr && CachedInstance->Get() != nullptr && IsValid(CachedInstance->Get()))
    {
        UE_LOG(LogTemp, Log, TEXT("[%s] 使用缓存的武器实例 - 类型: %s"), 
            *GetName(), *UEnum::GetValueAsString(WeaponType));
        return CachedInstance->Get();
    }

    // 获取武器类
    TSubclassOf<AWeaponBase> WeaponClass = GetWeaponClass(WeaponType);
    if (WeaponClass == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] 创建武器实例失败 - 未找到类型 %s 的注册类"), 
            *GetName(), *UEnum::GetValueAsString(WeaponType));
        return nullptr;
    }

    // 获取拥有者（必须是Character）
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] 创建武器实例失败 - Owner不是Character"), *GetName());
        return nullptr;
    }

    // 生成武器Actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerCharacter;
    SpawnParams.Instigator = OwnerCharacter;
    
    FVector SpawnLocation = OwnerCharacter->GetActorLocation() + WeaponSpawnOffset;
    FRotator SpawnRotation = OwnerCharacter->GetActorRotation();

    AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(
        WeaponClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (NewWeapon == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] 生成武器Actor失败"), *GetName());
        return nullptr;
    }

    // 初始状态隐藏
    NewWeapon->SetActorHiddenInGame(true);

    UE_LOG(LogTemp, Log, TEXT("[%s] 创建武器实例成功 - 类型: %s, 名称: %s"), 
        *GetName(), *UEnum::GetValueAsString(WeaponType), *NewWeapon->GetName());

    return NewWeapon;
}

void UWeaponManagerComponent::DestroyWeaponInstance(EWeaponType WeaponType)
{
    TObjectPtr<AWeaponBase>* CachedInstance = WeaponInstanceCache.Find(WeaponType);
    if (CachedInstance != nullptr && CachedInstance->Get() != nullptr && IsValid(CachedInstance->Get()))
    {
        CachedInstance->Get()->Destroy();
    }
    WeaponInstanceCache.Remove(WeaponType);
}

void UWeaponManagerComponent::DoWeaponSwitch()
{
    // 保存旧武器引用（在卸下之前保存）
    AWeaponBase* OldWeapon = CurrentWeapon;

    // 卸下当前武器
    if (CurrentWeapon != nullptr)
    {
        // 注意：UnequipCurrentWeapon 会将 CurrentWeapon 设为 nullptr
        UnequipCurrentWeapon();
    }

    // 创建或获取新武器实例
    AWeaponBase* NewWeapon = CreateWeaponInstance(PendingWeaponType);
    if (NewWeapon == nullptr)
    {
        bIsSwitchingWeapon = false;
        // 广播事件，通知切换失败
        OnCurrentWeaponChanged.Broadcast(nullptr, OldWeapon);
        return;
    }

    // 装备新武器
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter != nullptr)
    {
        // 使用 Execute_ 前缀调用蓝图可实现的接口函数
        IWeaponInterface::Execute_OnEquipped(NewWeapon, OwnerCharacter);
    }
    NewWeapon->SetActorHiddenInGame(false);
    NewWeapon->SetActorEnableCollision(true);

    // 绑定事件
    BindWeaponEvents(NewWeapon);

    // 设置当前武器（必须在 FinishWeaponSwitch 之前）
    CurrentWeapon = NewWeapon;

    // 更新当前索引
    TArray<EWeaponType> AvailableTypes = GetAvailableWeaponTypes();
    CurrentWeaponIndex = AvailableTypes.IndexOfByKey(PendingWeaponType);

    // 重置连招
    CurrentComboStage = 0;

    // 完成武器切换流程（广播事件等）
    FinishWeaponSwitch();

    UE_LOG(LogTemp, Log, TEXT("[%s] 武器切换执行 - 新武器: %s, 类型: %s"), 
        *GetName(), 
        *IWeaponInterface::Execute_GetWeaponName(NewWeapon).ToString(),
        *UEnum::GetValueAsString(IWeaponInterface::Execute_GetWeaponType(NewWeapon)));
}

void UWeaponManagerComponent::FinishWeaponSwitch()
{
    bIsSwitchingWeapon = false;
    WeaponSwitchTimer = 0.0f;

    // 广播切换完成事件
    EWeaponType CurrentType = GetCurrentWeaponType();
    OnWeaponSwitchCompleted.Broadcast(CurrentType);
    
    // 广播武器改变事件 - 传入新武器和旧武器（旧武器可能为nullptr）
    // 注意：由于 UnequipCurrentWeapon 已经将旧武器缓存，这里新武器是 CurrentWeapon
    // 为了保持一致性，我们在 DoWeaponSwitch 中保存了旧武器引用，但事件委托签名是 (NewWeapon, OldWeapon)
    // 所以这里传入 (CurrentWeapon, nullptr) 表示新武器已装备，旧武器已卸下
    OnCurrentWeaponChanged.Broadcast(CurrentWeapon, nullptr);

    UE_LOG(LogTemp, Log, TEXT("[%s] 武器切换完成 - 当前武器类型: %s, 武器名称: %s"), 
        *GetName(), 
        *UEnum::GetValueAsString(CurrentType),
        CurrentWeapon != nullptr ? *IWeaponInterface::Execute_GetWeaponName(CurrentWeapon).ToString() : TEXT("无"));
}

TArray<EWeaponType> UWeaponManagerComponent::GetAvailableWeaponTypes() const
{
    TArray<EWeaponType> Types;
    RegisteredWeaponClasses.GetKeys(Types);
    return Types;
}

void UWeaponManagerComponent::BindWeaponEvents(AWeaponBase* Weapon)
{
    if (Weapon == nullptr)
    {
        return;
    }

    // 绑定武器事件到管理器的处理方法
    Weapon->OnAttackStarted.AddDynamic(this, &UWeaponManagerComponent::HandleWeaponAttackStarted);
    Weapon->OnAttackHit.AddDynamic(this, &UWeaponManagerComponent::HandleWeaponAttackHit);
    Weapon->OnAttackEnded.AddDynamic(this, &UWeaponManagerComponent::HandleWeaponAttackEnded);
    Weapon->OnStateChanged.AddDynamic(this, &UWeaponManagerComponent::HandleWeaponStateChanged);
}

void UWeaponManagerComponent::UnbindWeaponEvents(AWeaponBase* Weapon)
{
    if (Weapon == nullptr)
    {
        return;
    }

    // 解绑事件
    Weapon->OnAttackStarted.RemoveDynamic(this, &UWeaponManagerComponent::HandleWeaponAttackStarted);
    Weapon->OnAttackHit.RemoveDynamic(this, &UWeaponManagerComponent::HandleWeaponAttackHit);
    Weapon->OnAttackEnded.RemoveDynamic(this, &UWeaponManagerComponent::HandleWeaponAttackEnded);
    Weapon->OnStateChanged.RemoveDynamic(this, &UWeaponManagerComponent::HandleWeaponStateChanged);
}

// ========== 事件回调 ==========

void UWeaponManagerComponent::HandleWeaponAttackStarted(EWeaponAttackInput AttackType, int32 ComboStage)
{
    UE_LOG(LogTemp, Verbose, TEXT("[%s] 武器攻击开始 - 类型: %s, 阶段: %d"), 
        *GetName(), *UEnum::GetValueAsString(AttackType), ComboStage);
}

void UWeaponManagerComponent::HandleWeaponAttackHit(const FDamageResult& DamageResult)
{
    if (DamageResult.bHit)
    {
        UE_LOG(LogTemp, Log, TEXT("[%s] 武器攻击命中 - 伤害: %.0f%s"), 
            *GetName(), DamageResult.FinalDamage, 
            DamageResult.bCritical ? TEXT(" (暴击)") : TEXT(""));
    }
}

void UWeaponManagerComponent::HandleWeaponAttackEnded()
{
    UE_LOG(LogTemp, Verbose, TEXT("[%s] 武器攻击结束"), *GetName());
    
    // 推进连招阶段
    AdvanceComboStage();
}

void UWeaponManagerComponent::HandleWeaponStateChanged(EWeaponState NewState, EWeaponState OldState)
{
    UE_LOG(LogTemp, Verbose, TEXT("[%s] 武器状态改变 - %s -> %s"), 
        *GetName(), *UEnum::GetValueAsString(OldState), *UEnum::GetValueAsString(NewState));
}
