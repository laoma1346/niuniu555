#include "RoomManager.h"
#include "RoomBase.h"
#include "LevelGenerator.h"
#include "ChapterConfigDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"

URoomManager* URoomManager::Get(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<URoomManager>();
}

void URoomManager::Initialize(FSubsystemCollectionBase& Collection)
{
	// 防止重复初始化
	if (bInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RoomManager] 已经初始化，跳过重复初始化"));
		return;
	}

	Super::Initialize(Collection);

	// 检查GameInstance（必须存在）
	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("[RoomManager] 初始化失败：GameInstance 为 nullptr"));
		return;
	}

	bInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("[RoomManager] 初始化成功"));

	// 订阅World清理事件（重要：GameInstanceSubsystem需要监听World变化）
	WorldCleanupDelegateHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &URoomManager::OnWorldCleanup);

	// 延迟初始化，等待World准备就绪
	// 使用成员函数绑定而非Lambda，更安全
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		FTimerDelegate InitDelegate;
		InitDelegate.BindUObject(this, &URoomManager::InitializeInternal);
		World->GetTimerManager().SetTimer(InitDelayTimerHandle, InitDelegate, 0.1f, false);
	}
}

void URoomManager::InitializeInternal()
{
	if (!IsValid(this) || !bInitialized)
	{
		return;
	}

	// 创建LevelGenerator
	LevelGenerator = NewObject<ULevelGenerator>(this);
	UE_LOG(LogTemp, Log, TEXT("[RoomManager] 地图生成器创建成功"));
}

void URoomManager::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	UE_LOG(LogTemp, Log, TEXT("[RoomManager] World清理事件 - SessionEnded: %s, CleanupResources: %s"),
		bSessionEnded ? TEXT("true") : TEXT("false"),
		bCleanupResources ? TEXT("true") : TEXT("false"));

	// 清理当前World相关的数据
	if (IsValid(World))
	{
		// 清除该World的所有定时器
		World->GetTimerManager().ClearTimer(InitDelayTimerHandle);
	}

	// 清空房间引用（但不销毁Actor，让World自己处理）
	RegisteredRooms.Empty();
	CurrentRoom = nullptr;
}

void URoomManager::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("[RoomManager] 反初始化"));

	// 取消订阅World事件
	if (WorldCleanupDelegateHandle.IsValid())
	{
		FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupDelegateHandle);
		WorldCleanupDelegateHandle.Reset();
	}

	// 清理定时器
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InitDelayTimerHandle);
	}

	// 清空数据
	RegisteredRooms.Empty();
	CurrentRoom = nullptr;
	LevelGenerator = nullptr;
	bInitialized = false;

	Super::Deinitialize();
}

void URoomManager::RegisterRoom(ARoomBase* Room)
{
	if (!IsValid(Room))
	{
		return;
	}

	if (!RegisteredRooms.Contains(Room))
	{
		RegisteredRooms.Add(Room);
		Room->RoomManagerPtr = this;

		UE_LOG(LogTemp, Log, TEXT("[RoomManager] 注册房间: %s (ID: %d)"), 
			*Room->GetName(), Room->RoomID);
	}
}

void URoomManager::UnregisterRoom(ARoomBase* Room)
{
	if (!IsValid(Room))
	{
		return;
	}

	RegisteredRooms.Remove(Room);
	Room->RoomManagerPtr = nullptr;

	UE_LOG(LogTemp, Log, TEXT("[RoomManager] 注销房间: %s"), *Room->GetName());
}

ARoomBase* URoomManager::GetRoomByID(int32 RoomID) const
{
	// ✅ 安全检查
	if (!IsValid(this))
	{
		return nullptr;
	}

	// 添加边界检查
	if (RoomID < 0)
	{
		return nullptr;
	}

	for (ARoomBase* Room : RegisteredRooms)
	{
		// 使用 IsValid 检查指针是否有效（未被 GC 回收）
		if (IsValid(Room) && Room->RoomID == RoomID)
		{
			return Room;
		}
	}
	return nullptr;
}

bool URoomManager::EnterRoom(ARoomBase* Room, bool bWithLoading)
{
	if (!Room)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RoomManager] 进入房间失败：房间为空"));
		return false;
	}

	if (!Room->CanEnter())
	{
		UE_LOG(LogTemp, Warning, TEXT("[RoomManager] 进入房间失败：房间 %s 无法进入"), *Room->GetName());
		return false;
	}

	ARoomBase* PreviousRoom = CurrentRoom;
	CurrentRoom = Room;

	// 执行房间切换逻辑
	if (bWithLoading && !Room->LevelName.IsNone())
	{
		// 异步加载关卡
		DoRoomTransition(PreviousRoom, Room);
	}
	else
	{
		// 直接切换，不显示加载
		// TODO: 处理PreviousRoom的离开逻辑

		// ✅ 安全检查PreviousRoom
		if (PreviousRoom && !IsValid(PreviousRoom))
		{
			PreviousRoom = nullptr;  // 如果无效，设为nullptr避免传递无效指针
		}

		Room->EnterRoom();
		
		// ✅ 广播前确保Room有效
		if (IsValid(Room))
		{
			OnRoomEntered.Broadcast(PreviousRoom, Room);
		}
	}

	return true;
}

