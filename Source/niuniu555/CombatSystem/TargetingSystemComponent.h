#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "TargetingSystemComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTargetingSystem, Log, All);

// 前向声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorChanged, AActor*, Actor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoolValueChanged, bool, bValue);

/**
 * 锁定目标信息结构体
 */
USTRUCT(BlueprintType)
struct FLockOnTargetInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "锁定")
	AActor* TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "锁定")
	float Distance = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "锁定")
	float Angle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "锁定")
	bool bIsVisible = false;

	bool IsValid() const { return TargetActor != nullptr; }
};

/**
 * 准星与锁定系统组件
 * 
 * 功能：
 * 1. 软锁定 - 摄像机自动向最近敌人吸附
 * 2. 硬锁定 - 手动切换目标，锁定图标UI
 * 3. 准星扩散 - 移动/射击时准星UI动态变化
 * 4. 目标筛选 - 基于距离、角度、可见性
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent, DisplayName = "准星与锁定系统"))
class NIUNIU555_API UTargetingSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTargetingSystemComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ========== 硬锁定功能 ==========

	/** 切换锁定状态（锁定/解锁） */
	UFUNCTION(BlueprintCallable, Category = "锁定")
	void ToggleLockOn();

	/** 尝试锁定目标 */
	UFUNCTION(BlueprintCallable, Category = "锁定")
	bool TryLockOn();

	/** 解除锁定 */
	UFUNCTION(BlueprintCallable, Category = "锁定")
	void Unlock();

	/** 切换到下一个锁定目标 */
	UFUNCTION(BlueprintCallable, Category = "锁定")
	void SwitchToNextTarget();

	/** 切换到上一个锁定目标 */
	UFUNCTION(BlueprintCallable, Category = "锁定")
	void SwitchToPreviousTarget();

	/** 是否处于锁定状态 */
	UFUNCTION(BlueprintPure, Category = "锁定")
	bool IsLockedOn() const { return CurrentTarget.IsValid(); }

	/** 获取当前锁定目标 */
	UFUNCTION(BlueprintPure, Category = "锁定")
	AActor* GetCurrentTarget() const { return CurrentTarget.TargetActor; }

	/** 获取锁定目标位置（带高度偏移） */
	UFUNCTION(BlueprintPure, Category = "锁定")
	FVector GetTargetLockLocation() const;

	// ========== 软锁定功能 ==========

	/** 获取最佳软锁定目标（用于摄像机自动吸附） */
	UFUNCTION(BlueprintPure, Category = "软锁定")
	AActor* GetSoftLockTarget() const;

	/** 检查是否有可软锁定的目标 */
	UFUNCTION(BlueprintPure, Category = "软锁定")
	bool HasSoftLockTarget() const;

	// ========== 准星功能 ==========

	/** 获取当前准星扩散值（0.0-1.0） */
	UFUNCTION(BlueprintPure, Category = "准星")
	float GetCrosshairSpread() const { return CurrentCrosshairSpread; }

	/** 添加准星扩散（射击时调用） */
	UFUNCTION(BlueprintCallable, Category = "准星")
	void AddCrosshairSpread(float Amount);

	/** 重置准星扩散 */
	UFUNCTION(BlueprintCallable, Category = "准星")
	void ResetCrosshairSpread();

	// ========== 目标查询 ==========

	/** 获取所有可锁定目标 */
	UFUNCTION(BlueprintCallable, Category = "目标查询")
	TArray<AActor*> GetAllLockableTargets() const;

	/** 检查目标是否可以被锁定 */
	UFUNCTION(BlueprintPure, Category = "目标查询")
	bool CanLockOnTarget(AActor* Target) const;

	/** 获取目标相对于玩家的角度 */
	UFUNCTION(BlueprintPure, Category = "目标查询")
	float GetTargetAngle(AActor* Target) const;

	/** 获取目标距离 */
	UFUNCTION(BlueprintPure, Category = "目标查询")
	float GetTargetDistance(AActor* Target) const;

	/** 检查目标是否在视野内 */
	UFUNCTION(BlueprintPure, Category = "目标查询")
	bool IsTargetInView(AActor* Target) const;

	// ========== 配置 ==========

	/** 最大锁定距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "配置|锁定")
	float MaxLockOnDistance = 1500.0f;

	/** 最大锁定角度（相对于摄像机正前方） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "配置|锁定")
	float MaxLockOnAngle = 45.0f;

	/** 锁定切换角度阈值（左右切换时） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "配置|锁定")
	float SwitchTargetAngleThreshold = 30.0f;

	/** 软锁定检测角度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "配置|软锁定")
	float SoftLockAngle = 15.0f;

	/** 目标高度偏移（锁定目标中心点上方） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "配置|锁定")
	float TargetHeightOffset = 100.0f;

	/** 可锁定目标的标签 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "配置|筛选")
	FName LockableTag = FName("Enemy");

	/** 准星扩散恢复速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "配置|准星")
	float CrosshairRecoverySpeed = 2.0f;

	/** 移动时的基础准星扩散 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "配置|准星")
	float MovementSpread = 0.2f;

	/** 是否显示调试信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "调试")
	bool bShowDebug = false;

	// ========== 事件 ==========

	/** 锁定目标变化事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnActorChanged OnTargetChanged;

	/** 锁定状态变化事件（true=锁定, false=解锁） */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnBoolValueChanged OnLockOnStateChanged;

protected:
	/** 当前锁定的目标 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "状态")
	FLockOnTargetInfo CurrentTarget;

	/** 当前准星扩散值 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "准星")
	float CurrentCrosshairSpread = 0.0f;

	/** 摄像机组件缓存 */
	UPROPERTY()
	UCameraComponent* CameraComponent = nullptr;

	/** 控制器缓存 */
	UPROPERTY()
	AController* OwnerController = nullptr;

protected:
	/** 查找最佳锁定目标 */
	AActor* FindBestLockOnTarget() const;

	/** 查找左侧/右侧的下一个目标 */
	AActor* FindTargetInDirection(bool bRight) const;

	/** 更新准星扩散 */
	void UpdateCrosshairSpread(float DeltaTime);

	/** 检测目标是否可见（射线检测） */
	bool CheckTargetVisibility(AActor* Target) const;

	/** 更新锁定状态 */
	void UpdateLockOn();

	/** 获取玩家眼睛位置 */
	FVector GetEyeLocation() const;

	/** 获取玩家视线方向 */
	FVector GetViewDirection() const;

	/** 绘制调试信息 */
	void DrawDebugInfo();

	/** 按角度排序目标 */
	TArray<FLockOnTargetInfo> GetSortedTargetsByAngle() const;
};
