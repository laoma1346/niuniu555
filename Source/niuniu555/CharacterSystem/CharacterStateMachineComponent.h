#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterStateMachineComponent.generated.h"

// 角色状态机日志
DECLARE_LOG_CATEGORY_EXTERN(LogCharacterStateMachine, Log, All);

/**
 * 角色状态枚举
 * 定义角色在游戏中的所有可能状态
 */
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	None		UMETA(DisplayName = "无"),
	Idle		UMETA(DisplayName = "待机"),
	Move		UMETA(DisplayName = "移动"),
	Attack		UMETA(DisplayName = "攻击"),
	Dodge		UMETA(DisplayName = "闪避"),
	Hit			UMETA(DisplayName = "受击"),
	Stun		UMETA(DisplayName = "眩晕"),
	Dead		UMETA(DisplayName = "死亡"),
	Interact	UMETA(DisplayName = "交互"),
	Skill		UMETA(DisplayName = "技能"),
	MAX			UMETA(Hidden)
};

/**
 * 状态转换规则结构体
 * 定义从某个状态可以转换到哪些状态
 */
USTRUCT(BlueprintType)
struct FStateTransitionRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "状态规则")
	TArray<ECharacterState> AllowedNextStates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "状态规则")
	bool bCanBeInterrupted = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "状态规则")
	float MinStateDuration = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterStateChanged, ECharacterState, OldState, ECharacterState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterStateEnter, ECharacterState, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterStateExit, ECharacterState, State);

/**
 * 角色状态机组件
 * 统一管理角色的所有状态，协调各组件之间的状态切换
 * 
 * 使用说明：
 * 1. 添加到玩家角色蓝图
 * 2. 其他组件（如DodgeComponent、WeaponManager）通过此组件查询/请求状态切换
 * 3. 使用CanPerformAction查询能否执行某动作
 * 4. 使用TryChangeState请求状态切换
 */
UCLASS(ClassGroup = (Character), meta = (BlueprintSpawnableComponent, DisplayName = "角色状态机"))
class NIUNIU555_API UCharacterStateMachineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterStateMachineComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ========== 状态查询 ==========

	/** 获取当前状态 */
	UFUNCTION(BlueprintPure, Category = "状态机")
	ECharacterState GetCurrentState() const { return CurrentState; }

	/** 获取上一状态 */
	UFUNCTION(BlueprintPure, Category = "状态机")
	ECharacterState GetPreviousState() const { return PreviousState; }

	/** 检查是否处于某状态 */
	UFUNCTION(BlueprintPure, Category = "状态机")
	bool IsInState(ECharacterState State) const { return CurrentState == State; }

	/** 检查是否处于可战斗状态 */
	UFUNCTION(BlueprintPure, Category = "状态机")
	bool IsCombatReady() const;

	/** 检查是否处于硬直状态（受击/眩晕） */
	UFUNCTION(BlueprintPure, Category = "状态机")
	bool IsInHitStun() const;

	/** 检查是否处于无敌状态 */
	UFUNCTION(BlueprintPure, Category = "状态机")
	bool IsInvincible() const;

	// ========== 动作可行性查询 ==========

	/** 能否攻击 */
	UFUNCTION(BlueprintPure, Category = "状态机|动作检查")
	bool CanAttack() const;

	/** 能否闪避 */
	UFUNCTION(BlueprintPure, Category = "状态机|动作检查")
	bool CanDodge() const;

	/** 能否移动 */
	UFUNCTION(BlueprintPure, Category = "状态机|动作检查")
	bool CanMove() const;

	/** 能否使用技能 */
	UFUNCTION(BlueprintPure, Category = "状态机|动作检查")
	bool CanUseSkill() const;

	/** 能否交互 */
	UFUNCTION(BlueprintPure, Category = "状态机|动作检查")
	bool CanInteract() const;

	/** 能否切换武器 */
	UFUNCTION(BlueprintPure, Category = "状态机|动作检查")
	bool CanSwitchWeapon() const;

	/** 通用动作检查 */
	UFUNCTION(BlueprintPure, Category = "状态机|动作检查")
	bool CanPerformAction(ECharacterState RequiredState) const;

	// ========== 状态切换 ==========

	/**
	 * 尝试切换到新状态
	 * @param NewState 目标状态
	 * @param bForce 是否强制切换（无视规则）
	 * @return 是否切换成功
	 */
	UFUNCTION(BlueprintCallable, Category = "状态机")
	bool TryChangeState(ECharacterState NewState, bool bForce = false);

	/**
	 * 强制切换状态（不检查规则）
	 * @param NewState 目标状态
	 */
	UFUNCTION(BlueprintCallable, Category = "状态机")
	void ForceChangeState(ECharacterState NewState);

	/** 返回上一状态 */
	UFUNCTION(BlueprintCallable, Category = "状态机")
	void RevertToPreviousState();

	/** 重置为Idle状态 */
	UFUNCTION(BlueprintCallable, Category = "状态机")
	void ResetToIdle();

	// ========== 状态持续时间 ==========

	/** 获取当前状态已持续时间 */
	UFUNCTION(BlueprintPure, Category = "状态机")
	float GetStateDuration() const { return CurrentStateDuration; }

	/** 检查当前状态是否持续超过指定时间 */
	UFUNCTION(BlueprintPure, Category = "状态机")
	bool IsStateDurationGreaterThan(float Time) const { return CurrentStateDuration >= Time; }

	// ========== 事件委托 ==========

	/** 状态变化事件（OldState, NewState） */
	UPROPERTY(BlueprintAssignable, Category = "状态机|事件")
	FOnCharacterStateChanged OnStateChanged;

	/** 进入状态事件 */
	UPROPERTY(BlueprintAssignable, Category = "状态机|事件")
	FOnCharacterStateEnter OnStateEnter;

	/** 退出状态事件 */
	UPROPERTY(BlueprintAssignable, Category = "状态机|事件")
	FOnCharacterStateExit OnStateExit;

	// ========== 配置 ==========

	/** 是否启用状态机日志（调试用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "配置")
	bool bEnableDebugLog = false;

	/** 是否显示调试信息到屏幕 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "配置")
	bool bShowDebugInfo = false;

protected:
	/** 当前状态 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "状态")
	ECharacterState CurrentState = ECharacterState::None;

	/** 上一状态 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "状态")
	ECharacterState PreviousState = ECharacterState::None;

	/** 当前状态开始时间 */
	float StateStartTime = 0.0f;

	/** 当前状态持续时间 */
	float CurrentStateDuration = 0.0f;

	/** 状态进入计数（用于调试） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "调试")
	TMap<ECharacterState, int32> StateEnterCount;

protected:
	/** 检查状态转换是否合法 */
	bool CanTransitionTo(ECharacterState NewState) const;

	/** 执行状态进入逻辑 */
	void OnEnterState(ECharacterState NewState);

	/** 执行状态退出逻辑 */
	void OnExitState(ECharacterState OldState);

	/** 更新调试显示 */
	void UpdateDebugDisplay();

	/** 打印状态机日志 */
	void LogStateMachine(const FString& Message);

public:
	// ========== 静态工具函数 ==========

	/** 获取状态名称（调试用） */
	UFUNCTION(BlueprintPure, Category = "状态机|工具")
	static FString GetStateName(ECharacterState State);
};
