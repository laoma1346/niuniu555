#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EquipmentTypes.h"
#include "EquipmentDataAsset.generated.h"

/**
 * 装备数据资产
 * 用于在蓝图中配置装备的基础属性、模型、音效等
 */
UCLASS(BlueprintType)
class NIUNIU555_API UEquipmentDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * 装备基础ID
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础信息")
	FString BaseID;

	/**
	 * 装备名称
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础信息")
	FString EquipmentName;

	/**
	 * 装备描述
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础信息", meta = (MultiLine = true))
	FString EquipmentDescription;

	/**
	 * 装备类型
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备类型")
	EEquipmentType EquipmentType;

	/**
	 * 装备槽位
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备类型")
	EEquipmentSlot EquipmentSlot;

	/**
	 * 装备品质
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备品质")
	EEquipmentQuality BaseQuality;

	/**
	 * 基础属性
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "属性")
	FEquipmentBaseStats BaseStats;

	/**
	 * 装备模型
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "外观")
	USkeletalMesh* EquipmentMesh;

	/**
	 * 装备材质
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "外观")
	UMaterialInterface* EquipmentMaterial;

	/**
	 * 装备图标
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "外观")
	UTexture2D* EquipmentIcon;

	/**
	 * 装备音效
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "音效")
	USoundBase* EquipSound;

	/**
	 * 装备音效
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "音效")
	USoundBase* UnequipSound;

	/**
	 * 套装ID
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "套装")
	FString SetID;

	/**
	 * 套装名称
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "套装")
	FString SetName;

	/**
	 * 最大耐久度
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "耐久度")
	int32 MaxDurability = 100;

	/**
	 * 装备价值（金币）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "经济")
	int32 GoldValue = 100;

	/**
	 * 修理成本系数
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "经济")
	float RepairCostFactor = 0.1f;

	/**
	 * 分解获得材料数量
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "经济")
	int32 ScrapMaterialAmount = 5;

	/**
	 * 词条池ID
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "词条")
	FString AffixPoolID;

	/**
	 * 最大词条数量
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "词条")
	int32 MaxAffixCount = 3;

	/**
	 * 前缀概率
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "词条")
	float PrefixChance = 0.5f;

	/**
	 * 后缀概率
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "词条")
	float SuffixChance = 0.5f;

	/**
	 * 生成装备实例数据
	 * @return 装备实例数据
	 */
	UFUNCTION(BlueprintCallable, Category = "装备")
	FEquipmentInstanceData GenerateEquipmentInstance();

	/**
	 * 计算修理成本
	 * @param CurrentDurability 当前耐久度
	 * @return 修理成本
	 */
	UFUNCTION(BlueprintCallable, Category = "装备")
	int32 CalculateRepairCost(int32 CurrentDurability) const;

	/**
	 * 计算装备价值
	 * @param EnhancementLevel 强化等级
	 * @return 装备价值
	 */
	UFUNCTION(BlueprintCallable, Category = "装备")
	int32 CalculateEquipmentValue(int32 EnhancementLevel) const;
};
