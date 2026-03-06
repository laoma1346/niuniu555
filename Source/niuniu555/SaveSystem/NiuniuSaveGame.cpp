// Copyright Epic Games, Inc. All Rights Reserved.

#include "NiuniuSaveGame.h"

UNiuniuSaveGame::UNiuniuSaveGame()
{
	// 初始化创建时间
	PlayerData.CreateTime = FDateTime::Now();
	PlayerData.LastSaveTime = PlayerData.CreateTime;
	SaveDisplayName = TEXT("新存档");
}

FString UNiuniuSaveGame::GetFormattedPlayTime() const
{
	const int32 TotalSeconds = FMath::FloorToInt32(PlayerData.TotalPlayTimeSeconds);
	const int32 Hours = TotalSeconds / 3600;
	const int32 Minutes = (TotalSeconds % 3600) / 60;
	const int32 Seconds = TotalSeconds % 60;

	if (Hours > 0)
	{
		return FString::Printf(TEXT("%d:%02d:%02d"), Hours, Minutes, Seconds);
	}
	else
	{
		return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	}
}

void UNiuniuSaveGame::UpdateLastSaveTime()
{
	PlayerData.LastSaveTime = FDateTime::Now();
}
