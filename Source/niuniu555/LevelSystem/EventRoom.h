#pragma once

#include "CoreMinimal.h"
#include "LevelTypes.h"
#include "RoomBase.h"
#include "EventRoom.generated.h"

// 事件房间专用委托（LevelTypes.h中没有，在此定义）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventActivated, const FEventConfig&, EventConfig);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventCompleted, bool, bSuccess);

/**
 * 事件房间
 * 随机事件、选择、赌博等
 */
UCLASS(Blueprintable)
class NIUNIU555_API AEventRoom : public ARoomBase
{
	GENERATED_BODY()

public:
	AEventRoom();

protected:
	virtual void BeginPlay() override;
	virtual void OnEnterRoom_Implementation() override;
	virtual void OnCompleteRoom_Implementation() override;

public:
	// ========== 事件配置 ==========

	/** 预设事件列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "事件配置")
	TArray<FEventConfig> AvailableEvents;

	/** 是否随机选择事件 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "事件配置")
	bool bRandomEvent = true;

	/** 指定事件索引（非随机时使用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "事件配置", meta = (EditCondition = "!bRandomEvent"))
	int32 FixedEventIndex = 0;

	// ========== 当前事件 ==========

	/** 当前事件配置 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "当前事件")
	FEventConfig CurrentEvent;

	/** 当前事件索引 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "当前事件")
	int32 CurrentEventIndex = -1;

	/** 事件是否已激活 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "当前事件")
	bool bEventActive = false;

	/** 已选择的选项索引 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "当前事件")
	int32 SelectedOptionIndex = -1;

	// ========== 事件处理 ==========

	/**
	 * 选择并激活事件
	 */
	UFUNCTION(BlueprintCallable, Category = "事件处理")
	void SelectEvent();

	/**
	 * 选择选项
	 * @param OptionIndex 选项索引
	 */
	UFUNCTION(BlueprintCallable, Category = "事件处理")
	void ChooseOption(int32 OptionIndex);

	/**
	 * 跳过事件
	 */
	UFUNCTION(BlueprintCallable, Category = "事件处理")
	void SkipEvent();

	/**
	 * 检查是否可以跳过
	 */
	UFUNCTION(BlueprintPure, Category = "事件处理")
	bool CanSkip() const;

	/**
	 * 检查选项是否可用（资源足够）
	 */
	UFUNCTION(BlueprintPure, Category = "事件处理")
	bool IsOptionAvailable(int32 OptionIndex) const;

	/**
	 * 输出当前事件信息到日志
	 */
	UFUNCTION(BlueprintCallable, Category = "事件处理")
	void LogCurrentEventInfo() const;

	// ========== 事件类型处理 ==========

	/** 处理祝福事件 */
	UFUNCTION(BlueprintImplementableEvent, Category = "事件处理")
	void HandleBlessingEvent(const FEventOption& Option);

	/** 处理诅咒事件 */
	UFUNCTION(BlueprintImplementableEvent, Category = "事件处理")
	void HandleCurseEvent(const FEventOption& Option);

	/** 处理宝藏事件 */
	UFUNCTION(BlueprintImplementableEvent, Category = "事件处理")
	void HandleTreasureEvent(const FEventOption& Option);

	/** 处理商店事件 */
	UFUNCTION(BlueprintImplementableEvent, Category = "事件处理")
	void HandleShopEvent(const FEventOption& Option);

	/** 处理赌博事件 */
	UFUNCTION(BlueprintImplementableEvent, Category = "事件处理")
	void HandleGambleEvent(const FEventOption& Option);

	// ========== 奖励/惩罚 ==========

	/** 应用奖励 */
	UFUNCTION(BlueprintCallable, Category = "奖励")
	void ApplyReward(const FEventOption& Option);

	/** 应用惩罚 */
	UFUNCTION(BlueprintCallable, Category = "奖励")
	void ApplyPenalty(const FEventOption& Option);

	/** 给予临时Buff */
	UFUNCTION(BlueprintCallable, Category = "奖励")
	void GrantTemporaryBuff(const FName& BuffName, float Duration);

	// ========== 事件委托 ==========

	/** 事件激活事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnEventActivated OnEventActivated;

	/** 选项选择事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnEventOptionSelected OnOptionSelected;

	/** 事件完成事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnEventCompleted OnEventCompleted;

protected:
	/** 创建默认事件 */
	void CreateDefaultEvents();

	/** 获取随机事件索引 */
	int32 GetRandomEventIndex() const;

	/** 执行选项效果 */
	void ExecuteOptionEffect(int32 OptionIndex);

	/** 应用生命值惩罚 */
	void ApplyHealthPenalty(float Percent);

	// ========== 玩家数据接口（临时实现） ==========

	/** 获取玩家金币 */
	int32 GetPlayerGold() const;

	/** 获取玩家神格碎片 */
	int32 GetPlayerFragments() const;

	/** 检查金币是否足够 */
	bool HasEnoughGold(int32 Amount) const;

	/** 检查神格碎片是否足够 */
	bool HasEnoughFragments(int32 Amount) const;

	/** 扣除玩家金币 */
	bool DeductPlayerGold(int32 Amount);

	/** 扣除玩家神格碎片 */
	bool DeductPlayerFragments(int32 Amount);

	/** 给予玩家金币 */
	void GivePlayerGold(int32 Amount);
};
