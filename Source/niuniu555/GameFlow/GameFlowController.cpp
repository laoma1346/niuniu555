// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFlowController.h"
#include "GameStateMachine.h"
#include "SceneManager.h"
#include "Kismet/GameplayStatics.h"

void UGameFlowController::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bIsInitialized = false;
	PrePauseGameSpeed = 1.0f;

	UE_LOG(LogTemp, Log, TEXT("GameFlowController: Initialized"));
}

void UGameFlowController::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("GameFlowController: Deinitialized"));
}

void UGameFlowController::InitializeGame()
{
	if (bIsInitialized)
	{
		return;
	}

	// 获取子系统
	UGameStateMachine* StateMachine = GetStateMachine();
	USceneManager* SceneMgr = GetSceneManager();

	if (StateMachine && SceneMgr)
	{
		// 绑定事件
		StateMachine->OnGameStateChanged.AddDynamic(this, &UGameFlowController::OnGameStateChanged);
		SceneMgr->OnSceneLoadingComplete.AddDynamic(this, &UGameFlowController::OnSceneLoadingComplete);

		bIsInitialized = true;
		UE_LOG(LogTemp, Log, TEXT("GameFlowController: Game initialized"));
	}
}

void UGameFlowController::StartNewGame()
{
	InitializeGame();

	UGameStateMachine* StateMachine = GetStateMachine();
	USceneManager* SceneMgr = GetSceneManager();

	if (StateMachine && SceneMgr)
	{
		// 先加载主菜单
		SceneMgr->LoadMainMenu();
		StateMachine->TransitionToState(EGameState::MainMenu, TEXT("StartNewGame"));

		UE_LOG(LogTemp, Log, TEXT("GameFlowController: New game started"));
	}
}

void UGameFlowController::ContinueGame(int32 SaveSlotIndex)
{
	InitializeGame();

	// TODO: 从存档加载游戏状态
	// 这里先实现基本流程
	FromMainMenuToCamp();

	UE_LOG(LogTemp, Log, TEXT("GameFlowController: Continuing game from slot %d"), SaveSlotIndex);
}

void UGameFlowController::FromMainMenuToCamp()
{
	UGameStateMachine* StateMachine = GetStateMachine();
	USceneManager* SceneMgr = GetSceneManager();

	if (StateMachine && SceneMgr)
	{
		StateMachine->StartGame();
		SceneMgr->LoadCamp();
	}
}

void UGameFlowController::FromCampToMapSelection()
{
	UGameStateMachine* StateMachine = GetStateMachine();
	USceneManager* SceneMgr = GetSceneManager();

	if (StateMachine && SceneMgr)
	{
		StateMachine->EnterMapSelection();
		SceneMgr->LoadMapSelection();
	}
}

void UGameFlowController::FromMapToCombat(const FString& CombatLevelName)
{
	UGameStateMachine* StateMachine = GetStateMachine();
	USceneManager* SceneMgr = GetSceneManager();

	if (StateMachine && SceneMgr)
	{
		StateMachine->StartCombat(CombatLevelName);
		SceneMgr->LoadCombatScene(CombatLevelName);
	}
}

void UGameFlowController::FromMapToEvent(const FString& EventID)
{
	UGameStateMachine* StateMachine = GetStateMachine();

	if (StateMachine)
	{
		StateMachine->EnterEvent(EventID);
		// 事件通常在同一场景或通过UI处理，不需要切换场景
	}
}

void UGameFlowController::FromMapToShop()
{
	UGameStateMachine* StateMachine = GetStateMachine();

	if (StateMachine)
	{
		StateMachine->EnterShop();
		// 商店通常通过UI实现
	}
}

void UGameFlowController::FromMapToSkillTree()
{
	UGameStateMachine* StateMachine = GetStateMachine();

	if (StateMachine)
	{
		StateMachine->EnterSkillTree();
		// 技能树通常通过UI实现
	}
}

