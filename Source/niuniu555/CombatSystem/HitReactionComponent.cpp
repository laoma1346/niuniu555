// Copyright Epic Games, Inc. All Rights Reserved.

#include "HitReactionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

UHitReactionComponent::UHitReactionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UHitReactionComponent::BeginPlay()
{
    Super::BeginPlay();

    // 获取角色和移动组件
    Character = Cast<ACharacter>(GetOwner());
    if (Character != nullptr)
    {
        CharacterMovement = Character->GetCharacterMovement();
        SkeletalMesh = Character->GetMesh();
        
        // 初始化材质
        InitializeMaterials();
    }
}

void UHitReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 更新闪白计时器
    if (bIsFlashing)
    {
        FlashTimer -= DeltaTime;
        if (FlashTimer <= 0.0f)
        {
            StopFlashEffect();
        }
    }

    // 更新击退
    if (bIsInKnockback)
    {
        UpdateKnockback(DeltaTime);
    }
}

// ========== 受击反馈接口 ==========

void UHitReactionComponent::ProcessHitReaction(const FHitInfo& HitInfo)
{
    UE_LOG(LogTemp, Log, TEXT("[HitReaction] ProcessHitReaction 被调用 - 伤害=%.1f, 击退力度=%.1f, 方向=%s"),
        HitInfo.Damage, HitInfo.KnockbackForce, *HitInfo.HitDirection.ToString());
    
    if (HitInfo.Damage <= 0.0f && !HitInfo.bCritical)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HitReaction] 跳过：伤害<=0且非暴击"));
        return;
    }

    // 1. 播放闪白效果
    if (FlashDuration > 0.0f)
    {
        PlayFlashEffect(FlashDuration, FlashColor);
    }

    // 2. 触发顿帧（如果是暴击或重击）
    if (HitInfo.bTriggerHitStop || HitInfo.bCritical || HitInfo.HitReaction == EHitReactionType::Heavy)
    {
        TriggerHitStop(HitInfo.HitStopDuration > 0.0f ? HitInfo.HitStopDuration : HitStopDuration);
    }

    // 3. 应用击退
    if (HitInfo.KnockbackForce > 0.0f && !HitInfo.HitDirection.IsNearlyZero())
    {
        UE_LOG(LogTemp, Log, TEXT("[HitReaction] 条件满足，准备调用 ApplyKnockback"));
        ApplyKnockback(HitInfo.HitDirection, HitInfo.KnockbackForce);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[HitReaction] 跳过击退: 力度=%.1f(需要>0), 方向是否零=%s"),
            HitInfo.KnockbackForce, HitInfo.HitDirection.IsNearlyZero() ? TEXT("是") : TEXT("否"));
    }

    // 4. 播放受击动画
    PlayHitReactionAnimation(HitInfo.HitReaction);
}

void UHitReactionComponent::PlayFlashEffect(float Duration, FLinearColor Color)
{
    if (SkeletalMesh == nullptr)
    {
        return;
    }

    // 使用默认持续时间
    float ActualDuration = (Duration > 0.0f) ? Duration : FlashDuration;
    
    // 创建或获取动态材质实例
    if (FlashMaterialInstance == nullptr)
    {
        // 创建一个简单的闪白材质实例
        // 注意：实际项目中应该使用特定的受击材质
        UMaterialInterface* BaseMaterial = SkeletalMesh->GetMaterial(0);
        if (BaseMaterial != nullptr)
        {
            FlashMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        }
    }

    if (FlashMaterialInstance != nullptr)
    {
        // 设置闪白颜色参数（假设材质有Color参数）
        FlashMaterialInstance->SetVectorParameterValue(FName("HitFlashColor"), Color);
        FlashMaterialInstance->SetScalarParameterValue(FName("HitFlashIntensity"), 1.0f);

        // 应用到所有材质槽
        for (int32 i = 0; i < SkeletalMesh->GetNumMaterials(); ++i)
        {
            SkeletalMesh->SetMaterial(i, FlashMaterialInstance);
        }

        bIsFlashing = true;
        FlashTimer = ActualDuration;
    }
}

