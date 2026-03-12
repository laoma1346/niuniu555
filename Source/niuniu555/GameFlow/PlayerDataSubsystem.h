#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerDataSubsystem.generated.h"

// 玩家数据变化事件委托（使用唯一命名避免冲突）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerGoldChanged, int32, NewAmount, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerFragmentsChanged, int32, NewAmount, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerHealthChanged, float, HealthPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerManaChanged, float, ManaPercent);

/**
 * 玩家数据子系统
 * 统一管理玩家金币、神格碎片、属性等数据
 * 供LevelSystem的各个房间（Event/Shop/Skill）统一访问
 */
UCLASS()
class NIUNIU555_API UPlayerDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 获取玩家数据子系统 */
	UFUNCTION(BlueprintPure, Category = "玩家数据", meta = (WorldContext = "WorldContextObject"))
	static UPlayerDataSubsystem* Get(const UObject* WorldContextObject);

public:
	// ========== 金币操作 ==========

	/** 获取当前金币 */
	UFUNCTION(BlueprintPure, Category = "玩家数据|金币")
	int32 GetGold() const { return CurrentGold; }

	/** 检查金币是否足够 */
	UFUNCTION(BlueprintPure, Category = "玩家数据|金币")
	bool HasEnoughGold(int32 Amount) const { return CurrentGold >= Amount; }

	/** 添加金币（可为负） */
	UFUNCTION(BlueprintCallable, Category = "玩家数据|金币")
	void AddGold(int32 Amount);

	/** 设置金币 */
	UFUNCTION(BlueprintCallable, Category = "玩家数据|金币")
	void SetGold(int32 Amount);

	/** 尝试扣除金币 */
	UFUNCTION(BlueprintCallable, Category = "玩家数据|金币")
	bool TryDeductGold(int32 Amount);

	// ========== 神格碎片操作 ==========

	/** 获取当前神格碎片 */
	UFUNCTION(BlueprintPure, Category = "玩家数据|神格碎片")
	int32 GetFragments() const { return CurrentFragments; }

	/** 检查神格碎片是否足够 */
	UFUNCTION(BlueprintPure, Category = "玩家数据|神格碎片")
	bool HasEnoughFragments(int32 Amount) const { return CurrentFragments >= Amount; }

	/** 添加神格碎片（可为负） */
	UFUNCTION(BlueprintCallable, Category = "玩家数据|神格碎片")
	void AddFragments(int32 Amount);

	/** 设置神格碎片 */
	UFUNCTION(BlueprintCallable, Category = "玩家数据|神格碎片")
	void SetFragments(int32 Amount);

	/** 尝试扣除神格碎片 */
	UFUNCTION(BlueprintCallable, Category = "玩家数据|神格碎片")
	bool TryDeductFragments(int32 Amount);

	// ========== 生命值操作 ==========

	/** 获取生命值百分比 */
	UFUNCTION(BlueprintPure, Category = "玩家数据|生命值")
	float GetHealthPercent() const { return CurrentHealthPercent; }

	/** 设置生命值百分比 */
	UFUNCTION(BlueprintCallable, Category = "玩家数据|生命值")
	void SetHealthPercent(float Percent);

	/** 修改生命值（百分比） */
	UFUNCTION(BlueprintCallable, Category = "玩家数据|生命值")
	void ModifyHealth(float DeltaPercent);

	// ========== 法力值操作 ==========

	/** 获取法力值百分比 */
	UFUNCTION(BlueprintPure, Category = "玩家数据|法力值")
	float GetManaPercent() const { return CurrentManaPercent; }

	/** 设置法力值百分比 */
	UFUNCTION(BlueprintCallable, Category = "玩家数据|法力值")
	void SetManaPercent(float Percent);

	// ========== 跨房间数据保持 ==========

	/** 保存当前玩家数据（房间切换前调用） */
	UFUNCTION(BlueprintCallable, Category = "玩家数据|存档")
	void SavePlayerData();

	/** 恢复玩家数据（房间切换后调用） */
	UFUNCTION(BlueprintCallable, Category = "玩家数据|存档")
	void RestorePlayerData();

	/** 重置所有数据（新游戏/章节重置） */
	UFUNCTION(BlueprintCallable, Category = "玩家数据|存档")
	void ResetAllData();

public:
	// ========== 事件委托 ==========

	/** 金币变化事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnPlayerGoldChanged OnGoldChanged;

	/** 神格碎片变化事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnPlayerFragmentsChanged OnFragmentsChanged;

	/** 生命值变化事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnPlayerHealthChanged OnHealthChanged;

	/** 法力值变化事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnPlayerManaChanged OnManaChanged;

private:
	/** 当前金币 */
	UPROPERTY()
	int32 CurrentGold = 0;

	/** 当前神格碎片 */
	UPROPERTY()
	int32 CurrentFragments = 0;

	/** 当前生命值百分比（0-1） */
	UPROPERTY()
	float CurrentHealthPercent = 1.0f;

	/** 当前法力值百分比（0-1） */
	UPROPERTY()
	float CurrentManaPercent = 1.0f;

	// ========== 存档数据 ==========

	/** 保存的金币 */
	int32 SavedGold = 0;

	/** 保存的神格碎片 */
	int32 SavedFragments = 0;

	/** 保存的生命值百分比 */
	float SavedHealthPercent = 1.0f;

	/** 保存的法力值百分比 */
	float SavedManaPercent = 1.0f;
};
