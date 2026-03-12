#include "LevelSystemCheats.h"
#include "RoomManager.h"
#include "RoomBase.h"
#include "CombatRoom.h"
#include "EventRoom.h"
#include "ShopRoom.h"
#include "LevelGenerator.h"
#include "GameFlow/PlayerDataSubsystem.h"
#include "EnemySystem/EnemyBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ========== 全局控制台命令（不依赖 CheatClass） ==========

static void Level_GenerateMap(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World)) return;
	int32 Seed = (Args.Num() > 0) ? FCString::Atoi(*Args[0]) : 0;
	int32 Layers = (Args.Num() > 1) ? FCString::Atoi(*Args[1]) : 3;
	
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM))
	{
		UE_LOG(LogTemp, Error, TEXT("[Cheat] RoomManager not found"));
		return;
	}
	
	RM->GenerateRandomMap(Seed, Layers);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Generated map with seed %d, %d layers"), Seed, Layers);
}

static void Level_ShowMap(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World)) return;
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] RoomManager not found"));
		return;
	}
	
	// 输出地图信息到日志
	UE_LOG(LogTemp, Log, TEXT("========== 地图信息 =========="));
	UE_LOG(LogTemp, Log, TEXT("RoomManager 已初始化"));
	if (ARoomBase* Current = RM->GetCurrentRoom())
	{
		UE_LOG(LogTemp, Log, TEXT("当前房间: %s (ID: %d)"), *Current->GetName(), Current->RoomID);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("当前房间: None"));
	}
	UE_LOG(LogTemp, Log, TEXT("=============================="));
}

static void Level_EnterRoom(const TArray<FString>& Args, UWorld* World)
{
	if (!World || Args.Num() == 0) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Usage: Level.EnterRoom [RoomID]"));
		return;
	}
	
	int32 RoomID = FCString::Atoi(*Args[0]);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Attempting to enter room %d"), RoomID);
	
	URoomManager* RM = URoomManager::Get(World);
	if (!RM)
	{
		UE_LOG(LogTemp, Error, TEXT("[Cheat] RoomManager not found!"));
		return;
	}
	
	ARoomBase* Room = RM->GetRoomByID(RoomID);
	if (!Room)
	{
		UE_LOG(LogTemp, Error, TEXT("[Cheat] Room with ID %d not found!"), RoomID);
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Found room: %s, attempting to enter..."), *Room->GetName());
	
	if (RM->EnterRoomByID(RoomID, false))
	{
		UE_LOG(LogTemp, Log, TEXT("[Cheat] Successfully entered room %d"), RoomID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Failed to enter room %d (may be locked or invalid state)"), RoomID);
	}
}

static void Level_CompleteRoom(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World)) return;
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM)) return;
	
	ARoomBase* CurrentRoom = RM->GetCurrentRoom();
	if (IsValid(CurrentRoom))
	{
		CurrentRoom->CompleteRoom();
		UE_LOG(LogTemp, Log, TEXT("[Cheat] Completed current room"));
	}
}

static void Level_ResetRoom(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World)) return;
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM)) return;
	
	ARoomBase* CurrentRoom = RM->GetCurrentRoom();
	if (IsValid(CurrentRoom))
	{
		CurrentRoom->ResetRoom();
		UE_LOG(LogTemp, Log, TEXT("[Cheat] Reset current room"));
	}
}

static void Level_GiveGold(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World) || Args.Num() == 0) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Usage: Level.GiveGold [Amount]"));
		return;
	}
	int32 Amount = FCString::Atoi(*Args[0]);
	UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(World);
	if (!IsValid(DataSys))
	{
		UE_LOG(LogTemp, Error, TEXT("[Cheat] PlayerDataSubsystem not found"));
		return;
	}
	DataSys->AddGold(Amount);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Gave %d gold, current: %d"), Amount, DataSys->GetGold());
}

static void Level_GiveFragments(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World) || Args.Num() == 0) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Usage: Level.GiveFragments [Amount]"));
		return;
	}
	int32 Amount = FCString::Atoi(*Args[0]);
	UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(World);
	if (!IsValid(DataSys))
	{
		UE_LOG(LogTemp, Error, TEXT("[Cheat] PlayerDataSubsystem not found"));
		return;
	}
	DataSys->AddFragments(Amount);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Gave %d fragments, current: %d"), Amount, DataSys->GetFragments());
}

