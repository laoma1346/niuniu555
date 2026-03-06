// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SceneManager.generated.h"

// 场景加载完成委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSceneLoadingComplete);

// 场景加载进度委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSceneLoadingProgress, float, Progress);

// 场景加载开始委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSceneLoadingStarted, const FString&, SceneName);

/**
 * 场景管理器 - 管理游戏的场景加载和切换
 * 支持异步加载、加载界面、场景过渡效果
 */
UCLASS()
class NIUNIU555_API USceneManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== 场景配置 ==========

	// 主菜单场景名
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow|Scene")
	FName MainMenuScene = TEXT("MainMenuMap");

	// 营地场景名
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow|Scene")
	FName CampScene = TEXT("CampMap");

	// 地图选择场景名
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow|Scene")
	FName MapSelectionScene = TEXT("MapSelectionMap");

	// 战斗场景前缀
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow|Scene")
	FString CombatScenePrefix = TEXT("Combat_");

	// 加载界面场景名
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow|Scene")
	FName LoadingScreenScene = TEXT("LoadingScreenMap");

	// ========== 场景查询 ==========

	// 获取当前场景名
	UFUNCTION(BlueprintPure, Category = "GameFlow|Scene")
	FName GetCurrentSceneName() const { return CurrentSceneName; }

	// 获取上一个场景名
	UFUNCTION(BlueprintPure, Category = "GameFlow|Scene")
	FName GetPreviousSceneName() const { return PreviousSceneName; }

	// 是否正在加载场景
	UFUNCTION(BlueprintPure, Category = "GameFlow|Scene")
	bool IsLoadingScene() const { return bIsLoading; }

	// 获取加载进度
	UFUNCTION(BlueprintPure, Category = "GameFlow|Scene")
	float GetLoadingProgress() const { return LoadingProgress; }

	// ========== 场景切换 ==========

	// 异步加载场景
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Scene")
	void LoadScene(const FName& SceneName, bool bShowLoadingScreen = true);

	// 加载主菜单
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Scene")
	void LoadMainMenu() { LoadScene(MainMenuScene, false); }

	// 加载营地
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Scene")
	void LoadCamp() { LoadScene(CampScene, true); }

	// 加载地图选择
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Scene")
	void LoadMapSelection() { LoadScene(MapSelectionScene, true); }

	// 加载战斗场景
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Scene")
	void LoadCombatScene(const FString& CombatLevelName);

	// 重新加载当前场景
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Scene")
	void ReloadCurrentScene();

	// ========== 加载控制 ==========

	// 显示加载界面
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Scene")
	void ShowLoadingScreen();

	// 隐藏加载界面
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Scene")
	void HideLoadingScreen();

	// 取消加载（如果可以）
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Scene")
	void CancelLoading();

	// ========== 事件委托 ==========

	// 场景加载开始
	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Scene|Events")
	FOnSceneLoadingStarted OnSceneLoadingStarted;

	// 场景加载进度更新
	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Scene|Events")
	FOnSceneLoadingProgress OnSceneLoadingProgress;

	// 场景加载完成
	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Scene|Events")
	FOnSceneLoadingComplete OnSceneLoadingComplete;

protected:
	// 当前场景名
	UPROPERTY(BlueprintReadOnly, Category = "GameFlow|Scene")
	FName CurrentSceneName;

	// 上一个场景名
	UPROPERTY(BlueprintReadOnly, Category = "GameFlow|Scene")
	FName PreviousSceneName;

	// 目标场景名
	UPROPERTY()
	FName TargetSceneName;

	// 是否正在加载
	UPROPERTY(BlueprintReadOnly, Category = "GameFlow|Scene")
	bool bIsLoading = false;

	// 加载进度
	UPROPERTY(BlueprintReadOnly, Category = "GameFlow|Scene")
	float LoadingProgress = 0.0f;

	// 是否显示加载界面
	UPROPERTY()
	bool bShouldShowLoadingScreen = true;

	// 加载句柄
	UPROPERTY()
	TWeakObjectPtr<UWorld> LoadingWorld;

	// 执行场景加载
	void ExecuteLoadScene(const FName& SceneName);

	// 加载完成回调
	void OnSceneLoaded();

	// 加载进度更新
	void UpdateLoadingProgress();

public:
	// Tick更新（用于更新加载进度）
	UFUNCTION(BlueprintCallable, Category = "GameFlow|Scene")
	void Update(float DeltaTime);
};
