#include "ChapterConfigDataAsset.h"

UChapterConfigDataAsset::UChapterConfigDataAsset()
{
	ChapterID = 1;
	ChapterName = TEXT("第一章");
	NumLayers = 3;
	MinRoomsPerLayer = 2;
	MaxRoomsPerLayer = 4;
	BranchProbability = 0.3f;
	MinShopRooms = 1;
	EventRoomProbability = 0.3f;
	SkillRoomProbability = 0.15f;
	EliteRoomProbability = 0.2f;
	EnemyDifficultyMultiplier = 1.0f;
	GoldRewardMultiplier = 1.0f;
	ExpRewardMultiplier = 1.0f;
	EquipmentDropRateBonus = 0.0f;
	AmbientLightColor = FLinearColor::White;
}

int32 UChapterConfigDataAsset::GetEstimatedRoomCount() const
{
	// 估算房间数：起点 + 各层房间 + BOSS房
	int32 AvgRoomsPerLayer = (MinRoomsPerLayer + MaxRoomsPerLayer) / 2;
	return 1 + (AvgRoomsPerLayer * NumLayers) + 1;
}

bool UChapterConfigDataAsset::IsValidConfig() const
{
	// 检查基本有效性
	if (NumLayers < 2)
	{
		return false;
	}

	if (MinRoomsPerLayer > MaxRoomsPerLayer)
	{
		return false;
	}

	if (MinRoomsPerLayer < 1)
	{
		return false;
	}

	// 检查概率总和是否合理
	float TotalProbability = EventRoomProbability + SkillRoomProbability + EliteRoomProbability;
	if (TotalProbability > 1.0f)
	{
		return false;
	}

	return true;
}

FString UChapterConfigDataAsset::GetDifficultyDescription() const
{
	if (EnemyDifficultyMultiplier < 0.8f)
	{
		return TEXT("简单");
	}
	else if (EnemyDifficultyMultiplier < 1.2f)
	{
		return TEXT("普通");
	}
	else if (EnemyDifficultyMultiplier < 1.8f)
	{
		return TEXT("困难");
	}
	else
	{
		return TEXT("极难");
	}
}