FAutoConsoleCommandWithWorldAndArgs GLevelGenerateMapCmd(
	TEXT("Level.GenerateMap"),
	TEXT("生成随机地图. 用法: Level.GenerateMap [Seed] [Layers]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_GenerateMap),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelShowMapCmd(
	TEXT("Level.ShowMap"),
	TEXT("显示当前地图信息"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_ShowMap),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelEnterRoomCmd(
	TEXT("Level.EnterRoom"),
	TEXT("进入指定房间. 用法: Level.EnterRoom [RoomID]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_EnterRoom),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelCompleteRoomCmd(
	TEXT("Level.CompleteRoom"),
	TEXT("完成当前房间"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_CompleteRoom),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelResetRoomCmd(
	TEXT("Level.ResetRoom"),
	TEXT("重置当前房间"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_ResetRoom),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelGiveGoldCmd(
	TEXT("Level.GiveGold"),
	TEXT("给予金币. 用法: Level.GiveGold [Amount]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_GiveGold),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelGiveFragmentsCmd(
	TEXT("Level.GiveFragments"),
	TEXT("给予神格碎片. 用法: Level.GiveFragments [Amount]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_GiveFragments),
	ECVF_Cheat
);

static void Level_KillAllEnemies(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World)) return;
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM)) return;
	
	ARoomBase* CurrentRoom = RM->GetCurrentRoom();
	if (!IsValid(CurrentRoom))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	ACombatRoom* CombatRoom = Cast<ACombatRoom>(CurrentRoom);
	if (!CombatRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not a combat room"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Cheat] ============== LevelKillAllEnemies 开始 =============="));
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Killing all enemies in room %s, 当前AliveEnemies数量: %d"), 
		*CombatRoom->GetName(), CombatRoom->AliveEnemies.Num());

	TArray<TWeakObjectPtr<AActor>> EnemiesToKill = CombatRoom->AliveEnemies;
	int32 KillCount = 0;
	for (TWeakObjectPtr<AActor>& WeakEnemy : EnemiesToKill)
	{
		if (WeakEnemy.IsValid())
		{
			AActor* Enemy = WeakEnemy.Get();
			AEnemyBase* EnemyBase = Cast<AEnemyBase>(Enemy);
			if (EnemyBase && IsValid(EnemyBase))
			{
				UE_LOG(LogTemp, Log, TEXT("[Cheat] 准备杀死敌人: %s"), *EnemyBase->GetName());
				CombatRoom->UnregisterEnemy(Enemy);
				EnemyBase->Die();
				KillCount++;
				UE_LOG(LogTemp, Log, TEXT("[Cheat] 敌人 %s 已处理"), *EnemyBase->GetName());
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Cheat] Killed %d enemies, 剩余AliveEnemyCount: %d"), 
		KillCount, CombatRoom->AliveEnemies.Num());
	UE_LOG(LogTemp, Log, TEXT("[Cheat] ============== LevelKillAllEnemies 结束 =============="));
}

static void Level_ShowPlayerData(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World)) return;
	UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(World);
	if (!IsValid(DataSys))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] PlayerDataSubsystem not found"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("========== 玩家数据 =========="));
	UE_LOG(LogTemp, Log, TEXT("金币: %d"), DataSys->GetGold());
	UE_LOG(LogTemp, Log, TEXT("神格碎片: %d"), DataSys->GetFragments());
	UE_LOG(LogTemp, Log, TEXT("=============================="));
}

static void Level_ShowEventInfo(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World)) return;
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM)) return;
	
	ARoomBase* CurrentRoom = RM->GetCurrentRoom();
	if (!IsValid(CurrentRoom))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AEventRoom* EventRoom = Cast<AEventRoom>(CurrentRoom);
	if (!EventRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not an EventRoom"));
		return;
	}

	EventRoom->LogCurrentEventInfo();
}

