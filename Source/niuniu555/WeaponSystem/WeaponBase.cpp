// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "CombatSystem/DamageCalculator.h"
#include "CombatSystem/DamageableInterface.h"
#include "WeaponDataAsset.h"

AWeaponBase::AWeaponBase()
{
    // 设置此Actor不是每帧Tick
    PrimaryActorTick.bCanEverTick = true;
    
    // 启用复制（多人游戏需要）
    bReplicates = true;
    SetReplicatingMovement(true);

    // 创建根组件
    WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("武器根组件"));
    RootComponent = WeaponRoot;

    // 创建武器网格体组件
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("武器网格体"));
    WeaponMesh->SetupAttachment(RootComponent);
    
    // 默认关闭碰撞
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));

    // 初始化默认武器名称
    WeaponName = FText::FromString(TEXT("基础武器"));
    WeaponDescription = FText::FromString(TEXT("一把普通的武器"));
}

void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();

    // 初始化状态
    CurrentState = EWeaponState::Idle;
    CurrentComboStage = 0;
    CurrentChargeRatio = 0.0f;
    ChargeTimer = 0.0f;
    bIsCharging = false;
    
    // 清空命中列表
    HitActorsThisAttack.Empty();

    // 从数据资产加载配置
    LoadConfigFromDataAsset();
}

void AWeaponBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 更新蓄力状态
    if (bIsCharging)
    {
        UpdateCharging(DeltaTime);
    }

    // 更新冷却计时器
    if (CooldownTimer > 0.0f)
    {
        CooldownTimer -= DeltaTime;
        if (CooldownTimer <= 0.0f && CurrentState == EWeaponState::Cooldown)
        {
            SetWeaponState(EWeaponState::Idle);
        }
    }
}

void AWeaponBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 清理动画委托
    if (OwnerAnimInstance != nullptr && OnMontageEndedDelegate.IsBound())
    {
        FOnMontageEnded EmptyDelegate;
        OwnerAnimInstance->Montage_SetEndDelegate(EmptyDelegate, nullptr);
    }

    Super::EndPlay(EndPlayReason);
}

// ========== 状态查询 ==========

bool AWeaponBase::CanAttack_Implementation() const
{
    // 只有在以下状态才能攻击：空闲、攻击中（取消窗口期）、连招中
    switch (CurrentState)
    {
    case EWeaponState::Idle:
        return true;
    case EWeaponState::Attacking:
        // 检查是否在Cancel Window内
        return CanCancelAttack();
    case EWeaponState::Cooldown:
        // 冷却完成
        return CooldownTimer <= 0.0f;
    default:
        return false;
    }
}

// ========== 攻击操作 ==========

bool AWeaponBase::PerformLightAttack_Implementation(int32 ComboStage)
{
    // 检查是否可以攻击
    if (!IWeaponInterface::Execute_CanAttack(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 无法执行轻击 - 当前状态: %s"), 
            *GetName(), *UEnum::GetValueAsString(CurrentState));
        return false;
    }

    // 检查动画蒙太奇是否设置
    if (LightAttackMontage == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 轻击动画蒙太奇未设置"), *GetName());
        return false;
    }

    // 更新连招阶段
    CurrentComboStage = FMath::Clamp(ComboStage, 0, IWeaponInterface::Execute_GetMaxComboStages(this) - 1);

    // 获取当前阶段对应的Section名称
    FName SectionName = NAME_None;
    if (LightComboStages.IsValidIndex(CurrentComboStage))
    {
        SectionName = LightComboStages[CurrentComboStage].StageName;
    }

    // 播放攻击动画
    if (!PlayAttackMontage(LightAttackMontage, SectionName))
    {
        return false;
    }

    // 设置武器状态
    SetWeaponState(EWeaponState::Attacking);

    // 清空命中列表
    HitActorsThisAttack.Empty();
    AttackStartTime = GetWorld()->GetTimeSeconds();

    // 广播攻击开始事件
    OnAttackStarted.Broadcast(EWeaponAttackInput::LightAttack, CurrentComboStage);

    UE_LOG(LogTemp, Log, TEXT("[%s] 开始轻击 - 阶段: %d, Section: %s"), 
        *GetName(), CurrentComboStage, *SectionName.ToString());

    // 延迟执行攻击检测（等武器挥出去）
    FTimerHandle AttackTraceTimer;
    GetWorldTimerManager().SetTimer(
        AttackTraceTimer,
        this,
        &AWeaponBase::PerformAttackTrace,
        0.15f,  // 延迟0.15秒，可根据动画调整
        false   // 只执行一次
    );

    return true;
}

