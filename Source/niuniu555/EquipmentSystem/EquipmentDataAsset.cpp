#include "EquipmentDataAsset.h"

FEquipmentInstanceData UEquipmentDataAsset::GenerateEquipmentInstance()
{
	FEquipmentInstanceData InstanceData;

	// 设置基础信息
	InstanceData.EquipmentID.BaseID = BaseID;
	InstanceData.EquipmentID.GenerateUniqueID();
	InstanceData.Quality = BaseQuality;
	InstanceData.Slot = EquipmentSlot;
	InstanceData.Type = EquipmentType;
	InstanceData.EquipmentName = EquipmentName;
	InstanceData.EquipmentDescription = EquipmentDescription;
	InstanceData.BaseStats = BaseStats;
	InstanceData.Durability = MaxDurability;
	InstanceData.MaxDurability = MaxDurability;
	InstanceData.EnhancementLevel = 0;
	InstanceData.bIsEquipped = false;

	// 设置套装信息
	if (!SetID.IsEmpty())
	{
		InstanceData.SetInfo.SetID = SetID;
		InstanceData.SetInfo.SetName = SetName;
	}

	// TODO: 生成随机词条
	// 这里可以根据AffixPoolID和概率生成词条

	return InstanceData;
}

int32 UEquipmentDataAsset::CalculateRepairCost(int32 CurrentDurability) const
{
	if (CurrentDurability >= MaxDurability)
	{
		return 0;
	}

	float DurabilityLoss = (float)(MaxDurability - CurrentDurability) / (float)MaxDurability;
	int32 RepairCost = FMath::RoundToInt(GoldValue * DurabilityLoss * RepairCostFactor);

	return FMath::Max(1, RepairCost);
}

int32 UEquipmentDataAsset::CalculateEquipmentValue(int32 EnhancementLevel) const
{
	// 基础价值 + 强化价值
	float EnhancementMultiplier = 1.0f + (EnhancementLevel * 0.1f);
	int32 TotalValue = FMath::RoundToInt(GoldValue * EnhancementMultiplier);

	return TotalValue;
}
