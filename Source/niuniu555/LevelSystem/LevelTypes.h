#pragma once

#include "CoreMinimal.h"
#include "LevelTypes.generated.h"

// 前置声明
class ARoomBase;
class AEnemyBase;

// 前置声明事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomStateChanged, ARoomBase*, Room);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoomEntered, ARoomBase*, FromRoom, ARoomBase*, ToRoom);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomLeft, ARoomBase*, Room);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMapGenerated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStarted, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveCompleted, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyKilled, AActor*, Enemy, int32, RemainingCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveProgressUpdated, float, ProgressPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventOptionSelected, int32, OptionIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopItemPurchased, int32, ItemIndex);

/**
 * 房间类型枚举
 */
UENUM(BlueprintType)
enum class ERoomType : uint8
{
	None UMETA(DisplayName = "无"),
	Start UMETA(DisplayName = "起点"),
	Combat UMETA(DisplayName = "战斗房"),
	Event UMETA(DisplayName = "事件房"),
	Shop UMETA(DisplayName = "商店房"),
	Skill UMETA(DisplayName = "技能房"),
	Boss UMETA(DisplayName = "BOSS房")
};

/**
 * 房间状态枚举
 */
UENUM(BlueprintType)
enum class ERoomState : uint8
{
	NotEntered UMETA(DisplayName = "未进入"),
	InProgress UMETA(DisplayName = "进行中"),
	Completed UMETA(DisplayName = "已完成"),
	Skipped UMETA(DisplayName = "已跳过"),
	Locked UMETA(DisplayName = "已锁定")
};

/**
 * 战斗房间类型
 */
UENUM(BlueprintType)
enum class ECombatRoomType : uint8
{
	Normal UMETA(DisplayName = "普通房间"),
	Elite UMETA(DisplayName = "精英房间"),
	Boss UMETA(DisplayName = "BOSS房间")
};

/**
 * 房间目标类型
 */
UENUM(BlueprintType)
enum class ECombatObjectiveType : uint8
{
	KillAll UMETA(DisplayName = "击杀所有敌人"),
	KillCount UMETA(DisplayName = "击杀指定数量"),
	SurviveTime UMETA(DisplayName = "存活指定时间"),
	DefendTarget UMETA(DisplayName = "保护目标"),
	KillBoss UMETA(DisplayName = "击杀BOSS")
};

/**
 * 事件类型
 */
UENUM(BlueprintType)
enum class EEventType : uint8
{
	None UMETA(DisplayName = "无"),
	Blessing UMETA(DisplayName = "神谕祝福"),
	Curse UMETA(DisplayName = "诅咒装备"),
	Treasure UMETA(DisplayName = "宝藏"),
	Shop UMETA(DisplayName = "神秘商人"),
	Gamble UMETA(DisplayName = "赌博"),
	Random UMETA(DisplayName = "随机事件")
};

/**
 * 商店商品类型
 */
UENUM(BlueprintType)
enum class EShopItemType : uint8
{
	Equipment UMETA(DisplayName = "装备"),
	Consumable UMETA(DisplayName = "消耗品"),
	Skill UMETA(DisplayName = "技能"),
	Service UMETA(DisplayName = "服务"),
	Random UMETA(DisplayName = "随机商品")
};

/**
 * 敌人生成组（供CombatRoom使用）
 */
USTRUCT(BlueprintType)
struct FEnemySpawnGroup
{
	GENERATED_BODY()

	/** 敌人类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEnemyBase> EnemyClass;

	/** 生成数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;

	/** 生成延迟（相对波次开始） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnDelay = 0.0f;

	/** 使用哪个生成器（-1表示随机） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnerIndex = -1;
};

/**
 * 波次配置结构体（供CombatRoom使用）
 */
USTRUCT(BlueprintType)
struct FCombatWaveConfig
{
	GENERATED_BODY()

	/** 波次名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WaveName;

	/** 延迟开始时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DelayBeforeStart = 3.0f;

	/** 敌人生成组 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEnemySpawnGroup> SpawnGroups;

	/** 是否等待所有敌人死亡才开始下一波 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWaitForClear = true;
};

/**
 * BOSS配置结构体（供CombatRoom使用）
 */
USTRUCT(BlueprintType)
struct FBossConfig
{
	GENERATED_BODY()

	/** BOSS类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEnemyBase> BossClass;

	/** BOSS名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BossName;

	/** 开场动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* IntroAnimation;

	/** BOSS战音乐 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* BossMusic;

	/** 阶段转换血量百分比 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> PhaseHealthThresholds;
};

/**
 * 事件选项结构体
 */
USTRUCT(BlueprintType)
struct FEventOption
{
	GENERATED_BODY()

	/** 选项名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OptionName;

	/** 选项描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OptionDescription;

	/** 选项图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* OptionIcon;

	/** 是否需要消耗金币 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCostsGold = false;

	/** 金币消耗 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bCostsGold"))
	int32 GoldCost = 0;

	/** 是否需要消耗神格碎片 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCostsFragments = false;

	/** 神格碎片消耗 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bCostsFragments"))
	int32 FragmentCost = 0;
};

/**
 * 事件配置结构体
 */
USTRUCT(BlueprintType)
struct FEventConfig
{
	GENERATED_BODY()

	/** 事件类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEventType EventType = EEventType::None;

	/** 事件标题 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventTitle;

	/** 事件描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = true))
	FString EventDescription;

	/** 事件图片 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* EventImage;

	/** 选项列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEventOption> Options;

	/** 是否允许跳过 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanSkip = true;
};

/**
 * 商店商品结构体
 */
USTRUCT(BlueprintType)
struct FShopItem
{
	GENERATED_BODY()

	/** 商品名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	/** 商品描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemDescription;

	/** 商品图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ItemIcon;

	/** 商品类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShopItemType ItemType = EShopItemType::Equipment;

	/** 商品类（如果是装备/物品） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ItemClass;

	/** 金币价格 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldPrice = 100;

	/** 神格碎片价格 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FragmentPrice = 0;

	/** 是否已售出 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bSold = false;

	/** 限量数量（0表示无限） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StockCount = 0;
};

/**
 * 地图节点结构体（供LevelGenerator使用）
 */
USTRUCT(BlueprintType)
struct FMapNode
{
	GENERATED_BODY()

	/** 节点ID */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 NodeID = -1;

	/** 所在层级 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 LayerIndex = 0;

	/** 房间类型 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ERoomType RoomType = ERoomType::None;

	/** 房间类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ARoomBase> RoomClass;

	/** 在地图上的位置 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D MapPosition = FVector2D::ZeroVector;

	/** 是否可以跳过 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bCanSkip = false;

	/** 是否已完成 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bCompleted = false;
};

/**
 * 地图生成结果结构体（供LevelGenerator使用）
 */
USTRUCT(BlueprintType)
struct FMapGenerationResult
{
	GENERATED_BODY()

	/** 是否成功 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bSuccess = false;

	/** 生成的节点 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FMapNode> Nodes;

	/** 起始节点ID */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 StartNodeID = -1;

	/** BOSS节点ID */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 BossNodeID = -1;
};
