// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyBase.h"
#include "EnemyDataAsset.h"
#include "EnemyStateMachineComponent.h"
#include "AttributeSystem/AttributeComponent.h"
#include "CombatSystem/HitReactionComponent.h"
#include "CombatSystem/DamageCalculator.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// 定义EnemyBase日志类别
DEFINE_LOG_CATEGORY(LogEnemy);

AEnemyBase::AEnemyBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;

    // 创建属性组件
    AttributeComp = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));

    // 创建受击反馈组件
    HitReactionComp = CreateDefaultSubobject<UHitReactionComponent>(TEXT("HitReactionComponent"));

    // 创建状态机组件
    StateMachineComp = CreateDefaultSubobject<UEnemyStateMachineComponent>(TEXT("StateMachineComponent"));

    // 默认值
    CurrentChapter = 1;
    bInitialized = false;
    bIsDying = false;
    bShowDebugInfo = false;

    // 攻击冷却初始化
    AttackCooldownTimer = 0.0f;
    bIsInAttackCooldown = false;

    // 设置自动 possess AI
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // 启用受击反应组件的默认配置
    if (HitReactionComp)
    {
        // 受击反馈组件配置
        HitReactionComp->FlashDuration = 0.15f;
        HitReactionComp->HitStopDuration = 0.05f;
    }
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogEnemy, Log, TEXT("[EnemyBase] %s BeginPlay 开始初始化"), *GetName());

    // 记录出生位置
    SpawnLocation = GetActorLocation();

    // 检查关键组件
    if (!AttributeComp)
    {
        UE_LOG(LogEnemy, Error, TEXT("[EnemyBase] %s AttributeComp is NULL! 敌人无法正常工作"), *GetName());
        return;
    }

    if (!StateMachineComp)
    {
        UE_LOG(LogEnemy, Error, TEXT("[EnemyBase] %s StateMachineComp is NULL!"), *GetName());
    }

    // 如果有数据资产，自动初始化
    if (EnemyData && !bInitialized)
    {
        InitializeFromDataAsset(EnemyData, CurrentChapter);
    }
    else if (!EnemyData)
    {
        UE_LOG(LogEnemy, Warning, TEXT("[EnemyBase] %s EnemyData未设置，使用默认血量100"), *GetName());
        // 使用默认属性
        AttributeComp->SetBaseValue(EAttributeType::MaxHealth, 100.0f);
        AttributeComp->SetBaseValue(EAttributeType::Health, 100.0f);
        bInitialized = true;
    }

    // 绑定状态机事件
    if (StateMachineComp)
    {
        StateMachineComp->OnStateChanged.AddDynamic(this, &AEnemyBase::OnStateChanged);
    }

    // 绑定属性变化事件（关键：血量变化时触发回调）
    if (AttributeComp)
    {
        AttributeComp->OnHealthChanged.AddDynamic(this, &AEnemyBase::OnHealthChanged);
        UE_LOG(LogEnemy, Log, TEXT("[EnemyBase] %s 已绑定血量变化事件"), *GetName());
    }

    // ===== 关键修复：确保血量不为0（防止IsAlive()返回false导致无法受伤）=====
    if (AttributeComp)
    {
        float CurrentHealth = AttributeComp->GetHealth();
        float MaxHealth = AttributeComp->GetMaxHealth();
        
        UE_LOG(LogEnemy, Log, TEXT("[EnemyBase] %s 血量检查 - 当前:%.0f 最大:%.0f"), 
            *GetName(), CurrentHealth, MaxHealth);
        
        // 如果当前血量为0或未初始化
        if (CurrentHealth <= 0.0f)
        {
            // 如果最大生命值也为0，先设置默认值
            if (MaxHealth <= 0.0f)
            {
                MaxHealth = 100.0f;
                AttributeComp->SetBaseValue(EAttributeType::MaxHealth, MaxHealth);
                UE_LOG(LogEnemy, Warning, TEXT("[EnemyBase] %s MaxHealth为0，强制设为100"), *GetName());
            }
            
            // 设置当前血量 = 最大生命值
            AttributeComp->SetBaseValue(EAttributeType::Health, MaxHealth);
            UE_LOG(LogEnemy, Warning, TEXT("[EnemyBase] %s CurrentHealth为0，强制初始化为:%.0f"), 
                *GetName(), MaxHealth);
        }
    }
    // ==================================================================

    // 输出最终初始化状态
    UE_LOG(LogEnemy, Log, TEXT("[EnemyBase] %s 初始化完成 - 最终血量:%.0f/%.0f"), 
        *GetName(), GetCurrentHealth(), GetMaxHealth());
}

void AEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 更新攻击冷却
    if (bIsInAttackCooldown)
    {
        AttackCooldownTimer -= DeltaTime;
        if (AttackCooldownTimer <= 0.0f)
        {
            AttackCooldownTimer = 0.0f;
            bIsInAttackCooldown = false;
            UE_LOG(LogEnemy, Log, TEXT("[EnemyBase] %s 攻击冷却结束"), *GetName());
        }
    }

    // 调试信息显示
    if (bShowDebugInfo && bInitialized)
    {
        FString CooldownInfo = bIsInAttackCooldown ? FString::Printf(TEXT(" | CD:%.1f"), AttackCooldownTimer) : TEXT("");
        FString DebugInfo = FString::Printf(TEXT("[%s] HP:%.0f/%.0f | State:%s%s"),
            *GetName(),
            GetCurrentHealth(),
            GetMaxHealth(),
            *UEnum::GetValueAsString(GetCurrentState()),
            *CooldownInfo);
        
        DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 100), DebugInfo, nullptr, FColor::White, 0.0f);
    }
}

void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void AEnemyBase::InitializeFromDataAsset(UEnemyDataAsset* DataAsset, int32 Chapter)
{
    if (!DataAsset)
    {
        UE_LOG(LogEnemy, Warning, TEXT("[EnemyBase] 初始化失败：数据资产为空"));
        return;
    }

    EnemyData = DataAsset;
    CurrentChapter = FMath::Max(1, Chapter);

    // 更新状态机数据
    if (StateMachineComp)
    {
        StateMachineComp->EnemyData = EnemyData;
    }

    // 更新属性数值
    UpdateAttributeStats();

    // 更新移动速度
    if (GetCharacterMovement() && EnemyData)
    {
        GetCharacterMovement()->MaxWalkSpeed = EnemyData->BaseMoveSpeed;
        GetCharacterMovement()->RotationRate = FRotator(0, EnemyData->BaseRotationSpeed, 0);
    }

    // 设置网格体和动画
    if (GetMesh() && EnemyData)
    {
        if (EnemyData->SkeletalMesh)
        {
            GetMesh()->SetSkeletalMesh(EnemyData->SkeletalMesh);
        }

        if (EnemyData->AnimBlueprintClass)
        {
            GetMesh()->SetAnimInstanceClass(EnemyData->AnimBlueprintClass);
        }

        // 根据尺寸调整缩放
        float SizeScale = EnemyData->GetSizeScale();
        GetMesh()->SetWorldScale3D(FVector(SizeScale));
    }

    bInitialized = true;

    // 调用蓝图事件
    BP_OnInitialized();

    UE_LOG(LogEnemy, Log, TEXT("[EnemyBase] %s 初始化完成 - 章节:%d"), *GetName(), CurrentChapter);
}

void AEnemyBase::SetChapter(int32 NewChapter)
{
    if (CurrentChapter != NewChapter)
    {
        CurrentChapter = FMath::Max(1, NewChapter);
        UpdateAttributeStats();
    }
}

void AEnemyBase::UpdateAttributeStats()
{
    if (!EnemyData || !AttributeComp)
    {
        return;
    }

    // 从数据资产获取章节调整后的属性
    float Health = EnemyData->GetHealthForChapter(CurrentChapter);
    float Attack = EnemyData->GetAttackForChapter(CurrentChapter);
    float Defense = EnemyData->GetDefenseForChapter(CurrentChapter);

    // 设置最大生命值（基础值）
    AttributeComp->SetBaseValue(EAttributeType::MaxHealth, Health);
    // 设置当前生命值 = 最大生命值（满血初始化）
    AttributeComp->SetBaseValue(EAttributeType::Health, Health);
    
    // 设置攻击力
    AttributeComp->SetBaseValue(EAttributeType::Attack, Attack);
    // 设置防御力
    AttributeComp->SetBaseValue(EAttributeType::Defense, Defense);
    // 设置移动速度
    AttributeComp->SetBaseValue(EAttributeType::MoveSpeed, EnemyData->BaseMoveSpeed);

    UE_LOG(LogEnemy, Log, TEXT("[EnemyBase] %s 属性更新 - HP:%.0f/%.0f ATK:%.0f DEF:%.0f"),
        *GetName(), AttributeComp->GetHealth(), AttributeComp->GetMaxHealth(), Attack, Defense);
}

// ========== IDamageableInterface实现 ==========

float AEnemyBase::GetCurrentHealth_Implementation() const
{
    if (AttributeComp)
    {
        return AttributeComp->GetHealth();
    }
    return 0.0f;
}

