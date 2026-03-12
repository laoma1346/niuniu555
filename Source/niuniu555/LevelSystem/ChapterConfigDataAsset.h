#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelTypes.h"
#include "ChapterConfigDataAsset.generated.h"

/**
 * 章节配置数据资产
 * 用于配置每个章节的参数
 */
UCLASS(BlueprintType)
class NIUNIU555_API UChapterConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UChapterConfigDataAsset();

	// ========== 章节基本信息 ==========

	/** 章节ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "章节信息")
	int32 ChapterID = 1;

	/** 章节名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "章节信息")
	FString ChapterName = TEXT("第一章");

	/** 章节描述 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "章节信息", meta = (MultiLine = true))
	FString ChapterDescription;

	/** 章节预览图 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "章节信息")
	UTexture2D* ChapterPreviewImage;

	// ========== 地图生成配置 ==========

	/** 章节层数 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "地图配置", meta = (ClampMin = "2", ClampMax = "10"))
	int32 NumLayers = 3;

	/** 每层最少房间数 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "地图配置", meta = (ClampMin = "1", ClampMax = "5"))
	int32 MinRoomsPerLayer = 2;

	/** 每层最多房间数 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "地图配置", meta = (ClampMin = "2", ClampMax = "6"))
	int32 MaxRoomsPerLayer = 4;

	/** 分支概率 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "地图配置", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BranchProbability = 0.3f;

	// ========== 房间类型概率 ==========

	/** 商店房每层最少数量 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "房间概率", meta = (ClampMin = "0"))
	int32 MinShopRooms = 1;

	/** 事件房出现概率 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "房间概率", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EventRoomProbability = 0.3f;

	/** 技能房出现概率 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "房间概率", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SkillRoomProbability = 0.15f;

	/** 精英房出现概率 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "房间概率", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EliteRoomProbability = 0.2f;

	// ========== 战斗配置 ==========

	/** 敌人难度系数 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "战斗配置", meta = (ClampMin = "0.5", ClampMax = "5.0"))
	float EnemyDifficultyMultiplier = 1.0f;

	/** BOSS配置 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "战斗配置")
	FBossConfig BossConfig;

	// ========== 奖励配置 ==========

	/** 基础金币奖励倍数 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "奖励配置", meta = (ClampMin = "0.5", ClampMax = "5.0"))
	float GoldRewardMultiplier = 1.0f;

	/** 基础经验奖励倍数 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "奖励配置", meta = (ClampMin = "0.5", ClampMax = "5.0"))
	float ExpRewardMultiplier = 1.0f;

	/** 装备掉落率加成 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "奖励配置", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EquipmentDropRateBonus = 0.0f;

	// ========== 环境配置 ==========

	/** 背景音乐 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "环境配置")
	USoundBase* BackgroundMusic;

	/** 环境特效 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "环境配置")
	UParticleSystem* AmbientEffect;

	/** 场景光照配置 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "环境配置")
	FLinearColor AmbientLightColor = FLinearColor::White;

	// ========== 函数 ==========

	/** 获取总房间数估计 */
	UFUNCTION(BlueprintPure, Category = "计算")
	int32 GetEstimatedRoomCount() const;

	/** 验证配置是否有效 */
	UFUNCTION(BlueprintPure, Category = "验证")
	bool IsValidConfig() const;

	/** 获取难度等级描述 */
	UFUNCTION(BlueprintPure, Category = "信息")
	FString GetDifficultyDescription() const;
};