bool URoomManager::EnterRoomByID(int32 RoomID, bool bWithLoading)
{
	// ✅ 安全检查
	if (!IsValid(this))
	{
		UE_LOG(LogTemp, Error, TEXT("[RoomManager] EnterRoomByID: this无效"));
		return false;
	}

	ARoomBase* Room = GetRoomByID(RoomID);
	if (!IsValid(Room))
	{
		UE_LOG(LogTemp, Warning, TEXT("[RoomManager] 找不到房间ID: %d"), RoomID);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[RoomManager] EnterRoomByID: 找到房间 %s，准备进入"), *Room->GetName());
	return EnterRoom(Room, bWithLoading);
}

bool URoomManager::LeaveCurrentRoom()
{
	if (!CurrentRoom)
	{
		return false;
	}

	OnRoomLeft.Broadcast(CurrentRoom);
	CurrentRoom = nullptr;

	return true;
}

void URoomManager::GenerateRandomMap(int32 Seed, int32 NumLayers)
{
	// ✅ 安全检查
	if (!IsValid(this))
	{
		UE_LOG(LogTemp, Error, TEXT("[RoomManager] GenerateRandomMap: this无效"));
		return;
	}

	if (!LevelGenerator)
	{
		UE_LOG(LogTemp, Error, TEXT("[RoomManager] 地图生成器未初始化"));
		return;
	}

	// 注意：在测试场景中，不要调用 ClearAllRooms() 来销毁手动放置的房间
	// 只清除地图数据，保留场景中的房间Actor
	CurrentRoom = nullptr;
	bMapGenerated = false;
	
	// 清除旧地图数据（但不销毁房间Actor）
	LevelGenerator->ClearMapData();

	// 生成新地图
	LevelGenerator->GenerateMap(Seed, NumLayers);

	// ✅ 检查是否仍然有效（生成过程中可能世界被切换）
	if (!IsValid(this))
	{
		UE_LOG(LogTemp, Warning, TEXT("[RoomManager] GenerateRandomMap: 生成后this无效，中止广播"));
		return;
	}

	bMapGenerated = true;
	CurrentLayer = 0;
	TotalLayers = NumLayers;

	// ✅ 广播前再次检查
	if (IsValid(this))
	{
		OnMapGenerated.Broadcast();
	}

	UE_LOG(LogTemp, Log, TEXT("[RoomManager] 地图生成完成: %d层（保留场景中的房间）"), NumLayers);
	UE_LOG(LogTemp, Warning, TEXT("[RoomManager] 提示：如需进入房间，请直接走到房间触发区域，或使用场景中的房间ID"));
}

void URoomManager::GenerateMapFromConfig(UChapterConfigDataAsset* Config, int32 Seed)
{
	if (!Config)
	{
		UE_LOG(LogTemp, Error, TEXT("[RoomManager] 章节配置为空，无法生成地图"));
		return;
	}

	if (!Config->IsValidConfig())
	{
		UE_LOG(LogTemp, Error, TEXT("[RoomManager] 章节配置无效"));
		return;
	}

	CurrentChapterConfig = Config;

	// 应用配置到LevelGenerator
	if (LevelGenerator)
	{
		LevelGenerator->MinRoomsPerLayer = Config->MinRoomsPerLayer;
		LevelGenerator->MaxRoomsPerLayer = Config->MaxRoomsPerLayer;
		LevelGenerator->BranchProbability = Config->BranchProbability;
		LevelGenerator->MinShopRooms = Config->MinShopRooms;
		LevelGenerator->EventRoomProbability = Config->EventRoomProbability;
		LevelGenerator->SkillRoomProbability = Config->SkillRoomProbability;
		LevelGenerator->EliteRoomProbability = Config->EliteRoomProbability;
	}

	// 使用配置的层数生成地图
	GenerateRandomMap(Seed, Config->NumLayers);

	UE_LOG(LogTemp, Log, TEXT("[RoomManager] 使用章节配置生成地图: %s"), *Config->ChapterName);
}

void URoomManager::ClearAllRooms()
{
	// 复制数组，因为销毁Actor会触发EndPlay，进而调用UnregisterRoom修改原数组
	TArray<ARoomBase*> RoomsToDestroy = RegisteredRooms;
	
	for (ARoomBase* Room : RoomsToDestroy)
	{
		if (IsValid(Room))
		{
			Room->Destroy();
		}
	}

	RegisteredRooms.Empty();
	CurrentRoom = nullptr;
	bMapGenerated = false;

	UE_LOG(LogTemp, Log, TEXT("[RoomManager] 清除所有房间"));
}

void URoomManager::DoRoomTransition(ARoomBase* FromRoom, ARoomBase* ToRoom)
{
	if (!ToRoom)
	{
		return;
	}

	// 注意：OpenLevel会销毁当前世界，后续代码不会执行
	// 正确的做法是使用流式关卡(Level Streaming)或在世界之间传递数据
	if (!ToRoom->LevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[RoomManager] 检测到LevelName设置，但OpenLevel会销毁当前世界。"));
		UE_LOG(LogTemp, Warning, TEXT("[RoomManager] 建议使用流式关卡或将所有房间放在同一关卡中。"));
		
		// 暂时不执行OpenLevel，改为在当前关卡中切换
		// UGameplayStatics::OpenLevel(this, ToRoom->LevelName);
		// return; // OpenLevel后世界销毁，不能执行后续代码
	}

	// 在当前关卡中切换房间
	ToRoom->EnterRoom();
	OnRoomEntered.Broadcast(FromRoom, ToRoom);
}