float AEnemyBase::GetMaxHealth_Implementation() const
{
    if (AttributeComp)
    {
        return AttributeComp->GetMaxHealth();
    }
    return 0.0f;
}

float AEnemyBase::GetHealthPercent_Implementation() const
{
    float MaxHealth = GetMaxHealth();
    if (MaxHealth > 0)
    {
        return GetCurrentHealth() / MaxHealth;
    }
    return 0.0f;
}

bool AEnemyBase::IsAlive_Implementation() const
{
    return !bIsDying && GetCurrentHealth() > 0;
}

bool AEnemyBase::CanBeDamaged_Implementation() const
{
    // 出生和死亡状态不受伤害
    if (StateMachineComp)
    {
        EEnemyState State = StateMachineComp->GetCurrentState();
        return State != EEnemyState::Spawn && State != EEnemyState::Dead;
    }
    return true;
}

float AEnemyBase::TakeDamage_Implementation(const FHitInfo& HitInfo)
{
    // 【强制日志】确保只要进函数就能看到，用于调试
    UE_LOG(LogEnemy, Warning, TEXT("【扣血函数进入】敌人:%s, 伤害:%.0f"), 
        *GetName(), HitInfo.Damage);
    
    UE_LOG(LogEnemy, Warning, TEXT("【扣血函数】当前血量:%.0f, IsAlive:%d"), 
        GetCurrentHealth(), IsAlive());

    if (!IsAlive())
    {
        UE_LOG(LogEnemy, Error, TEXT("【EnemyBase】%s IsAlive=false, 无法受伤! 当前血量:%.0f"), 
            *GetName(), GetCurrentHealth());
        return 0.0f;
    }

    if (!CanBeDamaged())
    {
        UE_LOG(LogEnemy, Warning, TEXT("【EnemyBase】%s 当前状态无法受伤"), *GetName());
        return 0.0f;
    }

    // 计算实际伤害
    float ActualDamage = CalculateActualDamage(HitInfo);
    UE_LOG(LogEnemy, Log, TEXT("【EnemyBase】%s 计算后实际伤害:%.0f"), *GetName(), ActualDamage);

    // 应用伤害到属性组件（负数表示伤害）
    float HealthBefore = GetCurrentHealth();
    if (AttributeComp)
    {
        UE_LOG(LogEnemy, Log, TEXT("【EnemyBase】%s 准备扣血: %.0f - %.0f"), 
            *GetName(), HealthBefore, ActualDamage);
        
        AttributeComp->ModifyHealth(-ActualDamage);
        
        UE_LOG(LogEnemy, Warning, TEXT("【EnemyBase】%s 血量变化: %.0f -> %.0f"), 
            *GetName(), HealthBefore, GetCurrentHealth());
    }
    else
    {
        UE_LOG(LogEnemy, Error, TEXT("【EnemyBase】%s AttributeComp is NULL! 无法扣血!"), *GetName());
    }

    // 触发受击反应
    PlayHitReaction(HitInfo);

    // 切换到受击状态
    if (StateMachineComp)
    {
        if (HitInfo.HitReaction == EHitReactionType::Stun)
        {
            StateMachineComp->StartStun();
            UE_LOG(LogEnemy, Log, TEXT("【EnemyBase】%s 进入眩晕状态"), *GetName());
        }
        else if (HitInfo.HitReaction != EHitReactionType::None)
        {
            StateMachineComp->StartHit();
            UE_LOG(LogEnemy, Log, TEXT("【EnemyBase】%s 进入受击状态"), *GetName());
        }
        else
        {
            UE_LOG(LogEnemy, Warning, TEXT("【EnemyBase】%s HitReaction为None，不进入受击状态"), *GetName());
        }
    }

    // 广播事件
    OnTakeDamageEvent.Broadcast(HitInfo);
    BP_OnTakeDamage(ActualDamage, HitInfo);

    // 检查死亡
    if (GetCurrentHealth() <= 0 && !bIsDying)
    {
        UE_LOG(LogEnemy, Error, TEXT("【EnemyBase】%s 血量归零，执行死亡"), *GetName());
        Die();
    }

    return ActualDamage;
}

float AEnemyBase::Heal_Implementation(float Amount)
{
    if (!IsAlive() || Amount <= 0)
    {
        return 0.0f;
    }

    if (AttributeComp)
    {
        // 获取治疗前血量
        float HealthBefore = AttributeComp->GetHealth();
        // 修改血量（正数）
        AttributeComp->ModifyHealth(Amount);
        // 返回实际治疗量
        return AttributeComp->GetHealth() - HealthBefore;
    }

    return 0.0f;
}

