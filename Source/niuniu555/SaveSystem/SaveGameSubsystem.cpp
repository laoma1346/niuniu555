// Copyright Epic Games, Inc. All Rights Reserved.

#include "SaveGameSubsystem.h"
#include "NiuniuSaveGame.h"
#include "Kismet/GameplayStatics.h"

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("SaveGameSubsystem: Initialized"));
}

void USaveGameSubsystem::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("SaveGameSubsystem: Deinitialized"));
}

FString USaveGameSubsystem::GetSaveSlotName(int32 SlotIndex) const
{
	return FString::Printf(TEXT("%s_%d"), *SaveSlotPrefix, SlotIndex);
}

bool USaveGameSubsystem::DoesSaveSlotExist(int32 SlotIndex) const
{
	const FString SlotName = GetSaveSlotName(SlotIndex);
	return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

void USaveGameSubsystem::CreateNewSaveGame(int32 SlotIndex, const FString& SaveName)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		OnSaveOperationCompleted.Broadcast(false, TEXT("无效的存档槽位"));
		return;
	}

	// 创建新的存档对象
	UNiuniuSaveGame* NewSaveGame = Cast<UNiuniuSaveGame>(UGameplayStatics::CreateSaveGameObject(UNiuniuSaveGame::StaticClass()));
	if (NewSaveGame)
	{
		NewSaveGame->SlotIndex = SlotIndex;
		NewSaveGame->SaveDisplayName = SaveName.IsEmpty() ? FString::Printf(TEXT("存档 %d"), SlotIndex + 1) : SaveName;
		NewSaveGame->UpdateLastSaveTime();

		// 异步保存
		const FString SlotName = GetSaveSlotName(SlotIndex);
		FAsyncSaveGameToSlotDelegate SavedDelegate;
		SavedDelegate.BindUObject(this, &USaveGameSubsystem::OnCreateSaveGameCompleted);
		UGameplayStatics::AsyncSaveGameToSlot(NewSaveGame, SlotName, 0, SavedDelegate);
	}
	else
	{
		OnSaveOperationCompleted.Broadcast(false, TEXT("创建存档对象失败"));
	}
}

void USaveGameSubsystem::LoadSaveGame(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		OnLoadOperationCompleted.Broadcast(false, TEXT("无效的存档槽位"));
		return;
	}

	if (!DoesSaveSlotExist(SlotIndex))
	{
		OnLoadOperationCompleted.Broadcast(false, TEXT("存档不存在"));
		return;
	}

	const FString SlotName = GetSaveSlotName(SlotIndex);
	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	LoadedDelegate.BindUObject(this, &USaveGameSubsystem::OnLoadSaveGameCompleted);
	UGameplayStatics::AsyncLoadGameFromSlot(SlotName, 0, LoadedDelegate);
}

void USaveGameSubsystem::SaveCurrentGame()
{
	if (!CurrentSaveGame)
	{
		OnSaveOperationCompleted.Broadcast(false, TEXT("没有当前存档可保存"));
		return;
	}

	CurrentSaveGame->UpdateLastSaveTime();
	const FString SlotName = GetSaveSlotName(CurrentSaveGame->SlotIndex);
	
	FAsyncSaveGameToSlotDelegate SavedDelegate;
	SavedDelegate.BindUObject(this, &USaveGameSubsystem::OnSaveGameCompleted);
	UGameplayStatics::AsyncSaveGameToSlot(CurrentSaveGame, SlotName, 0, SavedDelegate);
}

bool USaveGameSubsystem::DeleteSaveGame(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		return false;
	}

	const FString SlotName = GetSaveSlotName(SlotIndex);
	if (UGameplayStatics::DeleteGameInSlot(SlotName, 0))
	{
		// 如果删除的是当前加载的存档，清空当前存档
		if (CurrentSaveGame && CurrentSaveGame->SlotIndex == SlotIndex)
		{
			CurrentSaveGame = nullptr;
			CurrentSlotIndex = -1;
		}
		return true;
	}
	return false;
}

TArray<UNiuniuSaveGame*> USaveGameSubsystem::GetAllSaveGamesInfo() const
{
	TArray<UNiuniuSaveGame*> SaveGames;
	
	for (int32 i = 0; i < MaxSaveSlots; ++i)
	{
		if (DoesSaveSlotExist(i))
		{
			const FString SlotName = GetSaveSlotName(i);
			if (USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(SlotName, 0))
			{
				if (UNiuniuSaveGame* NiuniuSave = Cast<UNiuniuSaveGame>(SaveGame))
				{
					SaveGames.Add(NiuniuSave);
				}
			}
		}
	}
	
	return SaveGames;
}

void USaveGameSubsystem::UpdateGold(int32 Amount)
{
	if (CurrentSaveGame)
	{
		CurrentSaveGame->PlayerData.TotalGold += Amount;
		CurrentSaveGame->PlayerData.TotalGold = FMath::Max(0, CurrentSaveGame->PlayerData.TotalGold);
	}
}

void USaveGameSubsystem::AddPlayTime(float DeltaSeconds)
{
	if (CurrentSaveGame)
	{
		CurrentSaveGame->PlayerData.TotalPlayTimeSeconds += DeltaSeconds;
	}
}

void USaveGameSubsystem::UnlockSkill(const FName& SkillId)
{
	if (CurrentSaveGame && !SkillId.IsNone())
	{
		if (!CurrentSaveGame->PlayerData.UnlockedSkills.Contains(SkillId))
		{
			CurrentSaveGame->PlayerData.UnlockedSkills.Add(SkillId);
		}
	}
}

void USaveGameSubsystem::AddGodFragments(int32 Amount)
{
	if (CurrentSaveGame)
	{
		CurrentSaveGame->PlayerData.GodFragmentCount += Amount;
		CurrentSaveGame->PlayerData.GodFragmentCount = FMath::Max(0, CurrentSaveGame->PlayerData.GodFragmentCount);
	}
}

void USaveGameSubsystem::UpdateHighestChapter(int32 ChapterIndex)
{
	if (CurrentSaveGame)
	{
		CurrentSaveGame->PlayerData.HighestClearedChapter = FMath::Max(
			CurrentSaveGame->PlayerData.HighestClearedChapter, ChapterIndex);
	}
}

void USaveGameSubsystem::OnCreateSaveGameCompleted(const FString& SlotName, int32 UserIndex, bool bSuccess)
{
	if (bSuccess)
	{
		// 重新加载以获取保存后的对象
		LoadSaveGame(FCString::Atoi(*SlotName.RightChop(SaveSlotPrefix.Len() + 1)));
	}
	else
	{
		OnSaveOperationCompleted.Broadcast(false, TEXT("保存存档失败"));
	}
}

void USaveGameSubsystem::OnLoadSaveGameCompleted(const FString& SlotName, int32 UserIndex, USaveGame* LoadedSaveGame)
{
	if (UNiuniuSaveGame* NiuniuSave = Cast<UNiuniuSaveGame>(LoadedSaveGame))
	{
		CurrentSaveGame = NiuniuSave;
		CurrentSlotIndex = NiuniuSave->SlotIndex;
		OnLoadOperationCompleted.Broadcast(true, TEXT("加载成功"));
	}
	else
	{
		OnLoadOperationCompleted.Broadcast(false, TEXT("存档数据无效"));
	}
}

void USaveGameSubsystem::OnSaveGameCompleted(const FString& SlotName, int32 UserIndex, bool bSuccess)
{
	OnSaveOperationCompleted.Broadcast(bSuccess, bSuccess ? TEXT("保存成功") : TEXT("保存失败"));
}
