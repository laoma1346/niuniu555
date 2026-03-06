// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraControllerComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UCameraControllerComponent::UCameraControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCameraControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	// 初始化摄像机角度
	CurrentYaw = 0.0f;
	CurrentPitch = 0.0f;
	TargetYaw = 0.0f;
	TargetPitch = 0.0f;
}

void UCameraControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (CurrentMode)
	{
	case ECameraMode::Free:
		UpdateFreeCamera(DeltaTime);
		break;
	case ECameraMode::LockOn:
		UpdateLockOnCamera(DeltaTime);
		break;
	default:
		break;
	}

	// 检查锁定目标是否有效
	if (CurrentMode == ECameraMode::LockOn && !IsValidTarget(LockOnTarget))
	{
		ClearLockOnTarget();
		SetCameraMode(ECameraMode::Free);
	}
}

void UCameraControllerComponent::SetCameraMode(ECameraMode NewMode)
{
	if (NewMode == CurrentMode)
	{
		return;
	}

	CurrentMode = NewMode;

	if (CurrentMode == ECameraMode::LockOn)
	{
		// 切换到锁定模式，查找目标
		if (!LockOnTarget)
		{
			LockOnTarget = FindNearestEnemy();
		}
		if (!LockOnTarget)
		{
			// 没有找到目标，切回自由视角
			CurrentMode = ECameraMode::Free;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Camera mode changed to %d"), (int32)CurrentMode);
}

void UCameraControllerComponent::ToggleLockOn()
{
	if (CurrentMode == ECameraMode::LockOn)
	{
		SetCameraMode(ECameraMode::Free);
		ClearLockOnTarget();
	}
	else
	{
		AActor* NearestEnemy = FindNearestEnemy();
		if (NearestEnemy)
		{
			SetLockOnTarget(NearestEnemy);
			SetCameraMode(ECameraMode::LockOn);
		}
	}
}

void UCameraControllerComponent::SetLockOnTarget(AActor* Target)
{
	if (IsValidTarget(Target))
	{
		LockOnTarget = Target;
		UE_LOG(LogTemp, Log, TEXT("Lock on target: %s"), *Target->GetName());
	}
}

void UCameraControllerComponent::ClearLockOnTarget()
{
	LockOnTarget = nullptr;
}

void UCameraControllerComponent::AddCameraInput(float Yaw, float Pitch)
{
	if (CurrentMode == ECameraMode::Free)
	{
		TargetYaw += Yaw * FreeLookSpeed;
		TargetPitch += Pitch * FreeLookSpeed;

		// 限制俯仰角
		TargetPitch = FMath::Clamp(TargetPitch, MinPitch, MaxPitch);
	}
}

void UCameraControllerComponent::ResetCamera()
{
	TargetYaw = 0.0f;
	TargetPitch = 0.0f;
}

void UCameraControllerComponent::UpdateFreeCamera(float DeltaTime)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	// 平滑插值到目标角度
	CurrentYaw = FMath::FInterpTo(CurrentYaw, TargetYaw, DeltaTime, 5.0f);
	CurrentPitch = FMath::FInterpTo(CurrentPitch, TargetPitch, DeltaTime, 5.0f);

	// 计算摄像机旋转
	FRotator CameraRotation(CurrentPitch, CurrentYaw, 0.0f);

	// 设置角色朝向与摄像机一致（可选，根据需要决定是否同步）
	// Character->SetActorRotation(FRotator(0.0f, CurrentYaw, 0.0f));
}

void UCameraControllerComponent::UpdateLockOnCamera(float DeltaTime)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !LockOnTarget)
	{
		return;
	}

	// 计算角色到目标的方向
	FVector CharacterLocation = Character->GetActorLocation();
	FVector TargetLocation = LockOnTarget->GetActorLocation();
	FVector DirectionToTarget = (TargetLocation - CharacterLocation).GetSafeNormal();

	// 计算目标Yaw
	TargetYaw = DirectionToTarget.Rotation().Yaw;

	// 平滑转向
	float CurrentCharacterYaw = Character->GetActorRotation().Yaw;
	float NewYaw = FMath::FInterpTo(CurrentCharacterYaw, TargetYaw, DeltaTime, LockOnRotationSpeed);

	// 设置角色朝向目标
	Character->SetActorRotation(FRotator(0.0f, NewYaw, 0.0f));

	// 同步摄像机Yaw
	CurrentYaw = NewYaw;
	TargetYaw = NewYaw;
}

AActor* UCameraControllerComponent::FindNearestEnemy(float MaxDistance)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return nullptr;
	}

	FVector CharacterLocation = Character->GetActorLocation();
	AActor* NearestEnemy = nullptr;
	float MinDistance = MaxDistance;

	// 查找所有CombatEnemy（假设敌人类）
	TArray<AActor*> AllEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllEnemies);

	for (AActor* Actor : AllEnemies)
	{
		// 排除自己和非敌人
		if (Actor == GetOwner())
		{
			continue;
		}

		// 检查是否是敌人（可以根据标签或类来判断）
		if (Actor->ActorHasTag(FName("Enemy")) || Actor->GetName().Contains(TEXT("Enemy")))
		{
			float Distance = FVector::Distance(CharacterLocation, Actor->GetActorLocation());
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				NearestEnemy = Actor;
			}
		}
	}

	return NearestEnemy;
}

bool UCameraControllerComponent::IsValidTarget(AActor* Target) const
{
	return Target && !Target->IsPendingKillPending() && Target->IsValidLowLevel();
}
