// Copyright Epic Games, Inc. All Rights Reserved.

#include "DropTableDataAsset.h"

UDropTableDataAsset::UDropTableDataAsset()
{
    bAlwaysDropGold = true;
    GuaranteedGoldAmount = 10;
    bAlwaysDropExp = true;
    GuaranteedExpAmount = 10;
    EmptyDropChance = 0.0f;
    MaxDropItems = 3;
}

TArray<FDropItemEntry> UDropTableDataAsset::RollDrops() const
{
    TArray<FDropItemEntry> Result;

    // 检查是否空掉落
    if (FMath::FRand() < EmptyDropChance)
    {
        return Result;
    }

    // 计算总权重
    int32 TotalWeight = GetTotalWeight();
    if (TotalWeight <= 0)
    {
        return Result;
    }

    // 确定要掉落的物品数量
    int32 NumItemsToDrop = FMath::Min(MaxDropItems, DropEntries.Num());
    
    // 简单的权重随机抽取
    TArray<int32> SelectedIndices;
    for (int32 i = 0; i < NumItemsToDrop; ++i)
    {
        int32 RandomValue = FMath::RandRange(0, TotalWeight - 1);
        int32 CurrentWeight = 0;

        for (int32 j = 0; j < DropEntries.Num(); ++j)
        {
            // 跳过已选中的
            if (SelectedIndices.Contains(j))
            {
                continue;
            }

            CurrentWeight += DropEntries[j].DropWeight;
            if (RandomValue < CurrentWeight)
            {
                Result.Add(DropEntries[j]);
                SelectedIndices.Add(j);
                break;
            }
        }
    }

    return Result;
}

int32 UDropTableDataAsset::GetTotalWeight() const
{
    int32 Total = 0;
    for (const FDropItemEntry& Entry : DropEntries)
    {
        Total += Entry.DropWeight;
    }
    return Total;
}

bool UDropTableDataAsset::IsValidTable() const
{
    // 检查是否有有效条目
    for (const FDropItemEntry& Entry : DropEntries)
    {
        if (Entry.ItemClass != nullptr && Entry.DropWeight > 0)
        {
            return true;
        }
    }
    
    // 即使没有物品条目，如果有必定掉落也算有效
    return bAlwaysDropGold || bAlwaysDropExp;
}
