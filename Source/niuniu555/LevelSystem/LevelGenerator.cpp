#include "LevelGenerator.h"
#include "RoomBase.h"
#include "RoomManager.h"
#include "Engine/World.h"

ULevelGenerator::ULevelGenerator()
{
	MinRoomsPerLayer = 2;
	MaxRoomsPerLayer = 4;
	BranchProbability = 0.3f;
	MinShopRooms = 1;
	EventRoomProbability = 0.3f;
	EliteRoomProbability = 0.2f;
	SkillRoomProbability = 0.15f;
}

void ULevelGenerator::GenerateMap(int32 Seed, int32 NumLayers, int32 RetryCount)
{
	// ✅ 安全检查
	if (!IsValid(this))
	{
		UE_LOG(LogTemp, Error, TEXT("[LevelGenerator] GenerateMap: this无效"));
		return;
	}

	// 限制重试次数，防止无限递归
	if (RetryCount > 5)
	{
		UE_LOG(LogTemp, Error, TEXT("[LevelGenerator] 地图生成失败次数过多(%d次)，放弃生成"), RetryCount);
		return;
	}

	// 初始化随机数
	RandomStream = Seed != 0 ? FRandomStream(Seed) : FRandomStream(FMath::Rand());

	UE_LOG(LogTemp, Log, TEXT("[LevelGenerator] 开始生成地图: %d层, 种子: %d (重试次数: %d)"), NumLayers, Seed, RetryCount);

	// 清空旧数据
	MapNodes.Empty();
	NodeConnections.Empty();
	StartNodeID = -1;
	BossNodeID = -1;

	// 执行生成步骤
	GenerateLayerStructure(NumLayers);
	
	// ✅ 检查是否仍然有效
	if (!IsValid(this)) return;
	
	CreateRoomNodes();
	
	if (!IsValid(this)) return;
	
	ConnectNodes();
	
	if (!IsValid(this)) return;
	
	AddBranchPaths();
	
	if (!IsValid(this)) return;
	
	AssignRoomTypes();
	
	if (!IsValid(this)) return;

	// 验证连通性
	if (!ValidateConnectivity())
	{
		UE_LOG(LogTemp, Warning, TEXT("[LevelGenerator] 地图验证失败，重新生成 (第%d次重试)"), RetryCount + 1);
		GenerateMap(Seed + 1, NumLayers, RetryCount + 1); // 使用不同种子重新生成
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[LevelGenerator] 地图生成完成: %d个节点"), MapNodes.Num());
}

void ULevelGenerator::ClearMapData()
{
	MapNodes.Empty();
	NodeConnections.Empty();
	StartNodeID = -1;
	BossNodeID = -1;
	
	UE_LOG(LogTemp, Log, TEXT("[LevelGenerator] 地图数据已清除"));
}

void ULevelGenerator::GenerateLayerStructure(int32 NumLayers)
{
	// 第一层：起点（1个房间）
	{
		FMapNode Node;
		Node.NodeID = MapNodes.Num();
		Node.LayerIndex = 0;
		Node.RoomType = ERoomType::Start;
		Node.MapPosition = FVector2D(0, 0);
		MapNodes.Add(Node);
		StartNodeID = Node.NodeID;
	}

	// 中间层：普通房间
	for (int32 Layer = 1; Layer < NumLayers; ++Layer)
	{
		int32 RoomCount = RandomStream.RandRange(MinRoomsPerLayer, MaxRoomsPerLayer);

		for (int32 i = 0; i < RoomCount; ++i)
		{
			FMapNode Node;
			Node.NodeID = MapNodes.Num();
			Node.LayerIndex = Layer;
			// 房间类型稍后分配
			Node.MapPosition = FVector2D(Layer, i - (RoomCount - 1) * 0.5f);
			MapNodes.Add(Node);
		}
	}

	// 最后一层：BOSS房间（1个房间）
	{
		FMapNode Node;
		Node.NodeID = MapNodes.Num();
		Node.LayerIndex = NumLayers;
		Node.RoomType = ERoomType::Boss;
		Node.MapPosition = FVector2D(NumLayers, 0);
		MapNodes.Add(Node);
		BossNodeID = Node.NodeID;
	}
}

void ULevelGenerator::CreateRoomNodes()
{
	// 节点已在GenerateLayerStructure中创建
	// 这里可以添加额外的初始化逻辑
}

void ULevelGenerator::ConnectNodes()
{
	// 连接每层到下一层的节点
	int32 NumLayers = 0;
	for (const FMapNode& Node : MapNodes)
	{
		NumLayers = FMath::Max(NumLayers, Node.LayerIndex);
	}

	for (int32 Layer = 0; Layer < NumLayers; ++Layer)
	{
		TArray<int32> CurrentLayerNodes = GetNodesInLayer(Layer);
		TArray<int32> NextLayerNodes = GetNodesInLayer(Layer + 1);

		if (CurrentLayerNodes.Num() == 0 || NextLayerNodes.Num() == 0)
		{
			continue;
		}

		// 确保每个当前层节点至少有一个连接
		for (int32 i = 0; i < CurrentLayerNodes.Num(); ++i)
		{
			int32 CurrentNodeID = CurrentLayerNodes[i];
			
			// 连接到下一层的对应节点或随机节点
			int32 NextNodeIndex = FMath::Min(i, NextLayerNodes.Num() - 1);
			int32 NextNodeID = NextLayerNodes[NextNodeIndex];

			// 添加连接
			if (!NodeConnections.Contains(CurrentNodeID))
			{
				NodeConnections.Add(CurrentNodeID, TArray<int32>());
			}
			NodeConnections[CurrentNodeID].AddUnique(NextNodeID);
		}

		// 确保每个下一层节点至少被一个当前层节点连接
		for (int32 NextNodeID : NextLayerNodes)
		{
			bool bHasConnection = false;
			for (const auto& Pair : NodeConnections)
			{
				if (Pair.Value.Contains(NextNodeID))
				{
					bHasConnection = true;
					break;
				}
			}

			if (!bHasConnection && CurrentLayerNodes.Num() > 0)
			{
				int32 RandomCurrentNode = CurrentLayerNodes[RandomStream.RandRange(0, CurrentLayerNodes.Num() - 1)];
				NodeConnections[RandomCurrentNode].AddUnique(NextNodeID);
			}
		}
	}
}

void ULevelGenerator::AddBranchPaths()
{
	// 在同层或相邻层之间添加额外连接
	for (const auto& Pair : NodeConnections)
	{
		if (RandomStream.FRand() < BranchProbability)
		{
			int32 FromNode = Pair.Key;
			int32 FromLayer = MapNodes.IsValidIndex(FromNode) ? MapNodes[FromNode].LayerIndex : -1;

			// 查找可连接的节点
			TArray<int32> PossibleTargets;
			for (const FMapNode& Node : MapNodes)
			{
				if (Node.NodeID != FromNode && Node.LayerIndex > FromLayer)
				{
					PossibleTargets.Add(Node.NodeID);
				}
			}

			if (PossibleTargets.Num() > 0)
			{
				int32 TargetNode = PossibleTargets[RandomStream.RandRange(0, PossibleTargets.Num() - 1)];
				NodeConnections[FromNode].AddUnique(TargetNode);
			}
		}
	}
}

void ULevelGenerator::AssignRoomTypes()
{
	int32 ShopRoomCount = 0;

	for (FMapNode& Node : MapNodes)
	{
		// 起点和BOSS房间已设置
		if (Node.RoomType == ERoomType::Start || Node.RoomType == ERoomType::Boss)
		{
			continue;
		}

		float Rand = RandomStream.FRand();

		// 优先保证商店房数量（使用独立随机检查）
		if (ShopRoomCount < MinShopRooms)
		{
			Node.RoomType = ERoomType::Shop;
			ShopRoomCount++;
			continue;
		}

		// 其他类型概率计算（归一化）
		float TotalProb = EventRoomProbability + SkillRoomProbability + EliteRoomProbability;
		float NormalizedRand = Rand * TotalProb; // 将随机数映射到概率总和范围内

		if (NormalizedRand < EventRoomProbability)
		{
			Node.RoomType = ERoomType::Event;
		}
		else if (NormalizedRand < EventRoomProbability + SkillRoomProbability)
		{
			Node.RoomType = ERoomType::Skill;
		}
		else
		{
			Node.RoomType = ERoomType::Combat;
		}
	}
}

bool ULevelGenerator::ValidateConnectivity()
{
	// 连通性检查：确保从起点可以到达BOSS节点（关键路径验证）
	TSet<int32> Visited;
	TArray<int32> Stack;
	Stack.Add(StartNodeID);

	while (Stack.Num() > 0)
	{
		int32 Current = Stack.Pop();
		if (Visited.Contains(Current))
		{
			continue;
		}

		Visited.Add(Current);

		if (NodeConnections.Contains(Current))
		{
			for (int32 Next : NodeConnections[Current])
			{
				if (!Visited.Contains(Next))
				{
					Stack.Add(Next);
				}
			}
		}
	}

	// ✅ 检查是否能到达BOSS节点（关键路径验证）
	if (!Visited.Contains(BossNodeID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[LevelGenerator] 验证失败：无法到达BOSS节点"));
		return false;
	}

	// 可选：检查是否访问了大部分节点（如80%），仅作警告
	float CoveragePercent = (float)Visited.Num() / MapNodes.Num();
	if (CoveragePercent < 0.8f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LevelGenerator] 验证警告：节点覆盖率仅 %.0f%%"), CoveragePercent * 100);
	}

	return true;
}

TArray<int32> ULevelGenerator::GetNodesInLayer(int32 LayerIndex) const
{
	TArray<int32> Result;
	for (const FMapNode& Node : MapNodes)
	{
		if (Node.LayerIndex == LayerIndex)
		{
			Result.Add(Node.NodeID);
		}
	}
	return Result;
}

const TArray<int32>& ULevelGenerator::GetNodeConnections(int32 NodeID) const
{
	static const TArray<int32> EmptyArray;
	const TArray<int32>* Connections = NodeConnections.Find(NodeID);
	return Connections ? *Connections : EmptyArray;
}
