// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DodgeComponent.generated.h"

// 闪避状态枚举
UENUM(BlueprintType)
enum class EDodgeState : uint8
{
	Ready			UMETA(DisplayName = "准备就绪"),
	Dodging			UMETA(DisplayName = "闪避中"),
	Cooldown		UMETA(DisplayName = "冷却中"),
	Exhausted		UMETA(DisplayName = "体力耗尽")
};

// 闪避方向
UENUM(BlueprintType)
enum class EDodgeDirection : uint8
{
	Forward			UMETA(DisplayName = "前闪"),
	Backward		UMETA(DisplayName = "后闪"),
	Left			UMETA(DisplayName = "左闪"),
	Right			UMETA(DisplayName = "右闪"),
	Auto			UMETA(DisplayName = "自动"),
};

// 闪避开始委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDodgeStarted);

// 闪避结束委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDodgeEnded);

// 无敌帧开始委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvincibilityStarted);

// 无敌帧结束委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvincibilityEnded);

// 体力变化委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, CurrentStamina, float, MaxStamina);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NIUNIU555_API UDodgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDodgeComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== 闪避操作 ==========
	UFUNCTION(BlueprintCallable, Category = "Dodge")
	bool PerformDodge(EDodgeDirection Direction = EDodgeDirection::Auto);

	UFUNCTION(BlueprintPure, Category = "Dodge")
	bool CanDodge() const;

	// ========== 状态查询 ==========
	UFUNCTION(BlueprintPure, Category = "Dodge")
	EDodgeState GetDodgeState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Dodge")
	bool IsInvincible() const { return bIsInvincible; }

	UFUNCTION(BlueprintPure, Category = "Dodge")
	bool IsDodging() const { return CurrentState == EDodgeState::Dodging; }

	UFUNCTION(BlueprintPure, Category = "Dodge")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Dodge")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Dodge")
	float GetStaminaPercent() const;

	// ========== 体力恢复 ==========
	UFUNCTION(BlueprintCallable, Category = "Dodge")
	void RecoverStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Dodge")
	bool ConsumeStamina(float Amount);

	// ========== 配置参数 ==========
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Stamina")
	float DodgeStaminaCost = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Stamina")
	float StaminaRecoveryRate = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Movement")
	float DodgeDistance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Timing")
	float DodgeDuration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Timing")
	float InvincibilityStartTime = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Timing")
	float InvincibilityDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Timing")
	float DodgeCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Cancel")
	float CancelWindowTime = 0.2f;

	// ========== 事件委托 ==========
	UPROPERTY(BlueprintAssignable, Category = "Dodge|Events")
	FOnDodgeStarted OnDodgeStarted;

	UPROPERTY(BlueprintAssignable, Category = "Dodge|Events")
	FOnDodgeEnded OnDodgeEnded;

	UPROPERTY(BlueprintAssignable, Category = "Dodge|Events")
	FOnInvincibilityStarted OnInvincibilityStarted;

	UPROPERTY(BlueprintAssignable, Category = "Dodge|Events")
	FOnInvincibilityEnded OnInvincibilityEnded;

	UPROPERTY(BlueprintAssignable, Category = "Dodge|Events")
	FOnStaminaChanged OnStaminaChanged;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Dodge")
	EDodgeState CurrentState = EDodgeState::Ready;

	UPROPERTY(BlueprintReadOnly, Category = "Dodge")
	float CurrentStamina = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Dodge")
	bool bIsInvincible = false;

	UPROPERTY()
	float DodgeTimer = 0.0f;

	UPROPERTY()
	float CooldownTimer = 0.0f;

	void ExecuteDodgeMovement(EDodgeDirection Direction);
	void UpdateDodge(float DeltaTime);
	void UpdateStaminaRecovery(float DeltaTime);
	void UpdateInvincibility(float DeltaTime);
	void EndDodge();
	void StartCooldown();
	void EndCooldown();
};
