// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DropItemBase.h"
#include "EnemyTypes.h"
#include "DivineFragmentDropItem.generated.h"

/**
 * 神格碎片掉落物 - 玩家拾取后获得神格碎片
 * 用于技能树解锁和升级
 * 
 * 【重要】碎片颜色在蓝图默认值中配置，不在C++构造函数中初始化
 * 避免编辑器崩溃：UE5中C++构造函数操作TMap可能导致内存损坏
 */
UCLASS(Blueprintable, BlueprintType)
class NIUNIU555_API ADivineFragmentDropItem : public ADropItemBase
{
    GENERATED_BODY()

public:
    ADivineFragmentDropItem(const FObjectInitializer& ObjectInitializer);

    //~ Begin ADropItemBase Interface
    virtual void InitializeDrop(const FVector& SpawnLocation, const FVector& SpawnImpulse) override;
    virtual int32 GetValue() const override { return FragmentCount; }
    //~ End ADropItemBase Interface

    /** 设置碎片类型 */
    UFUNCTION(BlueprintCallable, Category = "神格碎片")
    void SetFragmentType(EDivineFragmentType InType);

    /** 获取碎片类型 */
    UFUNCTION(BlueprintPure, Category = "神格碎片")
    EDivineFragmentType GetFragmentType() const { return FragmentType; }

    /** 设置碎片数量 */
    UFUNCTION(BlueprintCallable, Category = "神格碎片")
    void SetFragmentCount(int32 Count);

protected:
    /** 碎片类型 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "神格碎片")
    EDivineFragmentType FragmentType = EDivineFragmentType::Universal;

    /** 碎片数量 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "神格碎片", meta = (ClampMin = "1"))
    int32 FragmentCount = 1;

    /** 碎片图标 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "神格碎片")
    TSoftObjectPtr<UTexture2D> FragmentIcon;

    /** 
     * 碎片颜色映射表
     * 【配置方式】在蓝图默认值(BP_DivineFragment)中配置：
     *   - Universal: 金色 (1.0, 0.85, 0.35)
     *   - WarPath: 红色 (0.9, 0.2, 0.2)
     *   - WealthPath: 蓝色 (0.2, 0.5, 0.9)
     *   - SupportPath: 绿色 (0.4, 0.9, 0.4)
     * 【警告】不要在C++构造函数中操作此TMap，会导致编辑器崩溃！
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "视觉效果")
    TMap<EDivineFragmentType, FLinearColor> FragmentColors;

    /** 拾取音效 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "音效")
    TSoftObjectPtr<USoundBase> PickupSound;

    /** 拾取特效 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "特效")
    TSoftObjectPtr<UParticleSystem> PickupEffect;

protected:
    //~ Begin ADropItemBase Interface
    virtual void OnPickup_Implementation(APawn* Player) override;
    //~ End ADropItemBase Interface

    /** 获取碎片颜色 */
    UFUNCTION(BlueprintPure, Category = "神格碎片")
    FLinearColor GetFragmentColor() const;

    /** 应用碎片到玩家（由子类或蓝图实现） */
    UFUNCTION(BlueprintNativeEvent, Category = "神格碎片")
    void ApplyFragmentToPlayer(APawn* Player, EDivineFragmentType InType, int32 Count);
    virtual void ApplyFragmentToPlayer_Implementation(APawn* Player, EDivineFragmentType InType, int32 Count);
};