void UGameFlowController::FromRoomToMap()
{
	UGameStateMachine* StateMachine = GetStateMachine();
	USceneManager* SceneMgr = GetSceneManager();

	if (StateMachine && SceneMgr)
	{
		StateMachine->EnterMapSelection();
		SceneMgr->LoadMapSelection();
	}
}

void UGameFlowController::CombatToSettlement(bool bVictory)
{
	UGameStateMachine* StateMachine = GetStateMachine();

	if (StateMachine)
	{
		StateMachine->CombatSettlement(bVictory);
		// 结算通常通过UI实现
	}
}

void UGameFlowController::SettlementToMap()
{
	FromRoomToMap();
}

void UGameFlowController::ReturnToMainMenu()
{
	UGameStateMachine* StateMachine = GetStateMachine();
	USceneManager* SceneMgr = GetSceneManager();

	if (StateMachine && SceneMgr)
	{
		StateMachine->ReturnToMainMenu();
		SceneMgr->LoadMainMenu();
	}
}

void UGameFlowController::QuitGame()
{
	// 先保存游戏（如果需要）
	// TODO: 调用存档系统保存

	// 退出游戏
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

UGameStateMachine* UGameFlowController::GetStateMachine() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UGameStateMachine>();
	}
	return nullptr;
}

USceneManager* UGameFlowController::GetSceneManager() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<USceneManager>();
	}
	return nullptr;
}

bool UGameFlowController::IsInGame() const
{
	if (UGameStateMachine* StateMachine = GetStateMachine())
	{
		EGameFlowState FlowState = StateMachine->GetCurrentFlowState();
		return FlowState != EGameFlowState::OutOfGame;
	}
	return false;
}

bool UGameFlowController::CanPause() const
{
	if (UGameStateMachine* StateMachine = GetStateMachine())
	{
		EGameState CurrentState = StateMachine->GetCurrentState();
		return CurrentState == EGameState::Combat ||
			   CurrentState == EGameState::MapSelection ||
			   CurrentState == EGameState::Camp;
	}
	return false;
}

void UGameFlowController::TogglePause()
{
	if (!CanPause())
	{
		return;
	}

	UGameStateMachine* StateMachine = GetStateMachine();
	if (!StateMachine)
	{
		return;
	}

	if (StateMachine->GetCurrentState() == EGameState::Pause)
	{
		// 恢复游戏
		StateMachine->ResumeGame();
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
	else
	{
		// 暂停游戏
		StateMachine->PauseGame();
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
}

void UGameFlowController::OnGameStateChanged(const FStateTransitionInfo& TransitionInfo)
{
	UE_LOG(LogTemp, Log, TEXT("GameFlowController: State changed %d -> %d"),
		(int32)TransitionInfo.PreviousState, (int32)TransitionInfo.NewState);

	// 根据状态变化执行相应逻辑
	switch (TransitionInfo.NewState)
	{
	case EGameState::MainMenu:
		// 主菜单逻辑
		break;

	case EGameState::Camp:
		// 营地准备逻辑
		break;

	case EGameState::Combat:
		// 战斗开始逻辑
		break;

	case EGameState::Pause:
		// 暂停逻辑
		break;

	case EGameState::Settlement:
		// 结算逻辑
		break;

	case EGameState::GameOver:
		// 游戏结束逻辑
		break;

	default:
		break;
	}
}

void UGameFlowController::OnSceneLoadingComplete()
{
	UE_LOG(LogTemp, Log, TEXT("GameFlowController: Scene loading completed"));
	// 场景加载完成后的逻辑
}

void UGameFlowController::Update(float DeltaTime)
{
	if (UGameStateMachine* StateMachine = GetStateMachine())
	{
		StateMachine->Update(DeltaTime);
	}

	if (USceneManager* SceneMgr = GetSceneManager())
	{
		SceneMgr->Update(DeltaTime);
	}
}
