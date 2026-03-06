// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HookshotComponent.generated.h"

// 钩索状态枚举
UENUM(BlueprintType)
enum class EHookshotState : uint8
{
	Ready			UMETA(DisplayName = "准备就绪"),
	Aiming			UMETA(DisplayName = "瞄准中"),
	Shooting		UMETA(DisplayName = "发射中"),
	Pulling			UMETA(DisplayName = "拉回中"),
	Cooldown		UMETA(DisplayName = "冷却中")
};

// 钩索拉回模式（已简化，仅支持目标拉向玩家）
UENUM(BlueprintType)
enum class EHookshotPullMode : uint8
{
	PullTargetToPlayer	UMETA(DisplayName = "目标拉向玩家")
};

// 钩索检测结果
USTRUCT(BlueprintType)
struct FHookshotHitResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bHit = false;

	UPROPERTY(BlueprintReadOnly)
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector HitNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> HitActor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	float Distance = 0.0f;
};

// 钩索发射委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHookshotFired);

// 钩索命中委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHookshotHit, const FHookshotHitResult&, HitResult);

// 钩索完成委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHookshotCompleted);

// 瞄准更新委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAimUpdated, const FHookshotHitResult&, AimResult);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NIUNIU555_API UHookshotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHookshotComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== 钩索操作 ==========
	UFUNCTION(BlueprintCallable, Category = "Hookshot")
	void StartAiming();

	UFUNCTION(BlueprintCallable, Category = "Hookshot")
	void EndAiming();

	UFUNCTION(BlueprintCallable, Category = "Hookshot")
	bool FireHookshot();

	UFUNCTION(BlueprintCallable, Category = "Hookshot")
	void CancelHookshot();

	// ========== 鼠标瞄准 ==========

	// 更新鼠标瞄准方向（每帧调用）
	UFUNCTION(BlueprintCallable, Category = "Hookshot")
	void UpdateMouseAim();

	// 获取当前鼠标瞄准方向
	UFUNCTION(BlueprintPure, Category = "Hookshot")
	FVector GetMouseAimDirection() const { return CachedMouseAimDirection; }

	// ========== 状态查询 ==========
	UFUNCTION(BlueprintPure, Category = "Hookshot")
	EHookshotState GetHookshotState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Hookshot")
	bool IsAiming() const { return CurrentState == EHookshotState::Aiming; }

	UFUNCTION(BlueprintPure, Category = "Hookshot")
	bool CanFire() const { return CurrentState == EHookshotState::Ready || CurrentState == EHookshotState::Aiming; }

	UFUNCTION(BlueprintPure, Category = "Hookshot")
	FHookshotHitResult GetAimResult() const { return AimResult; }

	// ========== 配置参数 ==========
	// 注意：当前仅支持 PullTargetToPlayer 模式（目标拉向玩家）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hookshot|Mode")
	EHookshotPullMode PullMode = EHookshotPullMode::PullTargetToPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hookshot|Range")
	float MaxRange = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hookshot|Speed")
	float FireSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hookshot|Speed")
	float PullSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hookshot|Timing")
	float CooldownTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hookshot|Aim")
	float AimLineMaxLength = 1000.0f;

	// 可拉取的Actor标签（作为辅助验证）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hookshot|Target")
	FName PullableActorTag = FName("Enemy");

	// 钩索检测的碰撞通道（在项目设置中配置）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hookshot|Collision")
	TEnumAsByte<ECollisionChannel> HookshotTraceChannel = ECC_GameTraceChannel1;

	// 是否启用标签验证
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hookshot|Target")
	bool bUseTagValidation = true;

	// ========== 事件委托 ==========
	UPROPERTY(BlueprintAssignable, Category = "Hookshot|Events")
	FOnHookshotFired OnHookshotFired;

	UPROPERTY(BlueprintAssignable, Category = "Hookshot|Events")
	FOnHookshotHit OnHookshotHit;

	UPROPERTY(BlueprintAssignable, Category = "Hookshot|Events")
	FOnHookshotCompleted OnHookshotCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Hookshot|Events")
	FOnAimUpdated OnAimUpdated;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Hookshot")
	EHookshotState CurrentState = EHookshotState::Ready;

	UPROPERTY()
	FHookshotHitResult AimResult;

	UPROPERTY()
	FVector HookshotHeadLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY()
	float CooldownTimer = 0.0f;

	UPROPERTY()
	float FireTimer = 0.0f;

	// 缓存的鼠标瞄准方向
	UPROPERTY()
	FVector CachedMouseAimDirection = FVector::ForwardVector;

	void UpdateAiming();
	FHookshotHitResult PerformRaycast();
	void UpdateFiring(float DeltaTime);
	void UpdatePulling(float DeltaTime);
	void UpdatePullingTargetToPlayer(float DeltaTime);
	void StartPulling();
	void EndPulling();
	void StartCooldown();
	void EndCooldown();
	FVector GetFireStartLocation() const;
	FVector GetFireDirection() const;
};