static void Level_ChooseOption(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World) || Args.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Usage: Level.ChooseOption [OptionIndex]"));
		return;
	}
	
	int32 OptionIndex = FCString::Atoi(*Args[0]);
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM)) return;
	
	ARoomBase* CurrentRoom = RM->GetCurrentRoom();
	if (!IsValid(CurrentRoom))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AEventRoom* EventRoom = Cast<AEventRoom>(CurrentRoom);
	if (!EventRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not an EventRoom"));
		return;
	}

	EventRoom->ChooseOption(OptionIndex);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Chose event option %d"), OptionIndex);
}

static void Level_SkipEvent(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World)) return;
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM)) return;
	
	ARoomBase* CurrentRoom = RM->GetCurrentRoom();
	if (!IsValid(CurrentRoom))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AEventRoom* EventRoom = Cast<AEventRoom>(CurrentRoom);
	if (!EventRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not an EventRoom"));
		return;
	}

	EventRoom->SkipEvent();
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Skipped event"));
}

static void Level_ShowShopInfo(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World)) return;
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM)) return;
	
	ARoomBase* CurrentRoom = RM->GetCurrentRoom();
	if (!IsValid(CurrentRoom))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AShopRoom* ShopRoom = Cast<AShopRoom>(CurrentRoom);
	if (!ShopRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not a ShopRoom"));
		return;
	}

	ShopRoom->LogShopInfo();
}

static void Level_BuyItem(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World) || Args.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Usage: Level.BuyItem [ItemIndex]"));
		return;
	}
	
	int32 ItemIndex = FCString::Atoi(*Args[0]);
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM)) return;
	
	ARoomBase* CurrentRoom = RM->GetCurrentRoom();
	if (!IsValid(CurrentRoom))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AShopRoom* ShopRoom = Cast<AShopRoom>(CurrentRoom);
	if (!ShopRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not a ShopRoom"));
		return;
	}

	if (ShopRoom->PurchaseItem(ItemIndex))
	{
		UE_LOG(LogTemp, Log, TEXT("[Cheat] Purchased item at index %d"), ItemIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Failed to purchase item at index %d"), ItemIndex);
	}
}

static void Level_RefreshShop(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World)) return;
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM)) return;
	
	ARoomBase* CurrentRoom = RM->GetCurrentRoom();
	if (!IsValid(CurrentRoom))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AShopRoom* ShopRoom = Cast<AShopRoom>(CurrentRoom);
	if (!ShopRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not a ShopRoom"));
		return;
	}

	ShopRoom->RefreshItems();
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Refreshed shop"));
}

static void Level_FinishShopping(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World)) return;
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM)) return;
	
	ARoomBase* CurrentRoom = RM->GetCurrentRoom();
	if (!IsValid(CurrentRoom))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AShopRoom* ShopRoom = Cast<AShopRoom>(CurrentRoom);
	if (!ShopRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not a ShopRoom"));
		return;
	}

	ShopRoom->FinishShopping();
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Finished shopping"));
}

static void Level_UnlockAllRooms(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World)) return;
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM)) return;

	for (ARoomBase* Room : RM->GetAllRooms())
	{
		if (Room && Room->RoomState == ERoomState::Locked)
		{
			Room->RoomState = ERoomState::NotEntered;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Cheat] Unlocked all rooms"));
}

static void Level_ResetChapter(const TArray<FString>& Args, UWorld* World)
{
	if (!IsValid(World)) return;
	URoomManager* RM = URoomManager::Get(World);
	if (!IsValid(RM))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] RoomManager not found"));
		return;
	}

	for (ARoomBase* Room : RM->GetAllRooms())
	{
		if (Room)
		{
			Room->ResetRoom();
		}
	}

	RM->LeaveCurrentRoom();
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Reset chapter"));
}

