// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraControllerComponent.generated.h"

// 摄像机模式枚举
UENUM(BlueprintType)
enum class ECameraMode : uint8
{
	Free			UMETA(DisplayName = "自由视角"),
	LockOn			UMETA(DisplayName = "锁定视角"),
	Fixed			UMETA(DisplayName = "固定视角")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NIUNIU555_API UCameraControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraControllerComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== 摄像机模式切换 ==========

	// 设置摄像机模式
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(ECameraMode NewMode);

	// 获取当前摄像机模式
	UFUNCTION(BlueprintPure, Category = "Camera")
	ECameraMode GetCameraMode() const { return CurrentMode; }

	// 切换锁定目标
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ToggleLockOn();

	// ========== 锁定目标 ==========

	// 设置锁定目标
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetLockOnTarget(AActor* Target);

	// 清除锁定目标
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ClearLockOnTarget();

	// 获取当前锁定目标
	UFUNCTION(BlueprintPure, Category = "Camera")
	AActor* GetLockOnTarget() const { return LockOnTarget; }

	// 是否处于锁定模式
	UFUNCTION(BlueprintPure, Category = "Camera")
	bool IsLockOn() const { return CurrentMode == ECameraMode::LockOn && LockOnTarget != nullptr; }

	// ========== 摄像机控制 ==========

	// 添加摄像机输入（鼠标/手柄）
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void AddCameraInput(float Yaw, float Pitch);

	// 重置摄像机到默认位置
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ResetCamera();

	// ========== 配置参数 ==========

	// 自由视角旋转速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Free")
	float FreeLookSpeed = 2.0f;

	// 锁定视角旋转速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|LockOn")
	float LockOnRotationSpeed = 5.0f;

	// 锁定视角距离
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|LockOn")
	float LockOnDistance = 300.0f;

	// 摄像机偏移（相对于角色）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Offset")
	FVector CameraOffset = FVector(-300.0f, 0.0f, 100.0f);

	// 最小俯仰角
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Limit")
	float MinPitch = -60.0f;

	// 最大俯仰角
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Limit")
	float MaxPitch = 60.0f;

protected:
	// 当前摄像机模式
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	ECameraMode CurrentMode = ECameraMode::Free;

	// 锁定目标
	UPROPERTY()
	TObjectPtr<AActor> LockOnTarget = nullptr;

	// 当前摄像机Yaw（水平）
	UPROPERTY()
	float CurrentYaw = 0.0f;

	// 当前摄像机Pitch（垂直）
	UPROPERTY()
	float CurrentPitch = 0.0f;

	// 目标Yaw
	UPROPERTY()
	float TargetYaw = 0.0f;

	// 目标Pitch
	UPROPERTY()
	float TargetPitch = 0.0f;

	// 更新自由视角
	void UpdateFreeCamera(float DeltaTime);

	// 更新锁定视角
	void UpdateLockOnCamera(float DeltaTime);

	// 查找最近的敌人
	AActor* FindNearestEnemy(float MaxDistance = 1000.0f);

	// 检查目标是否有效
	bool IsValidTarget(AActor* Target) const;
};