void UHitReactionComponent::StopFlashEffect()
{
    if (!bIsFlashing || SkeletalMesh == nullptr)
    {
        return;
    }

    // 恢复原始材质
    for (int32 i = 0; i < OriginalMaterials.Num() && i < SkeletalMesh->GetNumMaterials(); ++i)
    {
        if (OriginalMaterials[i] != nullptr)
        {
            SkeletalMesh->SetMaterial(i, OriginalMaterials[i]);
        }
    }

    // 清除闪白材质参数
    if (FlashMaterialInstance != nullptr)
    {
        FlashMaterialInstance->SetScalarParameterValue(FName("HitFlashIntensity"), 0.0f);
    }

    bIsFlashing = false;
    FlashTimer = 0.0f;
}

void UHitReactionComponent::TriggerHitStop(float Duration)
{
    float ActualDuration = (Duration > 0.0f) ? Duration : HitStopDuration;
    
    // 设置全局时间缩放实现顿帧效果
    // 注意：HitStopTimeDilation 默认值 0.05f（极慢而非冻结），避免完全卡死游戏
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), HitStopTimeDilation);
    
    // 使用 Unscaled 定时器恢复时间（不受时间膨胀影响，确保能触发）
    FTimerHandle HitStopTimerHandle;
    FTimerDelegate RestoreDelegate;
    RestoreDelegate.BindLambda([this]()
    {
        UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
    });
    
    // 使用 SetTimer with ETimerType::Unpaused（UE5.6 API）
    // 注意：如果 TimeDilation 很小，需要用 UnscaledTime 计算实际延迟
    float RealTimeDuration = ActualDuration * HitStopTimeDilation;
    GetWorld()->GetTimerManager().SetTimer(HitStopTimerHandle, RestoreDelegate, RealTimeDuration, false);
}

void UHitReactionComponent::ApplyKnockback(const FVector& Direction, float Force)
{
    UE_LOG(LogTemp, Log, TEXT("[HitReaction] ApplyKnockback 被调用 - 力度=%.1f, 方向=%s"),
        Force, *Direction.ToString());
    
    if (Character == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[HitReaction] ApplyKnockback 失败: Character 为空"));
        return;
    }
    
    if (CharacterMovement == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[HitReaction] ApplyKnockback 失败: CharacterMovement 为空"));
        return;
    }
    
    if (Direction.IsNearlyZero())
    {
        UE_LOG(LogTemp, Warning, TEXT("[HitReaction] ApplyKnockback 失败: 方向为零向量"));
        return;
    }

    // 计算击退速度（限制在最大速度内）
    FVector NormalizedDir = Direction.GetSafeNormal();
    float ClampedForce = FMath::Min(Force, KnockbackSpeed);
    
    // 使用 LaunchCharacter 实现击退（更可靠）
    FVector LaunchVelocity = NormalizedDir * ClampedForce;
    
    // 添加一点向上的力使击退更明显，并确保能越过小障碍
    LaunchVelocity.Z = FMath::Max(LaunchVelocity.Z, ClampedForce * 0.2f);
    
    UE_LOG(LogTemp, Log, TEXT("[HitReaction] LaunchCharacter - 速度=%s, 力度=%.1f"),
        *LaunchVelocity.ToString(), ClampedForce);
    
    // 确保 Character 可以物理移动（即使没有 Controller）
    CharacterMovement->bRunPhysicsWithNoController = true;
    
    // 先设置移动模式为 Falling，确保 LaunchCharacter 能生效
    CharacterMovement->SetMovementMode(MOVE_Falling);
    
    // 应用击退速度
    Character->LaunchCharacter(LaunchVelocity, true, true);
    
    // 记录击退状态
    CurrentKnockbackVelocity = LaunchVelocity;
    bIsInKnockback = true;
    
    // 禁用移动控制一段时间（击退期间玩家无法控制）
    // 注意：不清除速度，保留击退效果
    CharacterMovement->DisableMovement();
    
    // 使用定时器恢复移动控制（基于击退力度计算恢复时间）
    float RecoveryTime = FMath::Clamp(ClampedForce / 2000.0f, 0.1f, 0.5f);
    
    FTimerHandle KnockbackRecoveryTimer;
    FTimerDelegate RecoveryDelegate;
    RecoveryDelegate.BindLambda([this]()
    {
        if (IsValid(this) && CharacterMovement != nullptr)
        {
            CharacterMovement->SetMovementMode(MOVE_Walking);
            bIsInKnockback = false;
            CurrentKnockbackVelocity = FVector::ZeroVector;
            UE_LOG(LogTemp, Verbose, TEXT("[HitReaction] 击退恢复，移动模式重置为 Walking"));
        }
    });
    GetWorld()->GetTimerManager().SetTimer(KnockbackRecoveryTimer, RecoveryDelegate, RecoveryTime, false);
    
    UE_LOG(LogTemp, Log, TEXT("[HitReaction] 击退应用成功: 力度=%.0f, 恢复时间=%.2fs"), 
        ClampedForce, RecoveryTime);
}

