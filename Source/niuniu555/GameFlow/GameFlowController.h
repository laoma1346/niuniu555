// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFlowController.generated.h"

class UGameStateMachine;
class USceneManager;

// 游戏流程控制器 - 整合状态机和场景管理
UCLASS()
class NIUNIU555_API UGameFlowController : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== 游戏启动 ==========

	// 初始化游戏（从主菜单开始）
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void InitializeGame();

	// 开始新游戏
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void StartNewGame();

	// 继续游戏（从存档加载）
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void ContinueGame(int32 SaveSlotIndex);

	// ========== 核心流程 ==========

	// 从主菜单进入营地
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void FromMainMenuToCamp();

	// 从营地进入地图选择
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void FromCampToMapSelection();

	// 从地图选择进入战斗
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void FromMapToCombat(const FString& CombatLevelName);

	// 从地图选择进入事件
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void FromMapToEvent(const FString& EventID);

	// 从地图选择进入商店
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void FromMapToShop();

	// 从地图选择进入技能树
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void FromMapToSkillTree();

	// 从各种房间返回地图
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void FromRoomToMap();

	// 战斗结束进入结算
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void CombatToSettlement(bool bVictory);

	// 结算后返回地图
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void SettlementToMap();

	// 返回主菜单
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void ReturnToMainMenu();

	// 退出游戏
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void QuitGame();

	// ========== 状态查询 ==========

	// 获取状态机
	UFUNCTION(BlueprintPure, Category = "GameFlow|Controller")
	UGameStateMachine* GetStateMachine() const;

	// 获取场景管理器
	UFUNCTION(BlueprintPure, Category = "GameFlow|Controller")
	USceneManager* GetSceneManager() const;

	// 检查是否在游戏中
	UFUNCTION(BlueprintPure, Category = "GameFlow|Controller")
	bool IsInGame() const;

	// 检查是否可以暂停
	UFUNCTION(BlueprintPure, Category = "GameFlow|Controller")
	bool CanPause() const;

	// ========== 暂停控制 ==========

	// 暂停/恢复游戏
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void TogglePause();

	// ========== Tick更新 ==========
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Controller")
	void Update(float DeltaTime);

protected:
	// 状态改变回调
	UFUNCTION()
	void OnGameStateChanged(const FStateTransitionInfo& TransitionInfo);

	// 场景加载完成回调
	UFUNCTION()
	void OnSceneLoadingComplete();

	// 是否已初始化
	UPROPERTY()
	bool bIsInitialized = false;

	// 暂停前的游戏速度
	UPROPERTY()
	float PrePauseGameSpeed = 1.0f;
};
