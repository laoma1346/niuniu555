// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SaveSystemTestActor.generated.h"

class USaveGameSubsystem;

/**
 * 存档系统测试Actor - 用于快速测试存档功能
 * 可在编辑器中放置此Actor，通过按键或蓝图调用测试函数
 */
UCLASS()
class NIUNIU555_API ASaveSystemTestActor : public AActor
{
	GENERATED_BODY()

public:
	ASaveSystemTestActor();

	// ========== 测试功能 ==========

	// 测试创建新存档
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Test")
	void TestCreateSave(int32 SlotIndex = 0, const FString& SaveName = TEXT("测试存档"));

	// 测试加载存档
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Test")
	void TestLoadSave(int32 SlotIndex = 0);

	// 测试保存当前游戏
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Test")
	void TestSaveCurrentGame();

	// 测试删除存档
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Test")
	void TestDeleteSave(int32 SlotIndex = 0);

	// 测试添加金币
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Test")
	void TestAddGold(int32 Amount = 100);

	// 测试添加神格碎片
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Test")
	void TestAddGodFragments(int32 Amount = 10);

	// 测试解锁技能
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Test")
	void TestUnlockSkill(const FString& SkillId);

	// 测试更新章节
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Test")
	void TestUpdateChapter(int32 ChapterIndex);

	// 打印所有存档信息
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Test")
	void TestPrintAllSaveInfo();

	// 打印当前存档信息
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Test")
	void TestPrintCurrentSaveInfo();

protected:
	virtual void BeginPlay() override;

	// 绑定存档系统事件
	UFUNCTION()
	void OnSaveCompleted(bool bSuccess, const FString& Message);

	UFUNCTION()
	void OnLoadCompleted(bool bSuccess, const FString& Message);

	// 获取存档子系统
	USaveGameSubsystem* GetSaveSubsystem() const;

	// 是否已绑定事件
	bool bEventsBound = false;
};

/**
 * 存档自动保存组件 - 可附加到角色或GameMode上实现自动保存
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NIUNIU555_API USaveAutoSaveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USaveAutoSaveComponent();

	// 开始自动保存（每隔Interval秒自动保存一次）
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|AutoSave")
	void StartAutoSave(float Interval = 60.0f);

	// 停止自动保存
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|AutoSave")
	void StopAutoSave();

	// 立即执行一次保存
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|AutoSave")
	void DoAutoSaveNow();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 自动保存定时器
	FTimerHandle AutoSaveTimerHandle;

	// 是否启用自动保存
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSystem|AutoSave")
	bool bEnableAutoSave = false;

	// 自动保存间隔（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSystem|AutoSave")
	float AutoSaveInterval = 60.0f;

	// 自动保存槽位索引
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSystem|AutoSave")
	int32 AutoSaveSlotIndex = 0;

	// 保存定时器回调
	void OnAutoSaveTimer();
};
