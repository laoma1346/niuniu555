#pragma once

#include "CoreMinimal.h"
#include "LevelTypes.h"
#include "RoomBase.h"
#include "CombatRoom.generated.h"

class AEnemySpawner;

// 战斗房间专用事件委托（使用独特名称避免冲突）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatWaveStarted, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatWaveCompleted, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatEnemyKilled, AActor*, Enemy, int32, RemainingCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatObjectiveUpdated, float, ProgressPercent);

/**
 * 战斗房间
 * 玩家需要击败所有敌人或达到特定目标才能通关
 */
UCLASS(Blueprintable)
class NIUNIU555_API ACombatRoom : public ARoomBase
{
	GENERATED_BODY()

public:
	ACombatRoom();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnEnterRoom_Implementation() override;
	virtual void OnCompleteRoom_Implementation() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// ========== 房间配置 ==========

	/** 房间难度等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "战斗配置")
	int32 DifficultyLevel = 1;

	/** 房间类型细分 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "战斗配置")
	ECombatRoomType CombatType = ECombatRoomType::Normal;

	/** 波次配置列表（普通/精英房用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "战斗配置", meta = (EditCondition = "CombatType != ECombatRoomType::Boss"))
	TArray<FCombatWaveConfig> WaveConfigs;

	/** BOSS配置（BOSS房用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "战斗配置", meta = (EditCondition = "CombatType == ECombatRoomType::Boss"))
	FBossConfig BossConfig;

	// ========== 运行时状态 ==========

	/** 当前波次 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "战斗状态")
	int32 CurrentWave = 0;

	/** 总波次数 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "战斗状态")
	int32 TotalWaves = 0;

	/** 当前存活敌人数量 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "战斗状态")
	int32 AliveEnemyCount = 0;

	/** 房间是否已开始生成敌人 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "战斗状态")
	bool bWaveInProgress = false;

	/** 波次敌人生成是否已完成（防止在敌人生成前判定波次完成） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "战斗状态")
	bool bWaveSpawnComplete = false;

	// ========== 波次管理 ==========

	/** 开始波次 */
	UFUNCTION(BlueprintCallable, Category = "波次管理")
	void StartWave(int32 WaveIndex);

	/** 开始下一波 */
	UFUNCTION(BlueprintCallable, Category = "波次管理")
	void StartNextWave();

	/** 检查波次是否完成 */
	UFUNCTION(BlueprintCallable, Category = "波次管理")
	void CheckWaveComplete();

	/** 所有波次是否完成 */
	UFUNCTION(BlueprintPure, Category = "波次管理")
	bool AreAllWavesComplete() const;

	// ========== 敌人管理 ==========

	/** 注册生成的敌人 */
	UFUNCTION(BlueprintCallable, Category = "敌人管理")
	void RegisterSpawnedEnemy(AActor* Enemy);

	/** 注销敌人（死亡时调用） */
	UFUNCTION(BlueprintCallable, Category = "敌人管理")
	void UnregisterEnemy(AActor* Enemy);

protected:
	/** 敌人死亡回调（供委托绑定） */
	UFUNCTION()
	void OnEnemyDeath();

	/** 处理延迟生成（安全版本，非Lambda） */
	void ProcessDelayedSpawn();

	/** 当前处理中的生成组索引 */
	int32 CurrentSpawnGroupIndex = 0;

	/** 当前波次配置的临时存储（非UPROPERTY，内部不包含UObject指针） */
	TArray<FCombatWaveConfig> ActiveWaveConfigs;

public:
	/** 生成器列表 */
	UPROPERTY()
	TArray<AEnemySpawner*> EnemySpawners;

	/** 存活敌人列表 */
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> AliveEnemies;

	/** 已绑定死亡事件的敌人列表（用于EndPlay解绑） */
	TArray<TWeakObjectPtr<AEnemyBase>> BoundEnemies;

	/** 标记是否正在清理中（防止重复解绑） */
	bool bIsCleaningUp = false;

	/** 波次生成定时器句柄（用于清理） - 非UPROPERTY，FTimerHandle不是UObject */
	TArray<FTimerHandle> SpawnTimerHandles;

	/** 波次延迟定时器句柄 */
    FTimerHandle WaveDelayTimerHandle;
    
    /** 查找敌人生成器定时器句柄 */
    FTimerHandle FindSpawnersTimerHandle;
    
    // ========== 房间目标 ==========

	/** 房间目标类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "房间目标")
	ECombatObjectiveType ObjectiveType = ECombatObjectiveType::KillAll;

	/** 目标数值（如存活时间/击杀数） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "房间目标")
	float ObjectiveValue = 0.0f;

	/** 当前进度 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "房间目标")
	float CurrentObjectiveProgress = 0.0f;

	/** 更新目标进度 */
	UFUNCTION(BlueprintCallable, Category = "房间目标")
	void UpdateObjectiveProgress(float Delta);

	/** 检查目标是否达成 */
	UFUNCTION(BlueprintCallable, Category = "房间目标")
	bool CheckObjectiveComplete() const;

	// ========== 房间奖励 ==========

	/** 基础金币奖励 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "奖励")
	int32 BaseGoldReward = 100;

	/** 基础经验奖励 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "奖励")
	int32 BaseExpReward = 50;

	/** 额外奖励物品 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "奖励")
	TArray<TSubclassOf<class AActor>> BonusRewards;

	/** 发放奖励 */
	UFUNCTION(BlueprintCallable, Category = "奖励")
	void GiveRewards();

	// ========== 事件委托 ==========

	/** 波次开始事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnCombatWaveStarted OnWaveStarted;

	/** 波次完成事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnCombatWaveCompleted OnWaveCompleted;

	/** 敌人死亡事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnCombatEnemyKilled OnEnemyKilled;

	/** 目标进度更新事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnCombatObjectiveUpdated OnObjectiveProgressUpdated;

public:
	/** 手动注册敌人生成器（用于蓝图或延迟注册） */
	UFUNCTION(BlueprintCallable, Category = "战斗配置")
	void RegisterEnemySpawner(AEnemySpawner* Spawner);

	/** 清除所有注册的生成器 */
	UFUNCTION(BlueprintCallable, Category = "战斗配置")
	void ClearEnemySpawners();

protected:
	/** 生成BOSS */
	void SpawnBoss();

	/** 生成敌人生成组 */
	void SpawnEnemyGroup(const FEnemySpawnGroup& Group);

	/** 查找并缓存生成器 */
	void FindEnemySpawners();

	/** 波次完成检查定时器 */
	FTimerHandle WaveCheckTimerHandle;
};