void AEnemyBase::Die_Implementation()
{
    if (bIsDying)
    {
        return;
    }

    bIsDying = true;

    // 切换到死亡状态
    if (StateMachineComp)
    {
        StateMachineComp->StartDeath();
    }

    // 处理死亡
    HandleDeath();

    // 广播事件
    OnDeath.Broadcast();
    BP_OnDeath();
}

float AEnemyBase::GetDefense_Implementation() const
{
    // 优先从属性组件获取（如果有）
    if (AttributeComp)
    {
        return AttributeComp->GetDefense();
    }
    // 备用：从数据资产获取
    if (EnemyData)
    {
        return EnemyData->GetDefenseForChapter(CurrentChapter);
    }
    return 0.0f;
}

float AEnemyBase::GetResistance_Implementation(EDamageType DamageType) const
{
    // 基础抗性，可由子类或数据资产扩展
    return 0.0f;
}

void AEnemyBase::PlayHitReaction_Implementation(const FHitInfo& HitInfo)
{
    // 使用HitReactionComponent处理视觉反馈
    if (HitReactionComp)
    {
        // 播放受击特效
        if (EnemyData && EnemyData->HitEffect)
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EnemyData->HitEffect, HitInfo.HitLocation);
        }

        // 播放受击音效
        if (EnemyData && EnemyData->HitSound)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), EnemyData->HitSound, GetActorLocation());
        }

        // 材质闪白
        HitReactionComp->PlayFlashEffect();

        // 击退
        if (HitInfo.KnockbackForce > 0 && StateMachineComp && StateMachineComp->CanBeKnockedBack())
        {
            HitReactionComp->ApplyKnockback(HitInfo.HitDirection, HitInfo.KnockbackForce);
        }

        // 顿帧
        if (HitInfo.bTriggerHitStop)
        {
            HitReactionComp->TriggerHitStop(HitInfo.HitStopDuration);
        }
    }
}

EHitReactionType AEnemyBase::GetHitReactionType_Implementation(float Damage, float KnockbackForce) const
{
    if (!EnemyData)
    {
        return EHitReactionType::Light;
    }

    // 根据伤害比例判断受击类型
    float HealthPercent = Damage / GetMaxHealth();

    if (KnockbackForce > 500.0f || HealthPercent > 0.3f)
    {
        return EHitReactionType::Knockdown;
    }
    else if (KnockbackForce > 200.0f || HealthPercent > 0.15f)
    {
        return EHitReactionType::Knockback;
    }
    else if (Damage > EnemyData->BaseAttack * 2)
    {
        return EHitReactionType::Heavy;
    }

    return EHitReactionType::Light;
}

// ========== 辅助函数 ==========

float AEnemyBase::CalculateActualDamage(const FHitInfo& HitInfo) const
{
    // 基础伤害计算
    float Damage = HitInfo.Damage;

    // 防御减免
    float Defense = GetDefense();
    if (Defense > 0)
    {
        // 防御公式：伤害 = Max(伤害 * 0.1, 伤害 - 防御)
        Damage = FMath::Max(Damage * 0.1f, Damage - Defense);
    }

    // 抗性减免
    float Resistance = GetResistance(HitInfo.DamageType);
    Damage *= (1.0f - FMath::Clamp(Resistance, 0.0f, 0.9f));

    return FMath::Max(0.0f, Damage);
}

void AEnemyBase::HandleDeath()
{
    if (!EnemyData)
    {
        return;
    }

    // 播放死亡动画或布娃娃
    if (EnemyData->bUseRagdoll)
    {
        PlayDeathAnimation();
        
        // 延迟启用布娃娃
        FTimerHandle RagdollTimer;
        GetWorldTimerManager().SetTimer(RagdollTimer, this, &AEnemyBase::EnableRagdoll, 0.2f, false);
    }
    else
    {
        PlayDeathAnimation();
    }

    // 生成掉落物
    SpawnLoot();

    // 设置尸体清理定时器
    if (EnemyData->CorpseLifeTime > 0)
    {
        FTimerHandle DestroyTimer;
        GetWorldTimerManager().SetTimer(DestroyTimer, this, &AEnemyBase::DestroyCorpse, EnemyData->CorpseLifeTime, false);
    }
}

void AEnemyBase::SpawnLoot()
{
    if (!EnemyData)
    {
        return;
    }

    // 播放死亡特效
    if (EnemyData->DeathEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EnemyData->DeathEffect, GetActorLocation());
    }

    // 播放死亡音效
    if (EnemyData->DeathSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), EnemyData->DeathSound, GetActorLocation());
    }

    // TODO: 实际掉落物生成（由掉落系统处理）
    BP_OnSpawnLoot();
}