bool AWeaponBase::PerformHeavyAttack_Implementation(int32 ComboStage)
{
    // 检查是否可以攻击
    if (!IWeaponInterface::Execute_CanAttack(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 无法执行重击 - 当前状态: %s"), 
            *GetName(), *UEnum::GetValueAsString(CurrentState));
        return false;
    }

    // 检查动画蒙太奇是否设置
    if (HeavyAttackMontage == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 重击动画蒙太奇未设置"), *GetName());
        return false;
    }

    // 更新连招阶段
    CurrentComboStage = FMath::Clamp(ComboStage, 0, IWeaponInterface::Execute_GetMaxComboStages(this) - 1);

    // 获取当前阶段对应的Section名称
    FName SectionName = NAME_None;
    if (HeavyComboStages.IsValidIndex(CurrentComboStage))
    {
        SectionName = HeavyComboStages[CurrentComboStage].StageName;
    }

    // 播放攻击动画
    if (!PlayAttackMontage(HeavyAttackMontage, SectionName))
    {
        return false;
    }

    // 设置武器状态
    SetWeaponState(EWeaponState::Attacking);

    // 清空命中列表
    HitActorsThisAttack.Empty();
    AttackStartTime = GetWorld()->GetTimeSeconds();

    // 广播攻击开始事件
    OnAttackStarted.Broadcast(EWeaponAttackInput::HeavyAttack, CurrentComboStage);

    UE_LOG(LogTemp, Log, TEXT("[%s] 开始重击 - 阶段: %d, Section: %s"), 
        *GetName(), CurrentComboStage, *SectionName.ToString());

    // 延迟执行攻击检测（重击延迟稍长）
    FTimerHandle AttackTraceTimer;
    GetWorldTimerManager().SetTimer(
        AttackTraceTimer,
        this,
        &AWeaponBase::PerformAttackTrace,
        0.2f,  // 重击延迟0.2秒
        false
    );

    return true;
}

bool AWeaponBase::StartChargedAttack_Implementation()
{
    // 检查是否可以攻击
    if (!IWeaponInterface::Execute_CanAttack(this))
    {
        return false;
    }

    // 检查蓄力动画
    if (ChargedAttackMontage == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 蓄力攻击动画蒙太奇未设置"), *GetName());
        return false;
    }

    // 进入蓄力状态
    EnterChargingState();
    return true;
}

bool AWeaponBase::ReleaseChargedAttack_Implementation(float ChargeRatio)
{
    if (!bIsCharging)
    {
        return false;
    }

    // 退出蓄力状态
    ExitChargingState();

    // 记录蓄力比例
    CurrentChargeRatio = FMath::Clamp(ChargeRatio, 0.0f, 1.0f);

    // 播放蓄力攻击动画（从蓄力循环切换到攻击）
    if (ChargedAttackMontage != nullptr && WeaponOwner != nullptr)
    {
        UAnimInstance* AnimInst = WeaponOwner->GetMesh()->GetAnimInstance();
        if (AnimInst != nullptr)
        {
            // 跳转到攻击Section
            AnimInst->Montage_JumpToSection(ChargeAttackSectionName, ChargedAttackMontage);
        }
    }

    // 设置武器状态为攻击中
    SetWeaponState(EWeaponState::Attacking);

    // 清空命中列表
    HitActorsThisAttack.Empty();
    AttackStartTime = GetWorld()->GetTimeSeconds();

    UE_LOG(LogTemp, Log, TEXT("[%s] 释放蓄力攻击 - 蓄力比例: %.2f"), 
        *GetName(), CurrentChargeRatio);

    return true;
}

