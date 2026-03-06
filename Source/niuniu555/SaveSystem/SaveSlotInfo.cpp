// Copyright Epic Games, Inc. All Rights Reserved.

#include "SaveSlotInfo.h"
#include "NiuniuSaveGame.h"
#include "SaveGameSubsystem.h"
#include "Kismet/GameplayStatics.h"

void FSaveSlotInfo::InitializeFromSaveGame(int32 InSlotIndex, UNiuniuSaveGame* SaveGame)
{
	SlotIndex = InSlotIndex;
	
	if (SaveGame)
	{
		bHasSaveData = true;
		DisplayName = SaveGame->SaveDisplayName;
		SaveVersion = SaveGame->SaveVersion;
		
		// 时间格式化
		CreateTimeString = SaveGame->PlayerData.CreateTime.ToString(TEXT("%Y-%m-%d %H:%M"));
		LastSaveTimeString = SaveGame->PlayerData.LastSaveTime.ToString(TEXT("%Y-%m-%d %H:%M"));
		
		// 游戏时长
		PlayTimeString = SaveGame->GetFormattedPlayTime();
		
		// 游戏数据
		GoldAmount = SaveGame->PlayerData.TotalGold;
		GodFragmentCount = SaveGame->PlayerData.GodFragmentCount;
		HighestChapter = SaveGame->PlayerData.HighestClearedChapter;
	}
}

FSaveSlotInfo FSaveSlotInfo::GetEmptySlot(int32 InSlotIndex)
{
	FSaveSlotInfo EmptySlot;
	EmptySlot.SlotIndex = InSlotIndex;
	EmptySlot.bHasSaveData = false;
	EmptySlot.DisplayName = FString::Printf(TEXT("空槽位 %d"), InSlotIndex + 1);
	return EmptySlot;
}

TArray<FSaveSlotInfo> USaveSystemBlueprintLibrary::GetAllSaveSlotInfos(const UObject* WorldContextObject, int32 MaxSlots)
{
	TArray<FSaveSlotInfo> SlotInfos;
	
	if (!WorldContextObject)
	{
		return SlotInfos;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		return SlotInfos;
	}

	USaveGameSubsystem* SaveSubsystem = GameInstance->GetSubsystem<USaveGameSubsystem>();
	if (!SaveSubsystem)
	{
		return SlotInfos;
	}

	// 获取所有存档信息
	TArray<UNiuniuSaveGame*> SaveGames = SaveSubsystem->GetAllSaveGamesInfo();
	
	// 创建槽位信息映射
	TMap<int32, UNiuniuSaveGame*> SaveGameMap;
	for (UNiuniuSaveGame* SaveGame : SaveGames)
	{
		if (SaveGame)
		{
			SaveGameMap.Add(SaveGame->SlotIndex, SaveGame);
		}
	}

	// 构建完整槽位列表
	for (int32 i = 0; i < MaxSlots; ++i)
	{
		FSaveSlotInfo SlotInfo;
		
		if (UNiuniuSaveGame** FoundSave = SaveGameMap.Find(i))
		{
			SlotInfo.InitializeFromSaveGame(i, *FoundSave);
		}
		else
		{
			SlotInfo = FSaveSlotInfo::GetEmptySlot(i);
		}
		
		SlotInfos.Add(SlotInfo);
	}

	return SlotInfos;
}

FSaveSlotInfo USaveSystemBlueprintLibrary::GetSaveSlotInfo(const UObject* WorldContextObject, int32 SlotIndex)
{
	if (!WorldContextObject)
	{
		return FSaveSlotInfo::GetEmptySlot(SlotIndex);
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		return FSaveSlotInfo::GetEmptySlot(SlotIndex);
	}

	USaveGameSubsystem* SaveSubsystem = GameInstance->GetSubsystem<USaveGameSubsystem>();
	if (!SaveSubsystem)
	{
		return FSaveSlotInfo::GetEmptySlot(SlotIndex);
	}

	if (SaveSubsystem->DoesSaveSlotExist(SlotIndex))
	{
		// 加载存档获取信息
		const FString SlotName = SaveSubsystem->GetSaveSlotName(SlotIndex);
		if (USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(SlotName, 0))
		{
			if (UNiuniuSaveGame* NiuniuSave = Cast<UNiuniuSaveGame>(SaveGame))
			{
				FSaveSlotInfo SlotInfo;
				SlotInfo.InitializeFromSaveGame(SlotIndex, NiuniuSave);
				return SlotInfo;
			}
		}
	}

	return FSaveSlotInfo::GetEmptySlot(SlotIndex);
}

bool USaveSystemBlueprintLibrary::DoesSaveSlotExist(const UObject* WorldContextObject, int32 SlotIndex)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		return false;
	}

	USaveGameSubsystem* SaveSubsystem = GameInstance->GetSubsystem<USaveGameSubsystem>();
	if (!SaveSubsystem)
	{
		return false;
	}

	return SaveSubsystem->DoesSaveSlotExist(SlotIndex);
}

FString USaveSystemBlueprintLibrary::FormatPlayTime(float TotalSeconds)
{
	const int32 TotalSecs = FMath::FloorToInt32(TotalSeconds);
	const int32 Hours = TotalSecs / 3600;
	const int32 Minutes = (TotalSecs % 3600) / 60;
	const int32 Seconds = TotalSecs % 60;

	if (Hours > 0)
	{
		return FString::Printf(TEXT("%d:%02d:%02d"), Hours, Minutes, Seconds);
	}
	else
	{
		return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	}
}

FString USaveSystemBlueprintLibrary::GetCurrentTimestampString()
{
	return FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"));
}
