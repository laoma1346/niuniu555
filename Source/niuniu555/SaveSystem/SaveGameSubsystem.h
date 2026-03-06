// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

// 前向声明
class UNiuniuSaveGame;

// 存档操作结果委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveGameOperationCompleted, bool, bSuccess, const FString&, Message);

/**
 * 存档子系统 - 管理游戏存档的加载、保存、删除等操作
 */
UCLASS()
class NIUNIU555_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem 接口
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== 存档槽位管理 ==========

	// 获取存档槽位数量
	UFUNCTION(BlueprintPure, Category = "SaveSystem")
	int32 GetMaxSaveSlots() const { return MaxSaveSlots; }

	// 检查存档槽位是否存在
	UFUNCTION(BlueprintPure, Category = "SaveSystem")
	bool DoesSaveSlotExist(int32 SlotIndex) const;

	// 获取存档槽位文件名
	UFUNCTION(BlueprintPure, Category = "SaveSystem")
	FString GetSaveSlotName(int32 SlotIndex) const;

	// ========== 存档操作 ==========

	// 创建新存档（异步）
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void CreateNewSaveGame(int32 SlotIndex, const FString& SaveName);

	// 加载存档（异步）
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void LoadSaveGame(int32 SlotIndex);

	// 保存当前存档（异步）
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void SaveCurrentGame();

	// 删除存档
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	bool DeleteSaveGame(int32 SlotIndex);

	// ========== 当前存档访问 ==========

	// 获取当前加载的存档
	UFUNCTION(BlueprintPure, Category = "SaveSystem")
	UNiuniuSaveGame* GetCurrentSaveGame() const { return CurrentSaveGame; }

	// 是否有当前存档
	UFUNCTION(BlueprintPure, Category = "SaveSystem")
	bool HasCurrentSaveGame() const { return CurrentSaveGame != nullptr; }

	// ========== 存档列表 ==========

	// 获取所有存档信息（用于存档列表UI）
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	TArray<UNiuniuSaveGame*> GetAllSaveGamesInfo() const;

	// ========== 存档数据修改 ==========

	// 更新金币数量
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void UpdateGold(int32 Amount);

	// 增加游戏时长
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void AddPlayTime(float DeltaSeconds);

	// 解锁技能
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void UnlockSkill(const FName& SkillId);

	// 添加神格碎片
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void AddGodFragments(int32 Amount);

	// 更新最高通关章节
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void UpdateHighestChapter(int32 ChapterIndex);

	// ========== 委托事件 ==========

	// 存档操作完成事件
	UPROPERTY(BlueprintAssignable, Category = "SaveSystem|Events")
	FOnSaveGameOperationCompleted OnSaveOperationCompleted;

	// 加载操作完成事件
	UPROPERTY(BlueprintAssignable, Category = "SaveSystem|Events")
	FOnSaveGameOperationCompleted OnLoadOperationCompleted;

protected:
	// 最大存档槽位数
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SaveSystem|Config")
	int32 MaxSaveSlots = 5;

	// 存档文件前缀
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SaveSystem|Config")
	FString SaveSlotPrefix = TEXT("NiuniuSave");

	// 当前加载的存档
	UPROPERTY()
	TObjectPtr<UNiuniuSaveGame> CurrentSaveGame;

	// 当前存档槽位索引（-1表示无）
	UPROPERTY()
	int32 CurrentSlotIndex = -1;

	// 存档操作完成的回调
	UFUNCTION()
	void OnCreateSaveGameCompleted(const FString& SlotName, int32 UserIndex, bool bSuccess);
	UFUNCTION()
	void OnLoadSaveGameCompleted(const FString& SlotName, int32 UserIndex, USaveGame* LoadedSaveGame);
	UFUNCTION()
	void OnSaveGameCompleted(const FString& SlotName, int32 UserIndex, bool bSuccess);
};
