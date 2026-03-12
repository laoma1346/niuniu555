#pragma once

#include "CoreMinimal.h"
#include "LevelTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RoomManager.generated.h"

class UChapterConfigDataAsset;

class ARoomBase;
class ULevelGenerator;

// 房间管理器事件委托（使用独特名称避免与LevelTypes.h冲突）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoomEnteredDelegate, ARoomBase*, FromRoom, ARoomBase*, ToRoom);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomLeftDelegate, ARoomBase*, Room);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMapGeneratedDelegate);

/**
 * 房间管理器
 * 全局子系统，管理所有房间的生命周期和状态
 */
UCLASS()
class NIUNIU555_API URoomManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 获取房间管理器 */
	UFUNCTION(BlueprintPure, Category = "房间管理", meta = (WorldContext = "WorldContextObject"))
	static URoomManager* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== 房间注册 ==========

	/** 注册房间 */
	UFUNCTION(BlueprintCallable, Category = "房间管理")
	void RegisterRoom(ARoomBase* Room);

	/** 注销房间 */
	UFUNCTION(BlueprintCallable, Category = "房间管理")
	void UnregisterRoom(ARoomBase* Room);

	/** 获取所有房间 */
	UFUNCTION(BlueprintPure, Category = "房间管理")
	const TArray<ARoomBase*>& GetAllRooms() const { return RegisteredRooms; }

	/** 根据ID获取房间 */
	UFUNCTION(BlueprintPure, Category = "房间管理")
	ARoomBase* GetRoomByID(int32 RoomID) const;

	/** 获取当前房间 */
	UFUNCTION(BlueprintPure, Category = "房间管理")
	ARoomBase* GetCurrentRoom() const { return CurrentRoom; }

	// ========== 房间切换 ==========

	/**
	 * 进入房间
	 * @param Room 目标房间
	 * @param bWithLoading 是否显示加载界面
	 */
	UFUNCTION(BlueprintCallable, Category = "房间切换")
	bool EnterRoom(ARoomBase* Room, bool bWithLoading = true);

	/**
	 * 根据ID进入房间
	 */
	UFUNCTION(BlueprintCallable, Category = "房间切换")
	bool EnterRoomByID(int32 RoomID, bool bWithLoading = true);

	/** 离开当前房间 */
	UFUNCTION(BlueprintCallable, Category = "房间切换")
	bool LeaveCurrentRoom();

	// ========== 房间生成 ==========

	/**
	 * 生成随机地图
	 * @param Seed 随机种子（0表示使用随机种子）
	 * @param NumLayers 层数
	 */
	UFUNCTION(BlueprintCallable, Category = "地图生成")
	void GenerateRandomMap(int32 Seed = 0, int32 NumLayers = 3);

	/**
	 * 使用章节配置生成地图
	 * @param Config 章节配置
	 * @param Seed 随机种子
	 */
	UFUNCTION(BlueprintCallable, Category = "地图生成")
	void GenerateMapFromConfig(UChapterConfigDataAsset* Config, int32 Seed = 0);

	/** 清除所有房间 */
	UFUNCTION(BlueprintCallable, Category = "地图生成")
	void ClearAllRooms();

	// ========== 地图状态 ==========

	/** 获取地图是否已生成 */
	UFUNCTION(BlueprintPure, Category = "地图状态")
	bool HasGeneratedMap() const { return bMapGenerated; }

	/** 获取当前层数 */
	UFUNCTION(BlueprintPure, Category = "地图状态")
	int32 GetCurrentLayer() const { return CurrentLayer; }

	/** 获取总层数 */
	UFUNCTION(BlueprintPure, Category = "地图状态")
	int32 GetTotalLayers() const { return TotalLayers; }

	// ========== 持久化数据 ==========

	/** 进入房间时保留的玩家数据 */
	struct FCrossRoomData
	{
		float HealthPercent = 1.0f;
		float ManaPercent = 1.0f;
		int32 Gold = 0;
		int32 Fragments = 0;
	};

	/** 获取跨房间数据 */
	const FCrossRoomData& GetCrossRoomData() const { return CrossRoomData; }

	/** 更新跨房间数据 */
	void UpdateCrossRoomData(const FCrossRoomData& Data) { CrossRoomData = Data; }

	// ========== 事件委托 ==========

	/** 进入房间事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnRoomEnteredDelegate OnRoomEntered;

	/** 离开房间事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnRoomLeftDelegate OnRoomLeft;

	/** 地图生成完成事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnMapGeneratedDelegate OnMapGenerated;

protected:
	/** 是否已初始化（防止重复初始化） */
	bool bInitialized = false;

	/** 已注册的房间列表 */
	UPROPERTY()
	TArray<ARoomBase*> RegisteredRooms;

	/** 当前房间 */
	UPROPERTY()
	ARoomBase* CurrentRoom = nullptr;

	/** 地图是否已生成 */
	bool bMapGenerated = false;

	/** 当前层数 */
	int32 CurrentLayer = 0;

	/** 总层数 */
	int32 TotalLayers = 0;

	/** 跨房间数据 */
	FCrossRoomData CrossRoomData;

	/** 地图生成器（运行时创建） */
	UPROPERTY()
	ULevelGenerator* LevelGenerator = nullptr;

	/** 当前章节配置 */
	UPROPERTY()
	UChapterConfigDataAsset* CurrentChapterConfig = nullptr;

public:
	/** 获取当前章节配置 */
	UFUNCTION(BlueprintPure, Category = "章节配置")
	UChapterConfigDataAsset* GetCurrentChapterConfig() const { return CurrentChapterConfig; }

	/** 执行房间切换（协程） */
	void DoRoomTransition(ARoomBase* FromRoom, ARoomBase* ToRoom);

	/** 异步加载完成回调 */
	void OnLevelLoadComplete();

private:
	/** World清理事件委托句柄 */
	FDelegateHandle WorldCleanupDelegateHandle;
	
	/** 初始化定时器句柄 */
	FTimerHandle InitDelayTimerHandle;
	
	/** 处理World清理 */
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
	
	/** 初始化内部 */
	void InitializeInternal();
};
