#include "LevelTransitionManager.h"
#include "RoomBase.h"
// #include "CharacterSystem/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "Blueprint/UserWidget.h"

ULevelTransitionManager* ULevelTransitionManager::Get(const UObject* WorldContextObject)
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

	return GameInstance->GetSubsystem<ULevelTransitionManager>();
}

void ULevelTransitionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("[LevelTransitionManager] 初始化"));

	bIsLoading = false;
	LoadingProgress = 0.0f;
}

void ULevelTransitionManager::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("[LevelTransitionManager] 反初始化"));

	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->RemoveFromParent();
		LoadingScreenWidget = nullptr;
	}

	Super::Deinitialize();
}

void ULevelTransitionManager::LoadRoomLevel(ARoomBase* Room, bool bShowLoadingScreen)
{
	if (!Room || bIsLoading)
	{
		return;
	}

	TargetRoom = Room;
	bIsLoading = true;
	LoadingProgress = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("[LevelTransitionManager] 开始加载房间: %s"), *Room->RoomName);

	if (bShowLoadingScreen)
	{
		ShowLoadingScreen();
	}

	StartAsyncLoading();
}

void ULevelTransitionManager::ShowLoadingScreen()
{
	if (!LoadingScreenWidgetClass)
	{
		return;
	}

	// 获取玩家控制器
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		return;
	}

	// 创建加载界面
	LoadingScreenWidget = CreateWidget<UUserWidget>(PC, LoadingScreenWidgetClass);
	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->AddToViewport(100); // 高ZOrder

		// 设置输入模式（阻止玩家输入）
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(LoadingScreenWidget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

void ULevelTransitionManager::HideLoadingScreen()
{
	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->RemoveFromParent();
		LoadingScreenWidget = nullptr;
	}

	// 恢复游戏输入
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		if (PC)
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = false;
		}
	}
}

void ULevelTransitionManager::StartAsyncLoading()
{
	if (!TargetRoom)
	{
		return;
	}

	// 保存玩家状态
	SavePlayerState();

	// 开始加载进度检查
	GetWorld()->GetTimerManager().SetTimer(ProgressTimerHandle, this, &ULevelTransitionManager::CheckLoadingProgress, 0.1f, true);

	if (bUseStreaming && !TargetRoom->LevelName.IsNone())
	{
		// 使用关卡流媒体
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		LatentInfo.ExecutionFunction = FName("OnLevelLoaded");
		LatentInfo.Linkage = 0;
		LatentInfo.UUID = __LINE__;

		UGameplayStatics::LoadStreamLevel(this, TargetRoom->LevelName, true, true, LatentInfo);
	}
	else if (!TargetRoom->LevelName.IsNone())
	{
		// 直接打开关卡
		UGameplayStatics::OpenLevel(this, TargetRoom->LevelName);
		OnLevelLoaded();
	}
	else
	{
		// 没有指定关卡，直接完成
		OnLevelLoaded();
	}
}

void ULevelTransitionManager::CheckLoadingProgress()
{
	// 模拟进度增加
	LoadingProgress = FMath::Min(LoadingProgress + 0.1f, 0.95f);

	OnLoadingProgressUpdated.Broadcast(LoadingProgress);

	// 实际检查关卡加载状态
	if (TargetRoom && !TargetRoom->LevelName.IsNone())
	{
		// 获取关卡流
		ULevelStreaming* LevelStreaming = UGameplayStatics::GetStreamingLevel(this, TargetRoom->LevelName);
		if (LevelStreaming && LevelStreaming->IsLevelLoaded())
		{
			LoadingProgress = 1.0f;
			OnLoadingProgressUpdated.Broadcast(1.0f);
		}
	}
}

void ULevelTransitionManager::OnLevelLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("[LevelTransitionManager] 关卡加载完成"));

	// 恢复玩家状态
	RestorePlayerState();

	// 完成过渡
	FinishTransition();
}

void ULevelTransitionManager::FinishTransition()
{
	GetWorld()->GetTimerManager().ClearTimer(ProgressTimerHandle);

	HideLoadingScreen();

	if (TargetRoom)
	{
		TargetRoom->EnterRoom();
	}

	bIsLoading = false;
	LoadingProgress = 1.0f;

	OnTransitionFinished.Broadcast();
}

void ULevelTransitionManager::PlayTransition(bool bFadeToBlack, float Duration)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	OnTransitionStarted.Broadcast();

	// 使用玩家摄像机的Fade功能
	if (bFadeToBlack)
	{
		PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, Duration, FLinearColor::Black, false, true);
	}
	else
	{
		PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, Duration, FLinearColor::Black, false, true);
	}

	// 定时触发完成事件 - 使用成员函数绑定而非Lambda
	bIsFading = true;
	GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, this, &ULevelTransitionManager::ExecuteFadeComplete, Duration, false);
}

void ULevelTransitionManager::PerformRoomTransition(ARoomBase* FromRoom, ARoomBase* ToRoom)
{
	if (!ToRoom)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Error, TEXT("[LevelTransitionManager] PerformRoomTransition: World无效"));
		return;
	}

	// 淡出
	PlayTransition(true, DefaultTransitionDuration * 0.5f);

	// ✅ 保存目标房间和持续时间（避免使用Lambda捕获）
	TargetRoom = ToRoom;
	SavedTransitionDuration = DefaultTransitionDuration;

	// 延迟后加载新房间 - 使用成员函数绑定而非Lambda
	FTimerDelegate LoadDelegate;
	LoadDelegate.BindUObject(this, &ULevelTransitionManager::ExecuteDelayedLoad);
	World->GetTimerManager().SetTimer(TransitionTimerHandle, LoadDelegate, DefaultTransitionDuration * 0.5f, false);
}

void ULevelTransitionManager::ExecuteDelayedLoad()
{
	// ✅ 检查有效性
	if (!IsValid(this))
	{
		return;
	}

	if (IsValid(TargetRoom))
	{
		// 加载房间
		LoadRoomLevel(TargetRoom, false);

		// 淡入
		PlayTransition(false, SavedTransitionDuration * 0.5f);
	}

	// 清空临时引用
	TargetRoom = nullptr;
}

void ULevelTransitionManager::ExecuteFadeComplete()
{
	// ✅ 检查有效性
	if (!IsValid(this))
	{
		return;
	}

	bIsFading = false;
	OnTransitionFinished.Broadcast();
}

void ULevelTransitionManager::SavePlayerState()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	// 获取玩家属性
	// TODO: 从PlayerCharacter获取实际数值
	SavedHealthPercent = 1.0f;
	SavedManaPercent = 1.0f;

	// TODO: 从玩家数据获取资源
	SavedGold = 0;
	SavedFragments = 0;

	UE_LOG(LogTemp, Log, TEXT("[LevelTransitionManager] 保存玩家状态: HP=%.0f%% Gold=%d"), 
		SavedHealthPercent * 100.0f, SavedGold);
}

void ULevelTransitionManager::RestorePlayerState()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	// TODO: 恢复玩家属性
	// Player->SetHealthPercent(SavedHealthPercent);
	// Player->SetManaPercent(SavedManaPercent);
	// Player->SetGold(SavedGold);
	// Player->SetFragments(SavedFragments);

	UE_LOG(LogTemp, Log, TEXT("[LevelTransitionManager] 恢复玩家状态"));
}

APawn* ULevelTransitionManager::FindPlayerCharacter() const
{
	return UGameplayStatics::GetPlayerPawn(this, 0);
}