void AWeaponBase::CancelAttack_Implementation()
{
    if (CurrentState != EWeaponState::Attacking && CurrentState != EWeaponState::Charging)
    {
        return;
    }

    // 停止动画
    StopAttackMontage();

    // 如果在蓄力，退出蓄力状态
    if (bIsCharging)
    {
        ExitChargingState();
    }

    // 重置状态
    SetWeaponState(EWeaponState::Idle);
    ResetComboState();

    UE_LOG(LogTemp, Log, TEXT("[%s] 取消攻击"), *GetName());
}

// ========== 数据加载 ==========

void AWeaponBase::LoadConfigFromDataAsset()
{
    if (WeaponDataAsset == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 没有配置WeaponDataAsset，使用直接设置的属性"), *GetName());
        return;
    }

    // 加载基础信息
    WeaponType = WeaponDataAsset->WeaponType;
    WeaponName = WeaponDataAsset->WeaponName;
    WeaponDescription = WeaponDataAsset->WeaponDescription;
    WeaponRarity = WeaponDataAsset->Rarity;

    // 加载基础属性
    BaseStats = WeaponDataAsset->ToWeaponBaseStats();

    // 加载轻击连招配置
    LightAttackMontage = WeaponDataAsset->LightAttackMontage;
    LightComboStages.Empty();
    for (int32 i = 0; i < WeaponDataAsset->LightAttackSections.Num(); ++i)
    {
        FComboStageInfo StageInfo;
        StageInfo.StageName = WeaponDataAsset->LightAttackSections[i];
        StageInfo.bCanCancel = WeaponDataAsset->IsLightStageCancellable(i);
        StageInfo.CancelWindowStart = WeaponDataAsset->LightCancelWindowStart;
        StageInfo.CancelWindowEnd = WeaponDataAsset->LightCancelWindowEnd;
        
        if (WeaponDataAsset->LightAttackDamageMultipliers.IsValidIndex(i))
        {
            StageInfo.DamageMultiplier = WeaponDataAsset->LightAttackDamageMultipliers[i];
        }
        else
        {
            StageInfo.DamageMultiplier = 1.0f + (i * 0.2f);
        }
        
        LightComboStages.Add(StageInfo);
    }

    // 加载重击连招配置
    HeavyAttackMontage = WeaponDataAsset->HeavyAttackMontage;
    HeavyComboStages.Empty();
    for (int32 i = 0; i < WeaponDataAsset->HeavyAttackSections.Num(); ++i)
    {
        FComboStageInfo StageInfo;
        StageInfo.StageName = WeaponDataAsset->HeavyAttackSections[i];
        StageInfo.bCanCancel = WeaponDataAsset->IsHeavyStageCancellable(i);
        StageInfo.CancelWindowStart = WeaponDataAsset->HeavyCancelWindowStart;
        StageInfo.CancelWindowEnd = WeaponDataAsset->HeavyCancelWindowEnd;
        
        if (WeaponDataAsset->HeavyAttackDamageMultipliers.IsValidIndex(i))
        {
            StageInfo.DamageMultiplier = WeaponDataAsset->HeavyAttackDamageMultipliers[i];
        }
        else
        {
            StageInfo.DamageMultiplier = 1.5f + (i * 0.3f);
        }
        
        HeavyComboStages.Add(StageInfo);
    }

    // 加载蓄力攻击配置
    ChargedAttackMontage = WeaponDataAsset->ChargedAttackMontage;
    ChargeLoopSectionName = WeaponDataAsset->ChargeLoopSection;
    ChargeAttackSectionName = WeaponDataAsset->ChargeAttackSection;
    MaxChargeTime = WeaponDataAsset->MaxChargeTime;

    // 加载特效和音效
    HitEffect = WeaponDataAsset->NormalHitEffect;
    TrailEffect = WeaponDataAsset->TrailEffect;
    AttackSound = WeaponDataAsset->AttackSound;
    HitSound = WeaponDataAsset->HitSound;

    // 加载碰撞配置
    AttackTraceChannel = WeaponDataAsset->AttackTraceChannel;
    AttackStartBoneName = WeaponDataAsset->AttackStartSocket;
    AttackEndBoneName = WeaponDataAsset->AttackEndSocket;
    AttackTraceRadius = WeaponDataAsset->AttackTraceRadius;

    // 加载武器模型
    if (WeaponDataAsset->WeaponMesh != nullptr)
    {
        WeaponMesh->SetSkeletalMesh(WeaponDataAsset->WeaponMesh);
    }

    UE_LOG(LogTemp, Log, TEXT("[%s] 从数据资产加载配置: %s"), 
        *GetName(), *WeaponDataAsset->WeaponID.ToString());
}

