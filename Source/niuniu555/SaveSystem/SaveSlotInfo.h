// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SaveSlotInfo.generated.h"

class UNiuniuSaveGame;

/**
 * 存档槽位信息结构体 - 用于UI显示存档列表
 */
USTRUCT(BlueprintType)
struct FSaveSlotInfo
{
	GENERATED_BODY()

	// 槽位索引
	UPROPERTY(BlueprintReadWrite, Category = "SaveSlot")
	int32 SlotIndex = 0;

	// 是否存在存档
	UPROPERTY(BlueprintReadWrite, Category = "SaveSlot")
	bool bHasSaveData = false;

	// 存档显示名称
	UPROPERTY(BlueprintReadWrite, Category = "SaveSlot")
	FString DisplayName;

	// 创建时间字符串
	UPROPERTY(BlueprintReadWrite, Category = "SaveSlot")
	FString CreateTimeString;

	// 最后保存时间字符串
	UPROPERTY(BlueprintReadWrite, Category = "SaveSlot")
	FString LastSaveTimeString;

	// 游戏时长字符串（格式化后）
	UPROPERTY(BlueprintReadWrite, Category = "SaveSlot")
	FString PlayTimeString;

	// 金币数量
	UPROPERTY(BlueprintReadWrite, Category = "SaveSlot")
	int32 GoldAmount = 0;

	// 神格碎片数量
	UPROPERTY(BlueprintReadWrite, Category = "SaveSlot")
	int32 GodFragmentCount = 0;

	// 最高通关章节
	UPROPERTY(BlueprintReadWrite, Category = "SaveSlot")
	int32 HighestChapter = 0;

	// 存档版本号
	UPROPERTY(BlueprintReadWrite, Category = "SaveSlot")
	int32 SaveVersion = 0;

	// 默认构造函数
	FSaveSlotInfo() = default;

	// 从SaveGame对象创建槽位信息
	void InitializeFromSaveGame(int32 InSlotIndex, UNiuniuSaveGame* SaveGame);

	// 获取空槽位信息
	static FSaveSlotInfo GetEmptySlot(int32 InSlotIndex);
};

/**
 * 存档管理蓝图函数库 - 提供便捷的存档操作函数
 */
UCLASS()
class NIUNIU555_API USaveSystemBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 获取所有存档槽位信息（用于存档列表UI）
	 * @param WorldContextObject 世界上下文
	 * @param MaxSlots 最大槽位数
	 * @return 存档槽位信息数组
	 */
	UFUNCTION(BlueprintPure, Category = "SaveSystem|Library", meta = (WorldContext = "WorldContextObject"))
	static TArray<FSaveSlotInfo> GetAllSaveSlotInfos(const UObject* WorldContextObject, int32 MaxSlots = 5);

	/**
	 * 获取指定存档槽位信息
	 * @param WorldContextObject 世界上下文
	 * @param SlotIndex 槽位索引
	 * @return 存档槽位信息
	 */
	UFUNCTION(BlueprintPure, Category = "SaveSystem|Library", meta = (WorldContext = "WorldContextObject"))
	static FSaveSlotInfo GetSaveSlotInfo(const UObject* WorldContextObject, int32 SlotIndex);

	/**
	 * 检查存档槽位是否存在
	 * @param WorldContextObject 世界上下文
	 * @param SlotIndex 槽位索引
	 * @return 是否存在
	 */
	UFUNCTION(BlueprintPure, Category = "SaveSystem|Library", meta = (WorldContext = "WorldContextObject"))
	static bool DoesSaveSlotExist(const UObject* WorldContextObject, int32 SlotIndex);

	/**
	 * 格式化游戏时长为可读字符串
	 * @param TotalSeconds 总秒数
	 * @return 格式化后的字符串（如 "02:30" 或 "1:25:30"）
	 */
	UFUNCTION(BlueprintPure, Category = "SaveSystem|Library")
	static FString FormatPlayTime(float TotalSeconds);

	/**
	 * 获取当前系统时间字符串
	 * @return 格式化的时间字符串
	 */
	UFUNCTION(BlueprintPure, Category = "SaveSystem|Library")
	static FString GetCurrentTimestampString();
};
