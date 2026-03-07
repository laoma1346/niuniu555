// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DropTableDataAsset.generated.h"

/**
 * 掉落物品条目结构
 */
USTRUCT(BlueprintType)
struct FDropItemEntry
{
    GENERATED_BODY()

    // 掉落物品类（可以是武器、装备、道具等）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落物品")
    TSubclassOf<class AActor> ItemClass;

    // 掉落权重 - 权重越高掉落概率越大
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落物品", meta = (ClampMin = 1))
    int32 DropWeight;

    // 最小掉落数量
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落物品", meta = (ClampMin = 1))
    int32 MinQuantity;

    // 最大掉落数量
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落物品", meta = (ClampMin = 1))
    int32 MaxQuantity;

    // 保底计数 - 多少次击杀后必掉
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落物品", meta = (ClampMin = 0))
    int32 GuaranteeCount;

    FDropItemEntry()
        : DropWeight(10)
        , MinQuantity(1)
        , MaxQuantity(1)
        , GuaranteeCount(0)
    {}
};

/**
 * 掉落表数据资产 - 定义敌人死亡时的掉落物品
 * 文件路径: Source/niuniu555/EnemySystem/DropTableDataAsset.h
 */
UCLASS(Blueprintable, BlueprintType)
class NIUNIU555_API UDropTableDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UDropTableDataAsset();

    // ========== 掉落物品列表 ==========

    // 可掉落物品列表
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落表")
    TArray<FDropItemEntry> DropEntries;

    // 是否必定掉落金币
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落表")
    bool bAlwaysDropGold;

    // 必定掉落金币数量
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落表", meta = (EditCondition = "bAlwaysDropGold"))
    int32 GuaranteedGoldAmount;

    // 是否必定掉落经验
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落表")
    bool bAlwaysDropExp;

    // 必定掉落经验值
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落表", meta = (EditCondition = "bAlwaysDropExp"))
    int32 GuaranteedExpAmount;

    // 空掉落概率 (0-1) - 什么都不掉的概率
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落表", meta = (ClampMin = 0, ClampMax = 1))
    float EmptyDropChance;

    // 最大掉落物品数量 - 一次最多掉几件
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落表", meta = (ClampMin = 0))
    int32 MaxDropItems;

public:
    // 执行掉落判定，返回要生成的物品列表
    UFUNCTION(BlueprintCallable, Category = "掉落表")
    TArray<FDropItemEntry> RollDrops() const;

    // 获取总权重
    UFUNCTION(BlueprintPure, Category = "掉落表")
    int32 GetTotalWeight() const;

    // 验证掉落表是否有效
    UFUNCTION(BlueprintPure, Category = "掉落表")
    bool IsValidTable() const;
};