// ========== 连招相关 ==========

void AWeaponBase::ResetCombo_Implementation()
{
    ResetComboState();
}

int32 AWeaponBase::GetMaxComboStages_Implementation() const
{
    // 返回轻击和重击连招阶段数的最大值
    return FMath::Max(LightComboStages.Num(), HeavyComboStages.Num());
}

FComboStageInfo AWeaponBase::GetCurrentComboStageInfo() const
{
    // 根据当前状态返回对应的连招阶段信息
    if (CurrentState == EWeaponState::Attacking)
    {
        // 这里需要知道是轻击还是重击，简化处理返回轻击的
        if (LightComboStages.IsValidIndex(CurrentComboStage))
        {
            return LightComboStages[CurrentComboStage];
        }
    }
    return FComboStageInfo();
}

void AWeaponBase::ResetComboState()
{
    CurrentComboStage = 0;
    CurrentChargeRatio = 0.0f;
    HitActorsThisAttack.Empty();
}

void AWeaponBase::AdvanceComboStage()
{
    CurrentComboStage++;
    int32 MaxStage = IWeaponInterface::Execute_GetMaxComboStages(this);
    if (CurrentComboStage >= MaxStage)
    {
        CurrentComboStage = 0;
    }
}

// ========== 装备/卸下 ==========

void AWeaponBase::OnEquipped_Implementation(ACharacter* OwnerCharacter)
{
    if (OwnerCharacter == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 装备失败 - OwnerCharacter为空"), *GetName());
        return;
    }

    // 设置拥有者
    WeaponOwner = OwnerCharacter;

    // 附加到角色的武器插槽（通常是hand_r或WeaponSocket）
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
    AttachToComponent(OwnerCharacter->GetMesh(), AttachRules, FName("hand_r"));

    // 获取动画实例
    if (OwnerCharacter->GetMesh() != nullptr)
    {
        OwnerAnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
    }

    // 设置武器可见
    SetActorHiddenInGame(false);

    // 重置状态
    SetWeaponState(EWeaponState::Idle);
    ResetComboState();

    UE_LOG(LogTemp, Log, TEXT("[%s] 已装备到 %s"), *GetName(), *OwnerCharacter->GetName());
}

void AWeaponBase::OnUnequipped_Implementation()
{
    // 停止当前动画
    StopAttackMontage();

    // 取消附着
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // 隐藏武器
    SetActorHiddenInGame(true);

    // 清除拥有者
    WeaponOwner = nullptr;
    OwnerAnimInstance = nullptr;

    // 重置状态
    SetWeaponState(EWeaponState::Idle);
    ResetComboState();

    UE_LOG(LogTemp, Log, TEXT("[%s] 已卸下"), *GetName());
}

