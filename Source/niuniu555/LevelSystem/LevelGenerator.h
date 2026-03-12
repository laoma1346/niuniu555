#pragma once

#include "CoreMinimal.h"
#include "LevelTypes.h"
#include "UObject/NoExportTypes.h"
#include "LevelGenerator.generated.h"

class ARoomBase;

/**
 * 地图生成器
 * 负责生成随机地图结构（节点和连接）
 */
UCLASS()
class NIUNIU555_API ULevelGenerator : public UObject
{
	GENERATED_BODY()

public:
	ULevelGenerator();

	/**
	 * 生成随机地图
	 * @param Seed 随机种子
	 * @param NumLayers 地图层数
	 * @param RetryCount 内部使用，递归重试次数
	 */
	UFUNCTION(BlueprintCallable, Category = "地图生成")
	void GenerateMap(int32 Seed, int32 NumLayers, int32 RetryCount = 0);

	/**
	 * 清除地图数据（不销毁场景中的房间Actor）
	 */
	UFUNCTION(BlueprintCallable, Category = "地图生成")
	void ClearMapData();

	/**
	 * 获取生成的房间列表
	 */
	UFUNCTION(BlueprintPure, Category = "地图生成")
	const TArray<FMapNode>& GetGeneratedNodes() const { return MapNodes; }

	/**
	 * 获取起始节点
	 */
	UFUNCTION(BlueprintPure, Category = "地图生成")
	int32 GetStartNodeID() const { return StartNodeID; }

	/**
	 * 获取BOSS节点
	 */
	UFUNCTION(BlueprintPure, Category = "地图生成")
	int32 GetBossNodeID() const { return BossNodeID; }

	/**
	 * 获取节点的连接
	 */
	UFUNCTION(BlueprintPure, Category = "地图生成")
	const TArray<int32>& GetNodeConnections(int32 NodeID) const;

public:
	// ========== 生成配置 ==========

	/** 每层最少房间数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
	int32 MinRoomsPerLayer = 2;

	/** 每层最多房间数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
	int32 MaxRoomsPerLayer = 4;

	/** 分支概率（额外连接的概率） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
	float BranchProbability = 0.3f;

	/** 商店房每层最少数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
	int32 MinShopRooms = 1;

	/** 事件房出现概率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
	float EventRoomProbability = 0.3f;

	/** 精英房出现概率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
	float EliteRoomProbability = 0.2f;

	/** 技能房出现概率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "生成配置")
	float SkillRoomProbability = 0.15f;

protected:
	/** 地图节点列表 */
	UPROPERTY()
	TArray<FMapNode> MapNodes;

	/** 节点连接关系 */
	TMap<int32, TArray<int32>> NodeConnections;

	/** 起始节点ID */
	int32 StartNodeID = -1;

	/** BOSS节点ID */
	int32 BossNodeID = -1;

	/** 随机数生成器 */
	FRandomStream RandomStream;

	// ========== 生成步骤 ==========

	/** 生成层级结构 */
	void GenerateLayerStructure(int32 NumLayers);

	/** 创建房间节点 */
	void CreateRoomNodes();

	/** 连接节点 */
	void ConnectNodes();

	/** 添加分支路径 */
	void AddBranchPaths();

	/** 分配房间类型 */
	void AssignRoomTypes();

	/** 验证地图连通性 */
	bool ValidateConnectivity();

	/** 获取指定层的节点ID */
	TArray<int32> GetNodesInLayer(int32 LayerIndex) const;

	/** 获取指定层的房间数量 */
	int32 GetRoomCountInLayer(int32 LayerIndex) const;
};
