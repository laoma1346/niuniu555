#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "LevelSystemCheats.generated.h"

class URoomManager;
class ULevelGenerator;

/**
 * 关卡系统作弊管理器
 * 提供调试命令
 * 
 * 使用方法:
 * 1. 确保PlayerController的CheatClass设置为ULevelSystemCheats
 * 2. 在游戏中按~键打开控制台
 * 3. 输入命令如: LevelGenerateMap 12345 3
 */
UCLASS()
class NIUNIU555_API ULevelSystemCheats : public UCheatManager
{
	GENERATED_BODY()

public:
	// ========== 地图管理命令 ==========
	
	/** 
	 * 生成随机地图 
	 * 用法: LevelGenerateMap [Seed] [Layers]
	 * 示例: LevelGenerateMap 12345 3
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelGenerateMap(int32 Seed = 0, int32 Layers = 3);

	/** 
	 * 进入指定房间
	 * 用法: LevelEnterRoom [RoomID]
	 * 示例: LevelEnterRoom 1
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelEnterRoom(int32 RoomID);

	/** 
	 * 显示当前地图信息
	 * 用法: LevelShowMap
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelShowMap();

	// ========== 房间操作命令 ==========

	/** 
	 * 重置当前房间
	 * 用法: LevelResetRoom
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelResetRoom();

	/** 
	 * 完成当前房间
	 * 用法: LevelCompleteRoom
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelCompleteRoom();

	/** 
	 * 跳过当前房间
	 * 用法: LevelSkipRoom
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelSkipRoom();

	// ========== 战斗作弊命令 ==========

	/** 
	 * 生成敌人
	 * 用法: LevelSpawnEnemy [EnemyClassName] [Count]
	 * 示例: LevelSpawnEnemy BP_TestEnemy 5
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelSpawnEnemy(const FString& EnemyClassName, int32 Count = 1);

	/** 
	 * 杀死所有敌人
	 * 用法: LevelKillAllEnemies
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelKillAllEnemies();

	// ========== 资源作弊命令 ==========

	/** 
	 * 给予金币
	 * 用法: LevelGiveGold [Amount]
	 * 示例: LevelGiveGold 999
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelGiveGold(int32 Amount);

	/** 
	 * 给予神格碎片
	 * 用法: LevelGiveFragments [Amount]
	 * 示例: LevelGiveFragments 99
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelGiveFragments(int32 Amount);

	// ========== 事件房间作弊命令 ==========

	/** 
	 * 选择事件选项
	 * 用法: LevelChooseOption [OptionIndex]
	 * 示例: LevelChooseOption 0
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelChooseOption(int32 OptionIndex);

	/** 
	 * 跳过当前事件
	 * 用法: LevelSkipEvent
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelSkipEvent();

	/** 
	 * 显示当前事件信息
	 * 用法: LevelShowEventInfo
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelShowEventInfo();

	// ========== 商店房间作弊命令 ==========

	/** 
	 * 购买商店商品
	 * 用法: LevelBuyItem [ItemIndex]
	 * 示例: LevelBuyItem 0
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelBuyItem(int32 ItemIndex);

	/** 
	 * 完成购物离开商店
	 * 用法: LevelFinishShopping
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelFinishShopping();

	/** 
	 * 刷新商店商品
	 * 用法: LevelRefreshShop
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelRefreshShop();

	/** 
	 * 显示商店信息
	 * 用法: LevelShowShopInfo
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelShowShopInfo();

	/** 
	 * 显示玩家数据
	 * 用法: LevelShowPlayerData
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelShowPlayerData();

	/** 
	 * 开启所有房间
	 * 用法: LevelUnlockAllRooms
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelUnlockAllRooms();

	/** 
	 * 重置章节
	 * 用法: LevelResetChapter
	 */
	UFUNCTION(Exec, Category = "关卡作弊")
	void LevelResetChapter();

	// ========== 蓝图可调用函数（替代Exec命令） ==========

	/** 生成随机地图（蓝图版本） */
	UFUNCTION(BlueprintCallable, Category = "关卡作弊")
	void BP_GenerateMap(int32 Seed = 0, int32 Layers = 3);

	/** 进入房间（蓝图版本） */
	UFUNCTION(BlueprintCallable, Category = "关卡作弊")
	void BP_EnterRoom(int32 RoomID);

	/** 显示地图信息（蓝图版本） */
	UFUNCTION(BlueprintCallable, Category = "关卡作弊")
	void BP_ShowMap();

	/** 给予金币（蓝图版本） */
	UFUNCTION(BlueprintCallable, Category = "关卡作弊")
	void BP_GiveGold(int32 Amount);

	/** 给予碎片（蓝图版本） */
	UFUNCTION(BlueprintCallable, Category = "关卡作弊")
	void BP_GiveFragments(int32 Amount);

protected:
	/** 获取房间管理器 */
	URoomManager* GetRoomManager() const;

	/** 获取当前房间 */
	class ARoomBase* GetCurrentRoom() const;

	/** 获取玩家角色 */
	class APawn* GetPlayerPawn() const;
};