// ========== 伤害计算 ==========

FDamageResult AWeaponBase::CalculateDamage_Implementation(float BaseDamage, EDamageType InDamageType)
{
    FDamageResult Result;
    Result.DamageType = InDamageType;

    // 计算基础伤害
    float FinalDamage = BaseDamage * BaseStats.BaseAttack;

    // 暴击判定
    bool bIsCritical = FMath::RandRange(0.0f, 1.0f) < BaseStats.CriticalRate;
    if (bIsCritical)
    {
        FinalDamage *= BaseStats.CriticalDamage;
    }

    // 随机浮动 (0.9 - 1.1)
    float RandomFactor = FMath::RandRange(0.9f, 1.1f);
    FinalDamage *= RandomFactor;

    // 蓄力加成
    if (CurrentChargeRatio > 0.0f)
    {
        FinalDamage *= (1.0f + CurrentChargeRatio); // 蓄力满时伤害翻倍
    }

    Result.FinalDamage = FMath::FloorToInt(FinalDamage);
    Result.bCritical = bIsCritical;
    Result.bHit = true;

    return Result;
}

// ========== 公共方法 ==========

void AWeaponBase::SetWeaponState(EWeaponState NewState)
{
    if (CurrentState != NewState)
    {
        EWeaponState OldState = CurrentState;
        CurrentState = NewState;
        OnStateChanged.Broadcast(NewState, OldState);

        UE_LOG(LogTemp, Verbose, TEXT("[%s] 武器状态: %s -> %s"), 
            *GetName(), *UEnum::GetValueAsString(OldState), *UEnum::GetValueAsString(NewState));
    }
}

bool AWeaponBase::CanCancelAttack() const
{
    if (CurrentState != EWeaponState::Attacking)
    {
        return false;
    }

    // 获取当前阶段信息
    FComboStageInfo StageInfo = GetCurrentComboStageInfo();
    if (!StageInfo.bCanCancel)
    {
        return false;
    }

    // 检查是否在Cancel Window内
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ElapsedTime = CurrentTime - AttackStartTime;

    // 根据攻击速度调整窗口时间
    float AdjustedStart = StageInfo.CancelWindowStart / BaseStats.AttackSpeed;
    float AdjustedEnd = StageInfo.CancelWindowEnd / BaseStats.AttackSpeed;

    return ElapsedTime >= AdjustedStart && ElapsedTime <= AdjustedEnd;
}