FAutoConsoleCommandWithWorldAndArgs GLevelKillAllEnemiesCmd(
	TEXT("Level.KillAllEnemies"),
	TEXT("击杀所有敌人"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_KillAllEnemies),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelShowPlayerDataCmd(
	TEXT("Level.ShowPlayerData"),
	TEXT("显示玩家数据"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_ShowPlayerData),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelShowEventInfoCmd(
	TEXT("Level.ShowEventInfo"),
	TEXT("显示当前事件信息"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_ShowEventInfo),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelChooseOptionCmd(
	TEXT("Level.ChooseOption"),
	TEXT("选择事件选项. 用法: Level.ChooseOption [OptionIndex]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_ChooseOption),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelSkipEventCmd(
	TEXT("Level.SkipEvent"),
	TEXT("跳过当前事件"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_SkipEvent),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelShowShopInfoCmd(
	TEXT("Level.ShowShopInfo"),
	TEXT("显示商店商品列表"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_ShowShopInfo),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelBuyItemCmd(
	TEXT("Level.BuyItem"),
	TEXT("购买指定索引商品. 用法: Level.BuyItem [ItemIndex]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_BuyItem),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelRefreshShopCmd(
	TEXT("Level.RefreshShop"),
	TEXT("刷新商店商品"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_RefreshShop),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelFinishShoppingCmd(
	TEXT("Level.FinishShopping"),
	TEXT("完成购物离开商店"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_FinishShopping),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelUnlockAllRoomsCmd(
	TEXT("Level.UnlockAllRooms"),
	TEXT("开启所有房间"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_UnlockAllRooms),
	ECVF_Cheat
);

FAutoConsoleCommandWithWorldAndArgs GLevelResetChapterCmd(
	TEXT("Level.ResetChapter"),
	TEXT("重置章节"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Level_ResetChapter),
	ECVF_Cheat
);

URoomManager* ULevelSystemCheats::GetRoomManager() const
{
	return URoomManager::Get(this);
}

ARoomBase* ULevelSystemCheats::GetCurrentRoom() const
{
	URoomManager* RoomManager = GetRoomManager();
	if (!RoomManager)
	{
		return nullptr;
	}
	return RoomManager->GetCurrentRoom();
}

APawn* ULevelSystemCheats::GetPlayerPawn() const
{
	return UGameplayStatics::GetPlayerPawn(this, 0);
}

void ULevelSystemCheats::LevelGenerateMap(int32 Seed, int32 Layers)
{
	URoomManager* RoomManager = GetRoomManager();
	if (!RoomManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] RoomManager not found"));
		return;
	}

	RoomManager->GenerateRandomMap(Seed, Layers);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Generated map with seed %d, %d layers"), Seed, Layers);
}

void ULevelSystemCheats::LevelEnterRoom(int32 RoomID)
{
	URoomManager* RoomManager = GetRoomManager();
	if (!RoomManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] RoomManager not found"));
		return;
	}

	if (RoomManager->EnterRoomByID(RoomID, false))
	{
		UE_LOG(LogTemp, Log, TEXT("[Cheat] Entered room %d"), RoomID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Failed to enter room %d"), RoomID);
	}
}

void ULevelSystemCheats::LevelShowMap()
{
	URoomManager* RoomManager = GetRoomManager();
	if (!RoomManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] RoomManager not found"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Cheat] === Map Info ==="));
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Total Rooms: %d"), RoomManager->GetAllRooms().Num());
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Current Room: %s"), 
		RoomManager->GetCurrentRoom() ? *RoomManager->GetCurrentRoom()->GetName() : TEXT("None"));
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Map Generated: %s"), RoomManager->HasGeneratedMap() ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Current Layer: %d / %d"), RoomManager->GetCurrentLayer(), RoomManager->GetTotalLayers());

	// 列出所有房间
	for (ARoomBase* Room : RoomManager->GetAllRooms())
	{
		if (Room)
		{
			UE_LOG(LogTemp, Log, TEXT("[Cheat] Room %d: %s (Type: %s, State: %s)"),
				Room->RoomID,
				*Room->RoomName,
				*ARoomBase::GetRoomTypeName(Room->RoomType),
				*ARoomBase::GetRoomStateName(Room->RoomState));
		}
	}
}

void ULevelSystemCheats::LevelResetRoom()
{
	ARoomBase* CurrentRoom = GetCurrentRoom();
	if (!CurrentRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	CurrentRoom->ResetRoom();
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Reset room %s"), *CurrentRoom->GetName());
}

void ULevelSystemCheats::LevelCompleteRoom()
{
	ARoomBase* CurrentRoom = GetCurrentRoom();
	if (!CurrentRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	CurrentRoom->CompleteRoom();
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Completed room %s"), *CurrentRoom->GetName());
}

void ULevelSystemCheats::LevelSkipRoom()
{
	ARoomBase* CurrentRoom = GetCurrentRoom();
	if (!CurrentRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	CurrentRoom->SkipRoom();
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Skipped room %s"), *CurrentRoom->GetName());
}

void ULevelSystemCheats::LevelSpawnEnemy(const FString& EnemyClassName, int32 Count)
{
	// TODO: 实现敌人生成
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Spawning %d enemies of type %s"), Count, *EnemyClassName);
}

void ULevelSystemCheats::LevelKillAllEnemies()
{
	UE_LOG(LogTemp, Log, TEXT("[Cheat] ============== LevelKillAllEnemies 开始 =============="));
	
	ACombatRoom* CombatRoom = Cast<ACombatRoom>(GetCurrentRoom());
	if (!CombatRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not a combat room"));
		return;
	}

	if (!IsValid(CombatRoom))
	{
		UE_LOG(LogTemp, Error, TEXT("[Cheat] CombatRoom is invalid"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Cheat] Killing all enemies in room %s, 当前AliveEnemies数量: %d"), 
		*CombatRoom->GetName(), CombatRoom->AliveEnemies.Num());

	// 复制一份敌人列表，避免在遍历时修改原数组
	TArray<TWeakObjectPtr<AActor>> EnemiesToKill = CombatRoom->AliveEnemies;
	
	int32 KillCount = 0;
	for (TWeakObjectPtr<AActor>& WeakEnemy : EnemiesToKill)
	{
		if (WeakEnemy.IsValid())
		{
			AActor* Enemy = WeakEnemy.Get();
			
			// 检查是否是敌人基类
			AEnemyBase* EnemyBase = Cast<AEnemyBase>(Enemy);
			if (EnemyBase && IsValid(EnemyBase))
			{
				UE_LOG(LogTemp, Log, TEXT("[Cheat] 准备杀死敌人: %s"), *EnemyBase->GetName());
				
				// 先调用UnregisterEnemy直接从列表中移除
				CombatRoom->UnregisterEnemy(Enemy);
				
				// 再杀死敌人
				EnemyBase->Die();
				KillCount++;
				
				UE_LOG(LogTemp, Log, TEXT("[Cheat] 敌人 %s 已处理"), *EnemyBase->GetName());
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Cheat] Killed %d enemies, 剩余AliveEnemyCount: %d"), 
		KillCount, CombatRoom->AliveEnemyCount);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] ============== LevelKillAllEnemies 结束 =============="));
}

void ULevelSystemCheats::LevelGiveGold(int32 Amount)
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		DataSys->AddGold(Amount);
		UE_LOG(LogTemp, Log, TEXT("[Cheat] Gave %d gold, current: %d"), Amount, DataSys->GetGold());
	}
}

void ULevelSystemCheats::LevelGiveFragments(int32 Amount)
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		DataSys->AddFragments(Amount);
		UE_LOG(LogTemp, Log, TEXT("[Cheat] Gave %d fragments, current: %d"), Amount, DataSys->GetFragments());
	}
}

void ULevelSystemCheats::LevelUnlockAllRooms()
{
	URoomManager* RoomManager = GetRoomManager();
	if (!RoomManager)
	{
		return;
	}

	for (ARoomBase* Room : RoomManager->GetAllRooms())
	{
		if (Room && Room->RoomState == ERoomState::Locked)
		{
			Room->RoomState = ERoomState::NotEntered;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Cheat] Unlocked all rooms"));
}

void ULevelSystemCheats::LevelResetChapter()
{
	URoomManager* RoomManager = GetRoomManager();
	if (!RoomManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] RoomManager not found"));
		return;
	}

	// 重置所有房间
	for (ARoomBase* Room : RoomManager->GetAllRooms())
	{
		if (Room)
		{
			Room->ResetRoom();
		}
	}

	// 清除当前房间
	RoomManager->LeaveCurrentRoom();

	UE_LOG(LogTemp, Log, TEXT("[Cheat] Reset chapter"));
}

// ========== 蓝图版本函数实现 ==========

void ULevelSystemCheats::BP_GenerateMap(int32 Seed, int32 Layers)
{
	LevelGenerateMap(Seed, Layers);
}

void ULevelSystemCheats::BP_EnterRoom(int32 RoomID)
{
	LevelEnterRoom(RoomID);
}

void ULevelSystemCheats::BP_ShowMap()
{
	LevelShowMap();
}

void ULevelSystemCheats::BP_GiveGold(int32 Amount)
{
	LevelGiveGold(Amount);
}

void ULevelSystemCheats::BP_GiveFragments(int32 Amount)
{
	LevelGiveFragments(Amount);
}

// ========== 玩家数据命令 ==========

void ULevelSystemCheats::LevelShowPlayerData()
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		UE_LOG(LogTemp, Log, TEXT("========== 玩家数据 =========="));
		UE_LOG(LogTemp, Log, TEXT("金币: %d"), DataSys->GetGold());
		UE_LOG(LogTemp, Log, TEXT("神格碎片: %d"), DataSys->GetFragments());
		UE_LOG(LogTemp, Log, TEXT("=============================="));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] PlayerDataSubsystem not found"));
	}
}

