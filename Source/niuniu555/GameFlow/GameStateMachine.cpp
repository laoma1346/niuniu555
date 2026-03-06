// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameStateMachine.h"

void UGameStateMachine::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentState = EGameState::None;
	PreviousState = EGameState::None;
	StateEnterTime = 0.0f;
	CurrentWorldTime = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("GameStateMachine: Initialized"));
}

void UGameStateMachine::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("GameStateMachine: Deinitialized"));
}

EGameFlowState UGameStateMachine::GetCurrentFlowState() const
{
	switch (CurrentState)
	{
	case EGameState::MainMenu:
	case EGameState::GameOver:
		return EGameFlowState::OutOfGame;
	case EGameState::Camp:
	case EGameState::SkillTree:
		return EGameFlowState::Preparation;
	case EGameState::MapSelection:
	case EGameState::Shop:
	case EGameState::Event:
		return EGameFlowState::Adventure;
	case EGameState::Combat:
	case EGameState::Pause:
		return EGameFlowState::Combat;
	case EGameState::Settlement:
		return EGameFlowState::Settlement;
	default:
		return EGameFlowState::OutOfGame;
	}
}

bool UGameStateMachine::TransitionToState(EGameState NewState, const FString& TransitionData)
{
	if (NewState == CurrentState)
	{
		return false;
	}
	ExecuteStateTransition(NewState, TransitionData);
	return true;
}

void UGameStateMachine::StartGame()
{
	TransitionToState(EGameState::Camp, TEXT("StartGame"));
}

void UGameStateMachine::EnterMapSelection()
{
	TransitionToState(EGameState::MapSelection, TEXT("EnterMapSelection"));
}

void UGameStateMachine::StartCombat(const FString& LevelName)
{
	TransitionToState(EGameState::Combat, LevelName);
}

void UGameStateMachine::EnterEvent(const FString& EventID)
{
	TransitionToState(EGameState::Event, EventID);
}

void UGameStateMachine::EnterShop()
{
	TransitionToState(EGameState::Shop, TEXT("EnterShop"));
}

void UGameStateMachine::EnterSkillTree()
{
	TransitionToState(EGameState::SkillTree, TEXT("EnterSkillTree"));
}

void UGameStateMachine::PauseGame()
{
	if (CurrentState != EGameState::Pause)
	{
		TransitionToState(EGameState::Pause, TEXT("PauseGame"));
	}
}

void UGameStateMachine::ResumeGame()
{
	if (CurrentState == EGameState::Pause && PreviousState != EGameState::None)
	{
		TransitionToState(PreviousState, TEXT("ResumeGame"));
	}
}

void UGameStateMachine::CombatSettlement(bool bVictory)
{
	FString Data = bVictory ? TEXT("Victory") : TEXT("Defeat");
	TransitionToState(EGameState::Settlement, Data);
}

void UGameStateMachine::ReturnToMainMenu()
{
	TransitionToState(EGameState::MainMenu, TEXT("ReturnToMainMenu"));
}

void UGameStateMachine::GameOver()
{
	TransitionToState(EGameState::GameOver, TEXT("GameOver"));
}

void UGameStateMachine::Update(float DeltaTime)
{
	CurrentWorldTime += DeltaTime;
}

float UGameStateMachine::GetStateDuration() const
{
	return CurrentWorldTime - StateEnterTime;
}

void UGameStateMachine::ExecuteStateTransition(EGameState NewState, const FString& TransitionData)
{
	PreviousState = CurrentState;
	CurrentState = NewState;
	StateEnterTime = CurrentWorldTime;

	FStateTransitionInfo Info;
	Info.PreviousState = PreviousState;
	Info.NewState = CurrentState;
	Info.TransitionData = TransitionData;

	OnGameStateChanged.Broadcast(Info);

	UE_LOG(LogTemp, Log, TEXT("GameStateMachine: %d -> %d"), (int32)PreviousState, (int32)CurrentState);
}