void AWeaponBase::PerformAttackTrace()
{
    UE_LOG(LogTemp, Log, TEXT("[WeaponBase] PerformAttackTrace 被调用"));
    
    if (WeaponOwner == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[WeaponBase] WeaponOwner 为空，攻击检测取消"));
        return;
    }

    // 获取攻击检测的起始和结束位置
    FVector StartLocation = GetActorLocation();
    FVector EndLocation = StartLocation + WeaponOwner->GetActorForwardVector() * BaseStats.AttackRange;
    
    UE_LOG(LogTemp, Log, TEXT("[WeaponBase] 攻击检测: 起始=%s, 结束=%s, 范围=%.0f, 通道=%d"), 
        *StartLocation.ToString(), *EndLocation.ToString(), BaseStats.AttackRange, (int32)AttackTraceChannel);

    // 如果有骨骼名称配置，使用骨骼位置
    if (WeaponMesh != nullptr && !AttackStartBoneName.IsNone())
    {
        StartLocation = WeaponMesh->GetSocketLocation(AttackStartBoneName);
    }
    if (WeaponMesh != nullptr && !AttackEndBoneName.IsNone())
    {
        EndLocation = WeaponMesh->GetSocketLocation(AttackEndBoneName);
    }
    else
    {
        // 使用默认的攻击方向
        EndLocation = StartLocation + WeaponOwner->GetActorForwardVector() * BaseStats.AttackRange;
    }

    // 胶囊体检测参数
    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(WeaponOwner);

    // 执行胶囊体检测
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        StartLocation,
        EndLocation,
        FQuat::Identity,
        AttackTraceChannel,
        FCollisionShape::MakeSphere(AttackTraceRadius),
        QueryParams
    );

    UE_LOG(LogTemp, Log, TEXT("[WeaponBase] 攻击检测结果: %s, 命中数量=%d"), 
        bHit ? TEXT("命中") : TEXT("未命中"), HitResults.Num());

    // 调试用：绘制检测范围
    #if ENABLE_DRAW_DEBUG
    DrawDebugCapsule(GetWorld(), (StartLocation + EndLocation) * 0.5f, 
        (EndLocation - StartLocation).Size() * 0.5f, AttackTraceRadius, 
        FRotationMatrix::MakeFromZ(EndLocation - StartLocation).ToQuat(), 
        bHit ? FColor::Green : FColor::Red, false, 1.0f);
    #endif

    // 处理命中结果
    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            if (HitActor == nullptr)
            {
                continue;
            }

            UE_LOG(LogTemp, Log, TEXT("[WeaponBase] 处理命中目标: %s"), *HitActor->GetName());
            
            // 检查是否已经命中过（防止重复伤害）
            if (HitActorsThisAttack.Contains(HitActor))
            {
                UE_LOG(LogTemp, Verbose, TEXT("[WeaponBase] 目标 %s 已命中过，跳过"), *HitActor->GetName());
                continue;
            }

            // 添加到已命中列表
            HitActorsThisAttack.Add(HitActor);
            
            // 检查是否实现了 DamageableInterface
            bool bImplementsInterface = HitActor->GetClass()->ImplementsInterface(UDamageableInterface::StaticClass());
            UE_LOG(LogTemp, Log, TEXT("[WeaponBase] 目标 %s 实现 DamageableInterface: %s"), 
                *HitActor->GetName(), bImplementsInterface ? TEXT("是") : TEXT("否"));

            // ====== 新伤害计算系统 ======
            // 1. 准备伤害计算参数
            FDamageCalculationParams DamageParams;
            DamageParams.BaseAttack = BaseStats.BaseAttack;
            
            // 获取当前阶段伤害倍率
            FComboStageInfo CurrentStageInfo = GetCurrentComboStageInfo();
            DamageParams.SkillMultiplier = CurrentStageInfo.DamageMultiplier;
            
            DamageParams.DamageType = EDamageType::Physical;
            DamageParams.CriticalRate = BaseStats.CriticalRate;
            DamageParams.CriticalDamage = BaseStats.CriticalDamage;
            
            // 2. 从目标获取防御和抗性（如果目标实现了接口）- 使用 Execute_ 方式
            if (bImplementsInterface)
            {
                DamageParams.TargetDefense = IDamageableInterface::Execute_GetDefense(HitActor);
                DamageParams.PhysicalResistance = IDamageableInterface::Execute_GetResistance(HitActor, EDamageType::Physical);
                UE_LOG(LogTemp, Log, TEXT("[WeaponBase] 目标防御=%.0f, 抗性=%.2f"), 
                    DamageParams.TargetDefense, DamageParams.PhysicalResistance);
            }
            
            // 3. 使用伤害计算器计算伤害
            FDamageCalculationResult CalcResult = UDamageCalculator::CalculateDamage(DamageParams);
            
            // 4. 创建受击信息
            FHitInfo HitInfo;
            HitInfo.Damage = CalcResult.FinalDamage;
            HitInfo.bCritical = CalcResult.bCritical;
            HitInfo.DamageType = EDamageType::Physical;
            HitInfo.Attacker = WeaponOwner;
            HitInfo.HitLocation = Hit.ImpactPoint;
            
            // 安全计算受击方向
            if (WeaponOwner != nullptr)
            {
                HitInfo.HitDirection = (HitActor->GetActorLocation() - WeaponOwner->GetActorLocation()).GetSafeNormal();
            }
            else
            {
                HitInfo.HitDirection = FVector::ForwardVector;
            }
            
            HitInfo.KnockbackForce = CurrentStageInfo.KnockbackForce;
            HitInfo.HitReaction = EHitReactionType::Light;
            HitInfo.bTriggerHitStop = CalcResult.bCritical;
            HitInfo.HitStopDuration = 0.05f;
            
            // 5. 如果目标实现了DamageableInterface，使用新系统（使用 ImplementsInterface 判断，不是 Cast）
            if (bImplementsInterface)
            {
                UE_LOG(LogTemp, Log, TEXT("[WeaponBase] 准备调用 TakeDamage，目标=%s, 伤害=%.0f"), 
                    *HitActor->GetName(), HitInfo.Damage);
                
                IDamageableInterface::Execute_TakeDamage(HitActor, HitInfo);
                
                UE_LOG(LogTemp, Log, TEXT("[WeaponBase] TakeDamage 调用完成，目标=%s"), *HitActor->GetName());
            }
            else
            {
                // 6. 兼容旧系统（普通Actor）
                FDamageResult LegacyDamageResult;
                LegacyDamageResult.FinalDamage = CalcResult.FinalDamage;
                LegacyDamageResult.bCritical = CalcResult.bCritical;
                LegacyDamageResult.bHit = true;
                LegacyDamageResult.DamageType = EDamageType::Physical;
                LegacyDamageResult.HitLocation = Hit.ImpactPoint;
                LegacyDamageResult.HitNormal = Hit.ImpactNormal;
                LegacyDamageResult.HitActor = HitActor;
                
                ApplyDamageToActor(HitActor, LegacyDamageResult);
                
                UE_LOG(LogTemp, Log, TEXT("[%s] 攻击命中 %s [兼容模式], 伤害: %.0f%s"), 
                    *GetName(), *HitActor->GetName(), CalcResult.FinalDamage,
                    CalcResult.bCritical ? TEXT(" (暴击)") : TEXT(""));
            }
            
            // 7. 生成特效和音效
            SpawnHitEffect(Hit.ImpactPoint, Hit.ImpactNormal);
            PlayHitSound(Hit.ImpactPoint);
            
            // 8. 广播命中事件（兼容旧系统）
            FDamageResult BroadcastResult;
            BroadcastResult.FinalDamage = CalcResult.FinalDamage;
            BroadcastResult.bCritical = CalcResult.bCritical;
            BroadcastResult.bHit = true;
            BroadcastResult.DamageType = EDamageType::Physical;
            BroadcastResult.HitLocation = Hit.ImpactPoint;
            BroadcastResult.HitNormal = Hit.ImpactNormal;
            BroadcastResult.HitActor = HitActor;
            OnAttackHit.Broadcast(BroadcastResult);
        }
    }
}

