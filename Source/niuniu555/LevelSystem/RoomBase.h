#pragma once

#include "CoreMinimal.h"
#include "LevelTypes.h"
#include "GameFramework/Actor.h"
#include "RoomBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRoomSystem, Log, All);

// 房间事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomStateChangedDelegate, ARoomBase*, Room);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomDestroyedDelegate, ARoomBase*, Room);

class URoomManager;

/**
 * 房间基类
 * 所有房间类型的父类，提供基础功能
 */
UCLASS(Abstract, Blueprintable)
class NIUNIU555_API ARoomBase : public AActor
{
	GENERATED_BODY()

public:
	ARoomBase();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 延迟注册定时器句柄 */
	FTimerHandle RegisterDelayHandle;

	/** 执行延迟注册（成员函数绑定，非Lambda） */
	void ExecuteDelayedRegistration();

public:
	// ========== 房间基本信息 ==========

	/** 房间ID（地图中的唯一标识） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "房间信息")
	int32 RoomID = -1;

	/** 房间类型 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "房间信息")
	ERoomType RoomType = ERoomType::None;

	/** 房间名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "房间信息")
	FString RoomName;

	/** 房间描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "房间信息", meta = (MultiLine = true))
	FString RoomDescription;

	/** 房间状态 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "房间状态")
	ERoomState RoomState = ERoomState::NotEntered;

	// ========== 场景配置 ==========

	/** 场景关卡名称（异步加载用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "场景")
	FName LevelName;

	/** 背景音乐 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "场景")
	USoundBase* BackgroundMusic;

	/** 环境特效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "场景")
	UParticleSystem* AmbientEffect;

	/** 房间预览图片 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "场景")
	UTexture2D* PreviewImage;

	// ========== 房间管理 ==========

	/** 房间管理器引用 */
	UPROPERTY()
	URoomManager* RoomManagerPtr = nullptr;

	/** 连接的房间ID列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "连接")
	TArray<int32> ConnectedRoomIDs;

	// ========== 房间生命周期 ==========

	/**
	 * 进入房间
	 * 玩家进入此房间时调用
	 */
	UFUNCTION(BlueprintCallable, Category = "房间生命周期")
	virtual void EnterRoom();

	/**
	 * 完成房间
	 * 房间目标达成时调用
	 */
	UFUNCTION(BlueprintCallable, Category = "房间生命周期")
	virtual void CompleteRoom();

	/**
	 * 跳过房间
	 * 玩家选择跳过此房间时调用
	 */
	UFUNCTION(BlueprintCallable, Category = "房间生命周期")
	virtual void SkipRoom();

	/**
	 * 重置房间
	 * 重新开始此房间时调用
	 */
	UFUNCTION(BlueprintCallable, Category = "房间生命周期")
	virtual void ResetRoom();

	/**
	 * 检查房间是否已完成
	 */
	UFUNCTION(BlueprintPure, Category = "房间生命周期")
	bool IsCompleted() const { return RoomState == ERoomState::Completed; }

	/**
	 * 检查房间是否可以进入
	 */
	UFUNCTION(BlueprintPure, Category = "房间生命周期")
	bool CanEnter() const;

	// ========== 事件委托 ==========

	/** 房间进入事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnRoomStateChangedDelegate OnRoomEntered;

	/** 房间完成事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnRoomStateChangedDelegate OnRoomCompleted;

	/** 房间销毁事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnRoomDestroyedDelegate OnRoomDestroyed;

	/** 房间跳过事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnRoomStateChangedDelegate OnRoomSkipped;

protected:
	/** 房间进入实现（蓝图可覆盖） */
	UFUNCTION(BlueprintNativeEvent, Category = "房间实现")
	void OnEnterRoom();
	virtual void OnEnterRoom_Implementation();

	/** 房间完成实现（蓝图可覆盖） */
	UFUNCTION(BlueprintNativeEvent, Category = "房间实现")
	void OnCompleteRoom();
	virtual void OnCompleteRoom_Implementation();

	/** 房间跳过实现（蓝图可覆盖） */
	UFUNCTION(BlueprintNativeEvent, Category = "房间实现")
	void OnSkipRoom();
	virtual void OnSkipRoom_Implementation();

public:
	// ========== 工具函数 ==========

	/** 获取房间类型名称 */
	UFUNCTION(BlueprintPure, Category = "工具")
	static FString GetRoomTypeName(ERoomType Type);

	/** 获取房间状态名称 */
	UFUNCTION(BlueprintPure, Category = "工具")
	static FString GetRoomStateName(ERoomState State);

	/** 获取房间颜色（用于地图显示） */
	UFUNCTION(BlueprintPure, Category = "工具")
	static FLinearColor GetRoomTypeColor(ERoomType Type);

protected:
	/** 打印房间日志 */
	void LogRoom(const FString& Message);
};
