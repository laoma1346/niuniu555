// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AdvancedMovementComponent.generated.h"

// 移动状态枚举
UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Idle			UMETA(DisplayName = "静止"),
	Walking			UMETA(DisplayName = "行走"),
	Running			UMETA(DisplayName = "奔跑"),
	Sprinting		UMETA(DisplayName = "冲刺"),
	Stopping		UMETA(DisplayName = "急停")
};

// 地面检测信息
USTRUCT(BlueprintType)
struct FGroundInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bIsOnGround = false;

	UPROPERTY(BlueprintReadOnly)
	FVector GroundNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance = 0.0f;
};

/**
 * 角色移动组件 - 增强版移动控制
 * 支持：奔跑/急停/转向插值/地面检测
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NIUNIU555_API UAdvancedMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAdvancedMovementComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== 移动控制 ==========

	// 添加移动输入（方向）
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void AddMovementInput(FVector WorldDirection, float Scale = 1.0f);

	// 设置奔跑状态
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetRunning(bool bRunning);

	// 急停
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void PerformStop();

	// ========== 查询 ==========

	// 获取当前移动状态
	UFUNCTION(BlueprintPure, Category = "Movement")
	EMovementState GetMovementState() const { return CurrentState; }

	// 是否正在移动
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsMoving() const;

	// 获取当前速度
	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetCurrentSpeed() const { return CurrentVelocity.Size(); }

	// 获取当前速度向量
	UFUNCTION(BlueprintPure, Category = "Movement")
	FVector GetCurrentVelocity() const { return CurrentVelocity; }

	// 获取地面信息
	UFUNCTION(BlueprintPure, Category = "Movement")
	FGroundInfo GetGroundInfo() const { return GroundInfo; }

	// ========== 配置参数 ==========

	// 行走速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float WalkSpeed = 300.0f;

	// 奔跑速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float RunSpeed = 600.0f;

	// 转向速度（插值速度）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Rotation")
	float RotationSpeed = 10.0f;

	// 急停减速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stopping")
	float StopDeceleration = 2000.0f;

	// 加速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Acceleration")
	float Acceleration = 1500.0f;

	// 减速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Acceleration")
	float Deceleration = 800.0f;

protected:
	// 当前移动状态
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	EMovementState CurrentState = EMovementState::Idle;

	// 当前速度向量
	UPROPERTY()
	FVector CurrentVelocity = FVector::ZeroVector;

	// 输入方向
	UPROPERTY()
	FVector InputDirection = FVector::ZeroVector;

	// 是否正在奔跑
	UPROPERTY()
	bool bIsRunning = false;

	// 地面信息
	UPROPERTY()
	FGroundInfo GroundInfo;

	// 更新移动
	void UpdateMovement(float DeltaTime);

	// 更新转向
	void UpdateRotation(float DeltaTime);

	// 更新地面检测
	void UpdateGroundDetection();

	// 计算期望速度
	float GetDesiredSpeed() const;

	// 应用摩擦力
	void ApplyFriction(float DeltaTime, float Friction);

	// 应用加速度
	void ApplyAcceleration(float DeltaTime, FVector Direction, float Speed);
};