void AWeaponBase::NotifyAttackEnded()
{
    // 连招阶段推进
    if (CurrentState == EWeaponState::Attacking)
    {
        AdvanceComboStage();
    }

    // 设置冷却
    SetWeaponState(EWeaponState::Cooldown);
    CooldownTimer = BaseStats.CooldownTime;

    // 广播攻击结束事件
    OnAttackEnded.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("[%s] 攻击结束，进入冷却"), *GetName());
}

// ========== 保护方法 ==========

bool AWeaponBase::PlayAttackMontage(UAnimMontage* Montage, FName SectionName)
{
    if (WeaponOwner == nullptr || Montage == nullptr)
    {
        return false;
    }

    UAnimInstance* AnimInst = WeaponOwner->GetMesh()->GetAnimInstance();
    if (AnimInst == nullptr)
    {
        return false;
    }

    // 计算播放速率（根据攻击速度）
    float PlayRate = BaseStats.AttackSpeed;

    // 播放动画蒙太奇
    float Duration = AnimInst->Montage_Play(Montage, PlayRate);
    if (Duration > 0.0f)
    {
        // 如果有指定Section，跳转到该Section
        if (!SectionName.IsNone())
        {
            AnimInst->Montage_JumpToSection(SectionName, Montage);
        }

        // 绑定动画结束委托
        OnMontageEndedDelegate.BindUObject(this, &AWeaponBase::OnAttackMontageEnded);
        AnimInst->Montage_SetEndDelegate(OnMontageEndedDelegate, Montage);

        OwnerAnimInstance = AnimInst;
        return true;
    }

    return false;
}

