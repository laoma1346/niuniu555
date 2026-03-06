// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "NiuniuSaveGame.generated.h"

/**
 * 玩家存档数据结构 - 局外持久化数据
 */
USTRUCT(BlueprintType)
struct FPlayerPersistentData
{
	GENERATED_BODY()

	// 金币总量
	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	int32 TotalGold = 0;

	// 已解锁的技能ID列表
	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	TArray<FName> UnlockedSkills;

	// 最高通关章节
	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	int32 HighestClearedChapter = 0;

	// 神格碎片数量
	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	int32 GodFragmentCount = 0;

	// 已拥有的装备ID列表
	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	TArray<FName> OwnedEquipmentIds;

	// 存档创建时间
	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	FDateTime CreateTime;

	// 最后保存时间
	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	FDateTime LastSaveTime;

	// 游戏时长（秒）
	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	float TotalPlayTimeSeconds = 0.0f;
};

/**
 * 存档游戏类 - 存储所有需要持久化的游戏数据
 */
UCLASS()
class NIUNIU555_API UNiuniuSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UNiuniuSaveGame();

	// 玩家持久化数据
	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	FPlayerPersistentData PlayerData;

	// 存档版本号（用于兼容性处理）
	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	int32 SaveVersion = 1;

	// 存档槽位索引
	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	int32 SlotIndex = 0;

	// 存档显示名称
	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	FString SaveDisplayName;

	// 获取格式化后的游戏时长字符串
	UFUNCTION(BlueprintPure, Category = "SaveData")
	FString GetFormattedPlayTime() const;

	// 更新最后保存时间
	UFUNCTION(BlueprintCallable, Category = "SaveData")
	void UpdateLastSaveTime();
};
