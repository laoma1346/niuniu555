// Copyright Epic Games, Inc. All Rights Reserved.

#include "DodgeComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UDodgeComponent::UDodgeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentStamina = MaxStamina;
}

void UDodgeComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentStamina = MaxStamina;
	CurrentState = EDodgeState::Ready;
}

void UDodgeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateDodge(DeltaTime);
	UpdateStaminaRecovery(DeltaTime);
}

bool UDodgeComponent::PerformDodge(EDodgeDirection Direction)
{
	if (!CanDodge())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot dodge now!"));
		return false;
	}

	// 消耗体力
	if (!ConsumeStamina(DodgeStaminaCost))
	{
		return false;
	}

	// 执行闪避
	CurrentState = EDodgeState::Dodging;
	DodgeTimer = 0.0f;
	bIsInvincible = false;

	ExecuteDodgeMovement(Direction);

	OnDodgeStarted.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("Dodge started! Direction: %d"), (int32)Direction);

	return true;
}

bool UDodgeComponent::CanDodge() const
{
	return CurrentState == EDodgeState::Ready && CurrentStamina >= DodgeStaminaCost;
}

float UDodgeComponent::GetStaminaPercent() const
{
	if (MaxStamina > 0.0f)
	{
		return CurrentStamina / MaxStamina;
	}
	return 0.0f;
}

void UDodgeComponent::RecoverStamina(float Amount)
{
	float OldStamina = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);
	
	if (CurrentStamina != OldStamina)
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}

bool UDodgeComponent::ConsumeStamina(float Amount)
{
	if (CurrentStamina >= Amount)
	{
		CurrentStamina -= Amount;
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
		return true;
	}
	return false;
}

void UDodgeComponent::ExecuteDodgeMovement(EDodgeDirection Direction)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	// 计算闪避方向
	FVector DodgeDirection = FVector::ZeroVector;
	FVector Forward = Character->GetActorForwardVector();
	FVector Right = Character->GetActorRightVector();

	switch (Direction)
	{
	case EDodgeDirection::Forward:
		DodgeDirection = Forward;
		break;
	case EDodgeDirection::Backward:
		DodgeDirection = -Forward;
		break;
	case EDodgeDirection::Left:
		DodgeDirection = -Right;
		break;
	case EDodgeDirection::Right:
		DodgeDirection = Right;
		break;
	case EDodgeDirection::Auto:
	default:
		// 自动：根据输入方向，如果没有输入则向后闪
		DodgeDirection = -Forward;
		break;
	}

	// 执行闪避位移（使用LaunchCharacter实现冲刺效果）
	FVector LaunchVelocity = DodgeDirection.GetSafeNormal() * (DodgeDistance / DodgeDuration);
	LaunchVelocity.Z = 0.0f; // 保持水平移动

	Character->LaunchCharacter(LaunchVelocity, true, true);
}

void UDodgeComponent::UpdateDodge(float DeltaTime)
{
	// 处理闪避状态
	if (CurrentState == EDodgeState::Dodging)
	{
		DodgeTimer += DeltaTime;

		// 更新无敌帧
		UpdateInvincibility(DeltaTime);

		// 检查闪避结束
		if (DodgeTimer >= DodgeDuration)
		{
			EndDodge();
		}
	}
	// 处理冷却状态
	else if (CurrentState == EDodgeState::Cooldown)
	{
		CooldownTimer += DeltaTime;

		// 检查冷却结束
		if (CooldownTimer >= DodgeCooldown)
		{
			EndCooldown();
		}
	}
}

void UDodgeComponent::UpdateStaminaRecovery(float DeltaTime)
{
	// 只有在非闪避状态才恢复体力
	if (CurrentState != EDodgeState::Dodging && CurrentStamina < MaxStamina)
	{
		RecoverStamina(StaminaRecoveryRate * DeltaTime);
	}
}

void UDodgeComponent::UpdateInvincibility(float DeltaTime)
{
	// 检查是否进入无敌帧
	if (!bIsInvincible && DodgeTimer >= InvincibilityStartTime)
	{
		bIsInvincible = true;
		OnInvincibilityStarted.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("Invincibility started!"));
	}

	// 检查是否结束无敌帧
	if (bIsInvincible && DodgeTimer >= InvincibilityStartTime + InvincibilityDuration)
	{
		bIsInvincible = false;
		OnInvincibilityEnded.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("Invincibility ended!"));
	}
}

void UDodgeComponent::EndDodge()
{
	CurrentState = EDodgeState::Cooldown;
	CooldownTimer = 0.0f;
	OnDodgeEnded.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("Dodge ended!"));

	// 停止角色移动
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		Character->GetCharacterMovement()->StopMovementImmediately();
	}
}

void UDodgeComponent::StartCooldown()
{
	CurrentState = EDodgeState::Cooldown;
	CooldownTimer = 0.0f;
}

void UDodgeComponent::EndCooldown()
{
	CurrentState = EDodgeState::Ready;
	UE_LOG(LogTemp, Log, TEXT("Dodge ready!"));
}