void AWeaponBase::StopAttackMontage()
{
    if (WeaponOwner == nullptr)
    {
        return;
    }

    UAnimInstance* AnimInst = WeaponOwner->GetMesh()->GetAnimInstance();
    if (AnimInst == nullptr)
    {
        return;
    }

    // 停止所有攻击相关的动画蒙太奇
    if (LightAttackMontage != nullptr)
    {
        AnimInst->Montage_Stop(0.1f, LightAttackMontage);
    }
    if (HeavyAttackMontage != nullptr)
    {
        AnimInst->Montage_Stop(0.1f, HeavyAttackMontage);
    }
    if (ChargedAttackMontage != nullptr)
    {
        AnimInst->Montage_Stop(0.1f, ChargedAttackMontage);
    }
}

void AWeaponBase::ApplyDamageToActor(AActor* TargetActor, const FDamageResult& DamageInfo)
{
    if (TargetActor == nullptr || WeaponOwner == nullptr)
    {
        return;
    }

    // 应用UGameplayStatics伤害
    UGameplayStatics::ApplyDamage(
        TargetActor,
        DamageInfo.FinalDamage,
        WeaponOwner->GetController(),
        this,
        UDamageType::StaticClass()
    );
}

void AWeaponBase::SpawnHitEffect(const FVector& Location, const FVector& Normal)
{
    FRotator Rotation = Normal.Rotation();
    
    if (HitEffect != nullptr)
    {
        UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            HitEffect,
            Location,
            Rotation
        );
        if (ParticleComp)
        {
            ParticleComp->bAutoDestroy = true;
        }
    }
}

void AWeaponBase::PlayHitSound(const FVector& Location)
{
    if (HitSound == nullptr)
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        HitSound,
        Location
    );
}

void AWeaponBase::EnterChargingState()
{
    bIsCharging = true;
    ChargeTimer = 0.0f;
    CurrentChargeRatio = 0.0f;

    // 播放蓄力动画（循环部分）
    if (ChargedAttackMontage != nullptr && WeaponOwner != nullptr)
    {
        UAnimInstance* AnimInst = WeaponOwner->GetMesh()->GetAnimInstance();
        if (AnimInst != nullptr)
        {
            AnimInst->Montage_Play(ChargedAttackMontage);
            AnimInst->Montage_JumpToSection(ChargeLoopSectionName, ChargedAttackMontage);
        }
    }

    SetWeaponState(EWeaponState::Charging);

    UE_LOG(LogTemp, Log, TEXT("[%s] 开始蓄力"), *GetName());
}

void AWeaponBase::UpdateCharging(float DeltaTime)
{
    if (!bIsCharging)
    {
        return;
    }

    // 更新蓄力计时
    ChargeTimer += DeltaTime;
    CurrentChargeRatio = FMath::Clamp(ChargeTimer / MaxChargeTime, 0.0f, 1.0f);

    // 可以在这里添加蓄力特效更新
}

void AWeaponBase::ExitChargingState()
{
    bIsCharging = false;
    ChargeTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("[%s] 结束蓄力 - 最终蓄力比例: %.2f"), 
        *GetName(), CurrentChargeRatio);
}

void AWeaponBase::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // 动画结束，通知攻击结束
    NotifyAttackEnded();
}