void AEnemyBase::PlayDeathAnimation()
{
    if (EnemyData && EnemyData->DeathMontage)
    {
        PlayAnimMontage(EnemyData->DeathMontage);
    }
}

void AEnemyBase::EnableRagdoll()
{
    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    }

    // 禁用角色移动
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    // 设置布娃娃持续时间后清理
    if (EnemyData && EnemyData->RagdollDuration > 0)
    {
        FTimerHandle RagdollTimer;
        GetWorldTimerManager().SetTimer(RagdollTimer, this, &AEnemyBase::DestroyCorpse, EnemyData->RagdollDuration, false);
    }
}

void AEnemyBase::DestroyCorpse()
{
    Destroy();
}

// ========== 查询函数 ==========

bool AEnemyBase::IsBoss() const
{
    return EnemyData ? EnemyData->IsBoss() : false;
}

bool AEnemyBase::IsElite() const
{
    return EnemyData ? EnemyData->IsElite() : false;
}

EEnemyType AEnemyBase::GetEnemyType() const
{
    return EnemyData ? EnemyData->EnemyType : EEnemyType::Normal;
}

EEnemyState AEnemyBase::GetCurrentState() const
{
    return StateMachineComp ? StateMachineComp->GetCurrentState() : EEnemyState::None;
}

void AEnemyBase::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
}

// ========== 事件处理 ==========

void AEnemyBase::OnHealthChanged(float CurrentHealth, float MaxHealth, float Delta)
{
    // 血量变化处理（关键回调）
    UE_LOG(LogEnemy, Log, TEXT("[EnemyBase] %s 血量变化: %.0f/%.0f (变化: %+.0f)"), 
        *GetName(), CurrentHealth, MaxHealth, Delta);

    // 触发受击反馈（如果是受到伤害，即Delta为负）
    if (Delta < 0 && HitReactionComp)
    {
        // 构建一个临时的HitInfo用于触发受击反应
        FHitInfo HitInfo;
        HitInfo.Damage = -Delta;
        HitInfo.HitLocation = GetActorLocation();
        HitInfo.HitDirection = -GetActorForwardVector(); // 假设来自正面
        
        HitReactionComp->ProcessHitReaction(HitInfo);
        UE_LOG(LogEnemy, Log, TEXT("[EnemyBase] %s 触发受击反馈"), *GetName());
    }

    // 检查血量归零，触发死亡
    if (CurrentHealth <= 0.0f && !bIsDying)
    {
        UE_LOG(LogEnemy, Warning, TEXT("[EnemyBase] %s 血量归零，触发死亡"), *GetName());
        Die();
    }
}

void AEnemyBase::OnStateChanged(EEnemyState OldState, EEnemyState NewState)
{
    // 状态变更时的处理
    UE_LOG(LogEnemy, Log, TEXT("[EnemyBase] %s 状态变更: %s -> %s"),
        *GetName(),
        *UEnum::GetValueAsString(OldState),
        *UEnum::GetValueAsString(NewState));
}

void AEnemyBase::OnHitReactionReceived(const FHitInfo& HitInfo)
{
    // 受击反应处理
}

// ========== 攻击冷却管理 ==========

bool AEnemyBase::CanAttack() const
{
    // 检查是否在冷却中
    if (bIsInAttackCooldown)
    {
        return false;
    }
    
    // 检查状态机是否允许攻击
    if (StateMachineComp && !StateMachineComp->CanAttack())
    {
        return false;
    }
    
    return true;
}

void AEnemyBase::StartAttackCooldown()
{
    float BaseCooldown = 1.0f; // 默认1秒
    if (EnemyData)
    {
        BaseCooldown = EnemyData->GetRandomAttackInterval();
    }
    
    // 考虑攻击速度倍率的影响（攻击速度越快，冷却时间越短）
    float AttackSpeedMultiplier = 1.0f;
    if (EnemyData)
    {
        AttackSpeedMultiplier = EnemyData->AttackSpeedMultiplier;
    }
    
    // 确保攻击速度倍率大于0
    if (AttackSpeedMultiplier > 0.0f)
    {
        AttackCooldownTimer = BaseCooldown / AttackSpeedMultiplier;
    }
    else
    {
        AttackCooldownTimer = BaseCooldown;
    }
    
    bIsInAttackCooldown = true;
    UE_LOG(LogEnemy, Log, TEXT("[EnemyBase] %s 开始攻击冷却: %.2f秒 (攻击速度倍率: %.2f)"), *GetName(), AttackCooldownTimer, AttackSpeedMultiplier);
}
