// Copyright Epic Games, Inc. All Rights Reserved.

#include "SceneManager.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"

void USceneManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	PreviousSceneName = NAME_None;
	TargetSceneName = NAME_None;
	bIsLoading = false;
	LoadingProgress = 0.0f;

	// 获取当前场景名
	if (UWorld* World = GetWorld())
	{
		CurrentSceneName = FName(*World->GetMapName());
	}
	else
	{
		CurrentSceneName = NAME_None;
	}

	UE_LOG(LogTemp, Log, TEXT("SceneManager: Initialized, Current Scene: %s"), *CurrentSceneName.ToString());
}

void USceneManager::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("SceneManager: Deinitialized"));
}

void USceneManager::LoadScene(const FName& SceneName, bool bShowLoadingScreen)
{
	if (SceneName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("SceneManager: Cannot load scene with empty name"));
		return;
	}

	if (bIsLoading)
	{
		UE_LOG(LogTemp, Warning, TEXT("SceneManager: Already loading scene %s, cannot load %s"),
			*TargetSceneName.ToString(), *SceneName.ToString());
		return;
	}

	if (CurrentSceneName == SceneName)
	{
		UE_LOG(LogTemp, Warning, TEXT("SceneManager: Already in scene %s"), *SceneName.ToString());
		return;
	}

	// 记录场景切换
	PreviousSceneName = CurrentSceneName;
	TargetSceneName = SceneName;
	bShouldShowLoadingScreen = bShowLoadingScreen;
	bIsLoading = true;
	LoadingProgress = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("SceneManager: Starting load scene %s"), *SceneName.ToString());

	// 广播开始加载事件
	OnSceneLoadingStarted.Broadcast(SceneName.ToString());

	// 如果需要，显示加载界面
	if (bShowLoadingScreen)
	{
		ShowLoadingScreen();
	}

	// 执行加载
	ExecuteLoadScene(SceneName);
}

void USceneManager::LoadCombatScene(const FString& CombatLevelName)
{
	FString FullSceneName = CombatScenePrefix + CombatLevelName;
	LoadScene(FName(*FullSceneName), true);
}

void USceneManager::ReloadCurrentScene()
{
	if (!CurrentSceneName.IsNone())
	{
		LoadScene(CurrentSceneName, false);
	}
}

void USceneManager::ShowLoadingScreen()
{
	// TODO: 打开加载界面Widget
	// 这里先记录日志，后续可以实现UMG加载界面
	UE_LOG(LogTemp, Log, TEXT("SceneManager: Showing loading screen"));
}

void USceneManager::HideLoadingScreen()
{
	// TODO: 关闭加载界面Widget
	UE_LOG(LogTemp, Log, TEXT("SceneManager: Hiding loading screen"));
}

void USceneManager::CancelLoading()
{
	if (bIsLoading)
	{
		// 注意：UE的流式加载一旦开始很难取消
		// 这里主要是标记状态，实际加载会继续
		UE_LOG(LogTemp, Warning, TEXT("SceneManager: Loading cancellation not fully supported"));
		bIsLoading = false;
		LoadingProgress = 0.0f;
	}
}

void USceneManager::ExecuteLoadScene(const FName& SceneName)
{
	// 构建关卡路径
	FString LevelPath = FString::Printf(TEXT("/Game/Maps/%s"), *SceneName.ToString());
	
	// 使用UGameplayStatics的OpenLevel进行加载
	// 这是最简单的方式，会自动处理加载
	if (UWorld* World = GetWorld())
	{
		FString LevelName = SceneName.ToString();
		
		// 异步加载需要更复杂的实现，这里使用同步加载作为基础版本
		// 后续可以升级为真正的异步流式加载
		UGameplayStatics::OpenLevel(World, FName(*LevelName));
		
		// 更新当前场景
		CurrentSceneName = SceneName;
		
		// 模拟加载完成
		LoadingProgress = 1.0f;
		OnSceneLoaded();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SceneManager: Cannot get World to load scene"));
		bIsLoading = false;
	}
}

void USceneManager::OnSceneLoaded()
{
	bIsLoading = false;
	
	// 隐藏加载界面
	if (bShouldShowLoadingScreen)
	{
		HideLoadingScreen();
	}

	UE_LOG(LogTemp, Log, TEXT("SceneManager: Scene %s loaded successfully"), *CurrentSceneName.ToString());

	// 广播加载完成
	OnSceneLoadingComplete.Broadcast();
}

void USceneManager::UpdateLoadingProgress()
{
	// 在实际异步加载中，这里会更新LoadingProgress
	// 目前使用OpenLevel是同步的，所以直接设置为1.0
	if (bIsLoading)
	{
		LoadingProgress = FMath::Min(LoadingProgress + 0.01f, 0.99f);
		OnSceneLoadingProgress.Broadcast(LoadingProgress);
	}
}

void USceneManager::Update(float DeltaTime)
{
	if (bIsLoading)
	{
		UpdateLoadingProgress();
	}
}
