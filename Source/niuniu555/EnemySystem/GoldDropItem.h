// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DropItemBase.h"
#include "GoldDropItem.generated.h"

/**
 * 金币掉落物 - 玩家拾取后获得金币
 */
UCLASS(Blueprintable)
class NIUNIU555_API AGoldDropItem : public ADropItemBase
{
    GENERATED_BODY()

public:
    /** 
     * 金币掉落物
     * 继承的配置（在蓝图"类默认值"中查看）：
     * - 吸附配置：bEnableAutoMagnet, MagnetRange, MagnetSpeed
     * - 生命周期：LifeTime, BlinkStartTime
     */
    AGoldDropItem(const FObjectInitializer& ObjectInitializer);

    //~ Begin ADropItemBase Interface
    virtual void InitializeDrop(const FVector& SpawnLocation, const FVector& SpawnImpulse) override;
    virtual int32 GetValue() const override { return GoldAmount; }
    //~ End ADropItemBase Interface

    /** 设置金币数量 */
    UFUNCTION(BlueprintCallable, Category = "金币")
    void SetGoldAmount(int32 Amount);

    /** 获取金币数量 */
    UFUNCTION(BlueprintPure, Category = "金币")
    int32 GetGoldAmount() const { return GoldAmount; }

protected:
    /** 金币数量 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "金币", meta = (ClampMin = "1"))
    int32 GoldAmount = 10;

    /** 金币拾取音效 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "音效")
    TSoftObjectPtr<USoundBase> PickupSound;

    /** 金币拾取特效 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "特效")
    TSoftObjectPtr<UParticleSystem> PickupEffect;

protected:
    //~ Begin ADropItemBase Interface
    virtual void OnPickup_Implementation(APawn* Player) override;
    //~ End ADropItemBase Interface

    /** 应用金币到玩家（由子类或蓝图实现） */
    UFUNCTION(BlueprintNativeEvent, Category = "金币")
    void ApplyGoldToPlayer(APawn* Player, int32 Amount);
    virtual void ApplyGoldToPlayer_Implementation(APawn* Player, int32 Amount);
};
