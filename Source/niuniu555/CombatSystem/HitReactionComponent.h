// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DamageableInterface.h"
#include "HitReactionComponent.generated.h"

class UCharacterMovementComponent;
class USkeletalMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UAnimMontage;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NIUNIU555_API UHitReactionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHitReactionComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // ========== 受击反馈配置 ==========

    // 材质闪白持续时间（秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "受击反馈|闪白")
    float FlashDuration = 0.15f;

    // 闪白颜色
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "受击反馈|闪白")
    FLinearColor FlashColor = FLinearColor(1.0f, 0.3f, 0.3f, 1.0f);

    // 顿帧持续时间（秒）- 冻结游戏时间
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "受击反馈|顿帧")
    float HitStopDuration = 0.05f;

    // 顿帧时间缩放（0.01 = 极慢，产生"卡顿感"而非完全冻结；推荐 0.05-0.1）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "受击反馈|顿帧", meta = (ClampMin = 0.01, ClampMax = 1))
    float HitStopTimeDilation = 0.05f;

    // 击退速度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "受击反馈|击退")
    float KnockbackSpeed = 800.0f;

    // 击退减速度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "受击反馈|击退")
    float KnockbackDeceleration = 2000.0f;

    // 受击动画蒙太奇（轻击）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "受击反馈|动画")
    TObjectPtr<UAnimMontage> HitReactionLightMontage;

    // 受击动画蒙太奇（重击）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "受击反馈|动画")
    TObjectPtr<UAnimMontage> HitReactionHeavyMontage;

    // 击倒动画蒙太奇
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "受击反馈|动画")
    TObjectPtr<UAnimMontage> KnockdownMontage;

public:
    // ========== 受击反馈接口 ==========

    /**
     * 处理受击反馈
     * @param HitInfo 受击信息
     */
    UFUNCTION(BlueprintCallable, Category = "受击反馈")
    void ProcessHitReaction(const FHitInfo& HitInfo);

    /**
     * 播放材质闪白效果
     * @param Duration 持续时间
     * @param Color 闪白颜色
     */
    UFUNCTION(BlueprintCallable, Category = "受击反馈")
    void PlayFlashEffect(float Duration = -1.0f, FLinearColor Color = FLinearColor::White);

    /**
     * 停止闪白效果
     */
    UFUNCTION(BlueprintCallable, Category = "受击反馈")
    void StopFlashEffect();

    /**
     * 触发顿帧（Hit Stop）
     * @param Duration 顿帧持续时间
     */
    UFUNCTION(BlueprintCallable, Category = "受击反馈")
    void TriggerHitStop(float Duration = -1.0f);

    /**
     * 应用击退
     * @param Direction 击退方向
     * @param Force 击退力度
     */
    UFUNCTION(BlueprintCallable, Category = "受击反馈")
    void ApplyKnockback(const FVector& Direction, float Force);

    /**
     * 播放受击动画
     * @param ReactionType 受击反应类型
     */
    UFUNCTION(BlueprintCallable, Category = "受击反馈")
    void PlayHitReactionAnimation(EHitReactionType ReactionType);

    /**
     * 是否正在播放受击动画
     */
    UFUNCTION(BlueprintPure, Category = "受击反馈")
    bool IsPlayingHitReaction() const;

protected:
    // ========== 运行时状态 ==========

    // 是否正在闪白
    UPROPERTY()
    bool bIsFlashing = false;

    // 闪白计时器
    UPROPERTY()
    float FlashTimer = 0.0f;

    // 击退速度
    UPROPERTY()
    FVector CurrentKnockbackVelocity = FVector::ZeroVector;

    // 是否正在击退中
    UPROPERTY()
    bool bIsInKnockback = false;

    // 角色缓存
    UPROPERTY()
    TObjectPtr<ACharacter> Character = nullptr;
    
    // 角色移动组件缓存
    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> CharacterMovement = nullptr;

    // 骨骼网格体缓存（用于材质效果）
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> SkeletalMesh = nullptr;

    // 原始材质数组
    UPROPERTY()
    TArray<TObjectPtr<UMaterialInterface>> OriginalMaterials;

    // 动态材质实例
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> FlashMaterialInstance = nullptr;

protected:
    // ========== 内部方法 ==========

    // 初始化材质
    UFUNCTION()
    void InitializeMaterials();

    // 更新击退
    UFUNCTION()
    void UpdateKnockback(float DeltaTime);

    // 获取受击动画
    UFUNCTION()
    UAnimMontage* GetHitReactionMontage(EHitReactionType ReactionType) const;

    // 动画结束回调
    UFUNCTION()
    void OnHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
