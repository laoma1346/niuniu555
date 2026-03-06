// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdvancedMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UAdvancedMovementComponent::UAdvancedMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAdvancedMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAdvancedMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateGroundDetection();
	UpdateMovement(DeltaTime);
	UpdateRotation(DeltaTime);
}

void UAdvancedMovementComponent::AddMovementInput(FVector WorldDirection, float Scale)
{
	if (Scale != 0.0f && !WorldDirection.IsNearlyZero())
	{
		InputDirection = WorldDirection.GetSafeNormal() * Scale;
	}
	else
	{
		InputDirection = FVector::ZeroVector;
	}
}

void UAdvancedMovementComponent::SetRunning(bool bRunning)
{
	bIsRunning = bRunning;
}

void UAdvancedMovementComponent::PerformStop()
{
	CurrentState = EMovementState::Stopping;
	CurrentVelocity = FVector::ZeroVector;
	InputDirection = FVector::ZeroVector;
}

bool UAdvancedMovementComponent::IsMoving() const
{
	return GetCurrentSpeed() > 10.0f;
}

void UAdvancedMovementComponent::UpdateMovement(float DeltaTime)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	// 获取UE原生移动组件
	::UCharacterMovementComponent* UEMoveComp = Character->GetCharacterMovement();
	if (!UEMoveComp)
	{
		return;
	}

	// 同步我们的状态到UE移动组件
	if (bIsRunning)
	{
		UEMoveComp->MaxWalkSpeed = RunSpeed;
		CurrentState = IsMoving() ? EMovementState::Running : EMovementState::Idle;
	}
	else
	{
		UEMoveComp->MaxWalkSpeed = WalkSpeed;
		CurrentState = IsMoving() ? EMovementState::Walking : EMovementState::Idle;
	}

	// 更新当前速度（从UE移动组件同步）
	CurrentVelocity = UEMoveComp->Velocity;
	CurrentVelocity.Z = 0.0f; // 只关心水平速度
}

void UAdvancedMovementComponent::UpdateRotation(float DeltaTime)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	// 如果有输入方向，平滑转向到该方向
	if (!InputDirection.IsNearlyZero())
	{
		FVector TargetDirection = InputDirection.GetSafeNormal();
		FRotator CurrentRotation = Character->GetActorRotation();
		FRotator TargetRotation = TargetDirection.Rotation();

		// 只关注YAW（水平）转向
		TargetRotation.Pitch = 0.0f;
		TargetRotation.Roll = 0.0f;

		// 平滑插值转向
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);
		Character->SetActorRotation(NewRotation);
	}
}

void UAdvancedMovementComponent::UpdateGroundDetection()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	::UCharacterMovementComponent* UEMoveComp = Character->GetCharacterMovement();
	if (UEMoveComp)
	{
		GroundInfo.bIsOnGround = UEMoveComp->IsMovingOnGround();
	}
}

float UAdvancedMovementComponent::GetDesiredSpeed() const
{
	return bIsRunning ? RunSpeed : WalkSpeed;
}

void UAdvancedMovementComponent::ApplyFriction(float DeltaTime, float Friction)
{
	if (CurrentVelocity.IsNearlyZero())
	{
		return;
	}

	float Speed = CurrentVelocity.Size();
	float NewSpeed = FMath::Max(0.0f, Speed - Friction * DeltaTime);
	
	if (NewSpeed < 1.0f)
	{
		CurrentVelocity = FVector::ZeroVector;
	}
	else
	{
		CurrentVelocity = CurrentVelocity.GetSafeNormal() * NewSpeed;
	}
}

void UAdvancedMovementComponent::ApplyAcceleration(float DeltaTime, FVector Direction, float Speed)
{
	if (Direction.IsNearlyZero())
	{
		return;
	}

	FVector TargetVelocity = Direction.GetSafeNormal() * Speed;
	FVector DeltaVelocity = TargetVelocity - CurrentVelocity;

	// 限制加速度大小
	float MaxAccel = Acceleration * DeltaTime;
	if (DeltaVelocity.Size() > MaxAccel)
	{
		DeltaVelocity = DeltaVelocity.GetSafeNormal() * MaxAccel;
	}

	CurrentVelocity += DeltaVelocity;
}
