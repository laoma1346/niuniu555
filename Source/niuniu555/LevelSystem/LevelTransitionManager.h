#pragma once

#include "CoreMinimal.h"
#include "LevelTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelTransitionManager.generated.h"

class ARoomBase;

// 关卡过渡事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransitionStartedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransitionFinishedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadingProgressUpdatedDelegate, float, ProgressPercent);

/**
 * 关卡过渡管理器
 * 负责处理房间/关卡之间的加载和过渡效果
 */
UCLASS()
class NIUNIU555_API ULevelTransitionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 获取过渡管理器 */
	UFUNCTION(BlueprintPure, Category = "关卡过渡", meta = (WorldContext = "WorldContextObject"))
	static ULevelTransitionManager* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	// ========== 关卡加载 ==========

	/**
	 * 异步加载房间关卡
	 * @param TargetRoom 目标房间
	 * @param bShowLoadingScreen 是否显示加载界面
	 */
	UFUNCTION(BlueprintCallable, Category = "关卡加载")
	void LoadRoomLevel(ARoomBase* TargetRoom, bool bShowLoadingScreen = true);

	/**
	 * 检查是否正在加载中
	 */
	UFUNCTION(BlueprintPure, Category = "关卡加载")
	bool IsLoading() const { return bIsLoading; }

	/**
	 * 获取加载进度（0-1）
	 */
	UFUNCTION(BlueprintPure, Category = "关卡加载")
	float GetLoadingProgress() const { return LoadingProgress; }

	// ========== 过渡效果 ==========

	/**
	 * 播放过渡效果（黑屏/淡入淡出）
	 * @param bFadeToBlack true=淡出到黑，false=从黑淡入
	 * @param Duration 过渡时间
	 */
	UFUNCTION(BlueprintCallable, Category = "过渡效果")
	void PlayTransition(bool bFadeToBlack, float Duration = 1.0f);

	/**
	 * 执行完整的房间切换流程
	 */
	UFUNCTION(BlueprintCallable, Category = "房间切换")
	void PerformRoomTransition(ARoomBase* FromRoom, ARoomBase* ToRoom);

	// ========== 加载配置 ==========

	/** 加载界面Widget类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "配置")
	TSubclassOf<class UUserWidget> LoadingScreenWidgetClass;

	/** 默认过渡时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "配置")
	float DefaultTransitionDuration = 1.0f;

	/** 是否使用流媒体加载 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "配置")
	bool bUseStreaming = true;

	// ========== 跨房间数据保持 ==========

	/**
	 * 保存当前房间的玩家数据
	 */
	UFUNCTION(BlueprintCallable, Category = "数据保持")
	void SavePlayerState();

	/**
	 * 恢复玩家数据到新房间
	 */
	UFUNCTION(BlueprintCallable, Category = "数据保持")
	void RestorePlayerState();

	/**
	 * 获取保存的玩家生命值百分比
	 */
	UFUNCTION(BlueprintPure, Category = "数据保持")
	float GetSavedHealthPercent() const { return SavedHealthPercent; }

	/**
	 * 获取保存的玩家金币数
	 */
	UFUNCTION(BlueprintPure, Category = "数据保持")
	int32 GetSavedGold() const { return SavedGold; }

	/**
	 * 获取保存的神格碎片数
	 */
	UFUNCTION(BlueprintPure, Category = "数据保持")
	int32 GetSavedFragments() const { return SavedFragments; }

protected:
	/** 是否正在加载 */
	bool bIsLoading = false;

	/** 加载进度 */
	float LoadingProgress = 0.0f;

	/** 当前加载的房间 */
	UPROPERTY()
	ARoomBase* TargetRoom = nullptr;

	/** 过渡期间保存的持续时间 */
	float SavedTransitionDuration = 0.0f;

	/** 执行延迟加载（成员函数绑定，非Lambda） */
	void ExecuteDelayedLoad();

	/** 执行淡出完成回调（成员函数绑定，非Lambda） */
	void ExecuteFadeComplete();

	/** 淡出定时器句柄 */
	FTimerHandle FadeTimerHandle;

	/** 加载界面实例 */
	UPROPERTY()
	UUserWidget* LoadingScreenWidget = nullptr;

	/** 保存的玩家数据 */
	float SavedHealthPercent = 1.0f;
	float SavedManaPercent = 1.0f;
	int32 SavedGold = 0;
	int32 SavedFragments = 0;

	// ========== 内部方法 ==========

	/** 显示加载界面 */
	void ShowLoadingScreen();

	/** 隐藏加载界面 */
	void HideLoadingScreen();

	/** 开始异步加载 */
	void StartAsyncLoading();

	/** 加载完成回调 */
	void OnLevelLoaded();

	/** 检查加载进度 */
	void CheckLoadingProgress();

	/** 加载完成处理 */
	void FinishTransition();

	/** 查找玩家角色 */
	class APawn* FindPlayerCharacter() const;

	/** 定时器 */
	FTimerHandle ProgressTimerHandle;
	FTimerHandle TransitionTimerHandle;
	
	/** 是否正在淡出 */
	bool bIsFading = false;

public:
	// ========== 事件委托 ==========

	/** 过渡开始事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnTransitionStartedDelegate OnTransitionStarted;

	/** 过渡完成事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnTransitionFinishedDelegate OnTransitionFinished;

	/** 加载进度更新事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnLoadingProgressUpdatedDelegate OnLoadingProgressUpdated;
};
