// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameStateMachine.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
	None			UMETA(DisplayName = "无"),
	MainMenu		UMETA(DisplayName = "主菜单"),
	Camp			UMETA(DisplayName = "营地准备"),
	MapSelection	UMETA(DisplayName = "地图选择"),
	Loading			UMETA(DisplayName = "加载中"),
	Combat			UMETA(DisplayName = "战斗中"),
	Event			UMETA(DisplayName = "事件"),
	Shop			UMETA(DisplayName = "商店"),
	SkillTree		UMETA(DisplayName = "技能树"),
	Pause			UMETA(DisplayName = "暂停"),
	Settlement		UMETA(DisplayName = "结算"),
	GameOver		UMETA(DisplayName = "游戏结束")
};

UENUM(BlueprintType)
enum class EGameFlowState : uint8
{
	OutOfGame		UMETA(DisplayName = "游戏外"),
	Preparation		UMETA(DisplayName = "准备阶段"),
	Adventure		UMETA(DisplayName = "冒险阶段"),
	Combat			UMETA(DisplayName = "战斗阶段"),
	Settlement		UMETA(DisplayName = "结算阶段")
};

USTRUCT(BlueprintType)
struct FStateTransitionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EGameState PreviousState = EGameState::None;

	UPROPERTY(BlueprintReadOnly)
	EGameState NewState = EGameState::None;

	UPROPERTY(BlueprintReadOnly)
	FString TransitionData;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, const FStateTransitionInfo&, TransitionInfo);

UCLASS()
class NIUNIU555_API UGameStateMachine : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "GameFlow")
	EGameState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "GameFlow")
	EGameState GetPreviousState() const { return PreviousState; }

	UFUNCTION(BlueprintPure, Category = "GameFlow")
	EGameFlowState GetCurrentFlowState() const;

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	bool TransitionToState(EGameState NewState, const FString& TransitionData = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void EnterMapSelection();

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void StartCombat(const FString& LevelName);

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void EnterEvent(const FString& EventID);

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void EnterShop();

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void EnterSkillTree();

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void PauseGame();

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void ResumeGame();

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void CombatSettlement(bool bVictory);

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void ReturnToMainMenu();

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void GameOver();

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void Update(float DeltaTime);

	// 获取当前状态持续时间
	UFUNCTION(BlueprintPure, Category = "GameFlow")
	float GetStateDuration() const;

	UPROPERTY(BlueprintAssignable)
	FOnGameStateChanged OnGameStateChanged;

protected:
	UPROPERTY()
	EGameState CurrentState = EGameState::None;

	UPROPERTY()
	EGameState PreviousState = EGameState::None;

	UPROPERTY()
	float StateEnterTime = 0.0f;

	UPROPERTY()
	float CurrentWorldTime = 0.0f;

	void ExecuteStateTransition(EGameState NewState, const FString& TransitionData);
};