// ========== EventRoom 交互命令 ==========

void ULevelSystemCheats::LevelChooseOption(int32 OptionIndex)
{
	ARoomBase* CurrentRoom = GetCurrentRoom();
	if (!CurrentRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AEventRoom* EventRoom = Cast<AEventRoom>(CurrentRoom);
	if (!EventRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not an EventRoom"));
		return;
	}

	EventRoom->ChooseOption(OptionIndex);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Chose event option %d"), OptionIndex);
}

void ULevelSystemCheats::LevelSkipEvent()
{
	ARoomBase* CurrentRoom = GetCurrentRoom();
	if (!CurrentRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AEventRoom* EventRoom = Cast<AEventRoom>(CurrentRoom);
	if (!EventRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not an EventRoom"));
		return;
	}

	EventRoom->SkipEvent();
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Skipped event"));
}

void ULevelSystemCheats::LevelShowEventInfo()
{
	ARoomBase* CurrentRoom = GetCurrentRoom();
	if (!CurrentRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AEventRoom* EventRoom = Cast<AEventRoom>(CurrentRoom);
	if (!EventRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not an EventRoom"));
		return;
	}

	EventRoom->LogCurrentEventInfo();
}

// ========== ShopRoom 交互命令 ==========

void ULevelSystemCheats::LevelBuyItem(int32 ItemIndex)
{
	ARoomBase* CurrentRoom = GetCurrentRoom();
	if (!CurrentRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AShopRoom* ShopRoom = Cast<AShopRoom>(CurrentRoom);
	if (!ShopRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not a ShopRoom"));
		return;
	}

	if (ShopRoom->PurchaseItem(ItemIndex))
	{
		UE_LOG(LogTemp, Log, TEXT("[Cheat] Purchased item at index %d"), ItemIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Failed to purchase item at index %d"), ItemIndex);
	}
}

void ULevelSystemCheats::LevelRefreshShop()
{
	ARoomBase* CurrentRoom = GetCurrentRoom();
	if (!CurrentRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AShopRoom* ShopRoom = Cast<AShopRoom>(CurrentRoom);
	if (!ShopRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not a ShopRoom"));
		return;
	}

	ShopRoom->RefreshItems();
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Refreshed shop items"));
}

void ULevelSystemCheats::LevelFinishShopping()
{
	ARoomBase* CurrentRoom = GetCurrentRoom();
	if (!CurrentRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AShopRoom* ShopRoom = Cast<AShopRoom>(CurrentRoom);
	if (!ShopRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not a ShopRoom"));
		return;
	}

	ShopRoom->FinishShopping();
	UE_LOG(LogTemp, Log, TEXT("[Cheat] Finished shopping"));
}

void ULevelSystemCheats::LevelShowShopInfo()
{
	ARoomBase* CurrentRoom = GetCurrentRoom();
	if (!CurrentRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] No current room"));
		return;
	}

	AShopRoom* ShopRoom = Cast<AShopRoom>(CurrentRoom);
	if (!ShopRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] Current room is not a ShopRoom"));
		return;
	}

	ShopRoom->LogShopInfo();
}
