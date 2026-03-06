// Copyright Epic Games, Inc. All Rights Reserved.

#include "InputPriorityManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

void UInputPriorityManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentInputMode = EInputMode::GameOnly;
	CurrentPriority = EInputPriority::Gameplay;
	ModeStack.Empty();
	bIsPaused = false;

	UE_LOG(LogTemp, Log, TEXT("InputPriorityManager: Initialized"));
}

void UInputPriorityManager::Deinitialize()
{
	ModeStack.Empty();
	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("InputPriorityManager: Deinitialized"));
}

void UInputPriorityManager::SetInputMode(EInputMode NewMode)
{
	if (NewMode == CurrentInputMode)
	{
		return;
	}

	EInputMode OldMode = CurrentInputMode;
	CurrentInputMode = NewMode;

	// 根据模式设置优先级
	switch (NewMode)
	{
	case EInputMode::GameOnly:
		CurrentPriority = EInputPriority::Gameplay;
		break;
	case EInputMode::UIOnly:
		CurrentPriority = EInputPriority::UI;
		break;
	case EInputMode::GameAndUI:
		CurrentPriority = EInputPriority::Gameplay;
		break;
	case EInputMode::None:
		CurrentPriority = EInputPriority::Lowest;
		break;
	}

	// 应用到PlayerController
	ApplyInputMode(NewMode);

	// 广播事件
	OnInputModeChanged.Broadcast(NewMode, OldMode);

	UE_LOG(LogTemp, Log, TEXT("InputPriorityManager: Input mode changed from %d to %d"),
		(int32)OldMode, (int32)NewMode);
}

void UInputPriorityManager::PushInputMode(EInputMode OverrideMode)
{
	// 保存当前模式
	ModeStack.Push(CurrentInputMode);
	// 设置新模式
	SetInputMode(OverrideMode);

	UE_LOG(LogTemp, Log, TEXT("InputPriorityManager: Pushed input mode, stack size: %d"), ModeStack.Num());
}

void UInputPriorityManager::PopInputMode()
{
	if (ModeStack.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputPriorityManager: Mode stack is empty, cannot pop"));
		return;
	}

	EInputMode PreviousMode = ModeStack.Pop();
	SetInputMode(PreviousMode);

	UE_LOG(LogTemp, Log, TEXT("InputPriorityManager: Popped input mode, restored to %d, stack size: %d"),
		(int32)PreviousMode, ModeStack.Num());
}

bool UInputPriorityManager::IsPriorityAllowed(EInputPriority TestPriority) const
{
	return (uint8)TestPriority <= (uint8)CurrentPriority;
}

bool UInputPriorityManager::IsGameplayInputAllowed() const
{
	if (bIsPaused)
	{
		return false;
	}

	return CurrentInputMode == EInputMode::GameOnly ||
		   CurrentInputMode == EInputMode::GameAndUI;
}

bool UInputPriorityManager::IsUIInputAllowed() const
{
	return CurrentInputMode == EInputMode::UIOnly ||
		   CurrentInputMode == EInputMode::GameAndUI;
}

bool UInputPriorityManager::IsCombatInputAllowed() const
{
	return IsGameplayInputAllowed() && CurrentPriority <= EInputPriority::Combat;
}

void UInputPriorityManager::EnterCombatMode()
{
	SetInputMode(EInputMode::GameOnly);
	SetInputPriority(EInputPriority::Combat);
	UE_LOG(LogTemp, Log, TEXT("InputPriorityManager: Entered combat mode"));
}

void UInputPriorityManager::ExitCombatMode()
{
	SetInputMode(EInputMode::GameOnly);
	SetInputPriority(EInputPriority::Gameplay);
	UE_LOG(LogTemp, Log, TEXT("InputPriorityManager: Exited combat mode"));
}

void UInputPriorityManager::OpenUI(EInputPriority Priority)
{
	PushInputMode(EInputMode::GameAndUI);
	SetInputPriority(Priority);
	UE_LOG(LogTemp, Log, TEXT("InputPriorityManager: Opened UI"));
}

void UInputPriorityManager::CloseUI()
{
	PopInputMode();
	UE_LOG(LogTemp, Log, TEXT("InputPriorityManager: Closed UI"));
}

void UInputPriorityManager::OpenMenu()
{
	PushInputMode(EInputMode::UIOnly);
	SetInputPriority(EInputPriority::Menu);
	UE_LOG(LogTemp, Log, TEXT("InputPriorityManager: Opened menu"));
}

void UInputPriorityManager::CloseMenu()
{
	PopInputMode();
	UE_LOG(LogTemp, Log, TEXT("InputPriorityManager: Closed menu"));
}

void UInputPriorityManager::PauseGameplayInput()
{
	if (!bIsPaused)
	{
		PrePauseMode = CurrentInputMode;
		bIsPaused = true;
		SetInputMode(EInputMode::UIOnly);
		UE_LOG(LogTemp, Log, TEXT("InputPriorityManager: Gameplay input paused"));
	}
}

void UInputPriorityManager::ResumeGameplayInput()
{
	if (bIsPaused)
	{
		bIsPaused = false;
		SetInputMode(PrePauseMode);
		UE_LOG(LogTemp, Log, TEXT("InputPriorityManager: Gameplay input resumed"));
	}
}

void UInputPriorityManager::ApplyInputMode(EInputMode Mode)
{
	// 获取本地玩家控制器
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		switch (Mode)
		{
		case EInputMode::GameOnly:
			{
				FInputModeGameOnly GameOnlyMode;
				GameOnlyMode.SetConsumeCaptureMouseDown(true);
				PC->SetInputMode(GameOnlyMode);
				PC->bShowMouseCursor = false;
			}
			break;

		case EInputMode::UIOnly:
			{
				FInputModeUIOnly UIOnlyMode;
				UIOnlyMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
				PC->SetInputMode(UIOnlyMode);
				PC->bShowMouseCursor = true;
			}
			break;

		case EInputMode::GameAndUI:
			{
				FInputModeGameAndUI GameAndUIMode;
				GameAndUIMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				GameAndUIMode.SetHideCursorDuringCapture(false);
				PC->SetInputMode(GameAndUIMode);
				PC->bShowMouseCursor = true;
			}
			break;

		case EInputMode::None:
			{
				// 不设置任何输入模式
				PC->bShowMouseCursor = false;
			}
			break;
		}
	}
}