void UHitReactionComponent::PlayHitReactionAnimation(EHitReactionType ReactionType)
{
    if (Character == nullptr || ReactionType == EHitReactionType::None)
    {
        return;
    }

    UAnimMontage* Montage = GetHitReactionMontage(ReactionType);
    if (Montage == nullptr)
    {
        return;
    }

    UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
    if (AnimInstance == nullptr)
    {
        return;
    }

    // 播放受击动画
    float Duration = AnimInstance->Montage_Play(Montage);
    if (Duration > 0.0f)
    {
        // 绑定动画结束回调
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UHitReactionComponent::OnHitReactionMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
    }
}

bool UHitReactionComponent::IsPlayingHitReaction() const
{
    if (Character == nullptr)
    {
        return false;
    }

    UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
    if (AnimInstance == nullptr)
    {
        return false;
    }

    // 检查是否正在播放受击动画
    return AnimInstance->IsAnyMontagePlaying();
}

// ========== 内部方法 ==========

void UHitReactionComponent::InitializeMaterials()
{
    if (SkeletalMesh == nullptr)
    {
        return;
    }

    // 保存原始材质
    OriginalMaterials.Empty();
    for (int32 i = 0; i < SkeletalMesh->GetNumMaterials(); ++i)
    {
        OriginalMaterials.Add(SkeletalMesh->GetMaterial(i));
    }
}

void UHitReactionComponent::UpdateKnockback(float DeltaTime)
{
    // 现在击退使用 LaunchCharacter + 定时器恢复模式
    // 此方法保留用于状态监控和提前结束击退
    if (!bIsInKnockback || CharacterMovement == nullptr || Character == nullptr)
    {
        return;
    }
    
    // 检查角色是否已经着地且速度接近零，如果是则提前结束击退状态
    if (CharacterMovement->IsMovingOnGround() && Character->GetVelocity().SizeSquared() < 100.0f)
    {
        bIsInKnockback = false;
        CurrentKnockbackVelocity = FVector::ZeroVector;
        // 注意：实际恢复移动由定时器控制，这里只是更新状态标志
    }
}

UAnimMontage* UHitReactionComponent::GetHitReactionMontage(EHitReactionType ReactionType) const
{
    switch (ReactionType)
    {
    case EHitReactionType::Light:
        return HitReactionLightMontage;
    case EHitReactionType::Heavy:
    case EHitReactionType::Knockback:
        return HitReactionHeavyMontage;
    case EHitReactionType::Knockdown:
        return KnockdownMontage;
    default:
        return HitReactionLightMontage;
    }
}

void UHitReactionComponent::OnHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // 动画结束，可以在这里触发后续逻辑
    UE_LOG(LogTemp, Verbose, TEXT("[%s] 受击动画结束"), *GetName());
}
