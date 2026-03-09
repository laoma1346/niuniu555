// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DropItemBase.h"
#include "EquipmentSystem/EquipmentTypes.h"
#include "EquipmentDropItem.generated.h"

/**
 * 装备掉落物 - 玩家拾取后获得装备
 */
UCLASS(Blueprintable)
class NIUNIU555_API AEquipmentDropItem : public ADropItemBase
{
    GENERATED_BODY()

public:
    AEquipmentDropItem(const FObjectInitializer& ObjectInitializer);

    //~ Begin ADropItemBase Interface
    virtual void InitializeDrop(const FVector& SpawnLocation, const FVector& SpawnImpulse) override;
    virtual int32 GetValue() const override { return static_cast<int32>(Quality); }
    //~ End ADropItemBase Interface

    /** 设置装备品质 */
    UFUNCTION(BlueprintCallable, Category = "装备")
    void SetEquipmentQuality(EEquipmentQuality InQuality);

    /** 获取装备品质 */
    UFUNCTION(BlueprintPure, Category = "装备")
    EEquipmentQuality GetEquipmentQuality() const { return Quality; }

    /** 获取品质颜色（用于特效） */
    UFUNCTION(BlueprintPure, Category = "装备")
    FLinearColor GetQualityColor() const;

protected:
    /** 装备品质 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备")
    EEquipmentQuality Quality = EEquipmentQuality::Common;

    /** 装备名称（显示用） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备")
    FText EquipmentName;

    /** 装备图标 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "装备")
    TSoftObjectPtr<UTexture2D> EquipmentIcon;

    /** 品质对应的颜色 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "视觉效果")
    TMap<EEquipmentQuality, FLinearColor> QualityColors;

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

    /** 根据品质更新网格体材质颜色 */
    UFUNCTION()
    void UpdateMeshColor();

    /** 应用装备到玩家（由子类或蓝图实现） */
    UFUNCTION(BlueprintNativeEvent, Category = "装备")
    void ApplyEquipmentToPlayer(APawn* Player, EEquipmentQuality InQuality);
    virtual void ApplyEquipmentToPlayer_Implementation(APawn* Player, EEquipmentQuality InQuality);
};
