// Copyright Epic Games, Inc. All Rights Reserved.

#include "HookshotComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

UHookshotComponent::UHookshotComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHookshotComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentState = EHookshotState::Ready;
	CachedMouseAimDirection = FVector::ForwardVector;
}

void UHookshotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新鼠标瞄准
	if (CurrentState == EHookshotState::Aiming || CurrentState == EHookshotState::Ready)
	{
		UpdateMouseAim();
	}

	// 更新瞄准
	if (CurrentState == EHookshotState::Aiming)
	{
		UpdateAiming();
	}

	// 更新发射
	if (CurrentState == EHookshotState::Shooting)
	{
		UpdateFiring(DeltaTime);
	}

	// 更新拉回
	if (CurrentState == EHookshotState::Pulling)
	{
		UpdatePulling(DeltaTime);
	}

	// 更新冷却
	if (CurrentState == EHookshotState::Cooldown)
	{
		CooldownTimer -= DeltaTime;
		if (CooldownTimer <= 0.0f)
		{
			EndCooldown();
		}
	}
}

void UHookshotComponent::UpdateMouseAim()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC)
	{
		// 如果没有玩家控制器，使用角色朝向
		CachedMouseAimDirection = Character->GetActorForwardVector();
		return;
	}

	// 获取鼠标在屏幕上的位置
	float MouseX, MouseY;
	PC->GetMousePosition(MouseX, MouseY);

	// 将屏幕位置转换为世界方向
	FVector WorldLocation, WorldDirection;
	if (PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection))
	{
		// 只使用水平方向（忽略Z轴差异）
		CachedMouseAimDirection = WorldDirection;
		CachedMouseAimDirection.Z = 0.0f;
		CachedMouseAimDirection = CachedMouseAimDirection.GetSafeNormal();
	}
}

void UHookshotComponent::StartAiming()
{
	if (CurrentState == EHookshotState::Ready)
	{
		CurrentState = EHookshotState::Aiming;
		UpdateMouseAim(); // 立即更新一次瞄准
		UpdateAiming();
		UE_LOG(LogTemp, Log, TEXT("Hookshot: Start Aiming"));
	}
}

void UHookshotComponent::EndAiming()
{
	if (CurrentState == EHookshotState::Aiming)
	{
		CurrentState = EHookshotState::Ready;
		UE_LOG(LogTemp, Log, TEXT("Hookshot: End Aiming"));
	}
}

bool UHookshotComponent::FireHookshot()
{
	if (!CanFire())
	{
		return false;
	}

	// 执行射线检测（使用鼠标瞄准方向）
	FHookshotHitResult HitResult = PerformRaycast();

	if (!HitResult.bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hookshot: No target hit!"));
		return false;
	}

	// 开始发射
	CurrentState = EHookshotState::Shooting;
	TargetLocation = HitResult.HitLocation;
	HookshotHeadLocation = GetFireStartLocation();
	FireTimer = 0.0f;

	OnHookshotFired.Broadcast();
	OnHookshotHit.Broadcast(HitResult);

	UE_LOG(LogTemp, Log, TEXT("Hookshot: Fired to %s"), *TargetLocation.ToString());

	return true;
}

void UHookshotComponent::CancelHookshot()
{
	if (CurrentState == EHookshotState::Shooting || CurrentState == EHookshotState::Pulling)
	{
		CurrentState = EHookshotState::Cooldown;
		CooldownTimer = CooldownTime * 0.5f; // 取消时减半冷却
	}
}

void UHookshotComponent::UpdateAiming()
{
	AimResult = PerformRaycast();
	OnAimUpdated.Broadcast(AimResult);
}

FHookshotHitResult UHookshotComponent::PerformRaycast()
{
	FHookshotHitResult Result;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return Result;
	}

	FVector Start = GetFireStartLocation();
	FVector Direction = GetFireDirection(); // 现在使用鼠标方向
	FVector End = Start + Direction * MaxRange;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	// 使用专属碰撞通道进行检测（默认为GameTraceChannel1）
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, HookshotTraceChannel, QueryParams);

	// 如果启用了标签验证，检查命中的Actor是否有指定标签
	if (bHit && bUseTagValidation && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor->ActorHasTag(PullableActorTag))
		{
			// 标签不匹配，视为未命中
			bHit = false;
		}
	}

	if (bHit)
	{
		Result.bHit = true;
		Result.HitLocation = HitResult.Location;
		Result.HitNormal = HitResult.Normal;
		Result.HitActor = HitResult.GetActor();
		Result.Distance = HitResult.Distance;
	}

	return Result;
}

void UHookshotComponent::UpdateFiring(float DeltaTime)
{
	FireTimer += DeltaTime;

	// 计算钩索头当前位置
	FVector Start = GetFireStartLocation();
	float TotalDistance = FVector::Distance(Start, TargetLocation);
	float TravelDistance = FireSpeed * FireTimer;

	if (TravelDistance >= TotalDistance)
	{
		// 到达目标，开始拉回
		HookshotHeadLocation = TargetLocation;
		StartPulling();
	}
	else
	{
		// 更新钩索头位置
		FVector Direction = (TargetLocation - Start).GetSafeNormal();
		HookshotHeadLocation = Start + Direction * TravelDistance;
	}
}

void UHookshotComponent::UpdatePulling(float DeltaTime)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		EndPulling();
		return;
	}

	// 模式：把目标拉向玩家（唯一支持的模式）
	UpdatePullingTargetToPlayer(DeltaTime);
}

void UHookshotComponent::UpdatePullingTargetToPlayer(float DeltaTime)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		EndPulling();
		return;
	}

	// 获取命中的Actor - 多重验证避免报错
	AActor* TargetActor = AimResult.HitActor;
	if (!IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Hookshot: Target actor is no longer valid, stopping pull."));
		EndPulling();
		return;
	}

	// 验证Actor根组件是否有效（避免已销毁但未清除引用的对象）
	USceneComponent* TargetRoot = TargetActor->GetRootComponent();
	if (!IsValid(TargetRoot))
	{
		UE_LOG(LogTemp, Warning, TEXT("Hookshot: Target actor root component is invalid, stopping pull."));
		EndPulling();
		return;
	}

	FVector PlayerLocation = Character->GetActorLocation();
	FVector TargetLocation_Current = TargetActor->GetActorLocation();
	
	// 计算目标到玩家的方向
	FVector ToPlayer = PlayerLocation - TargetLocation_Current;
	float DistanceToPlayer = ToPlayer.Size();

	// 检查目标是否已经被拉到玩家附近
	if (DistanceToPlayer <= 150.0f)
	{
		EndPulling();
		return;
	}

	// 计算移动方向
	FVector MoveDirection = ToPlayer.GetSafeNormal();
	FVector NewLocation = TargetLocation_Current + MoveDirection * PullSpeed * DeltaTime;

	// 设置目标新位置（添加有效性检查，避免报错）
	if (IsValid(TargetActor))
	{
		TargetActor->SetActorLocation(NewLocation);
		
		// 玩家朝向目标
		FRotator TargetRotation = (-MoveDirection).Rotation();
		Character->SetActorRotation(FRotator(0.0f, TargetRotation.Yaw, 0.0f));
	}
	else
	{
		EndPulling();
	}
}

void UHookshotComponent::StartPulling()
{
	CurrentState = EHookshotState::Pulling;
	UE_LOG(LogTemp, Log, TEXT("Hookshot: Start Pulling"));
}

void UHookshotComponent::EndPulling()
{
	OnHookshotCompleted.Broadcast();
	StartCooldown();
	UE_LOG(LogTemp, Log, TEXT("Hookshot: End Pulling"));
}

void UHookshotComponent::StartCooldown()
{
	CurrentState = EHookshotState::Cooldown;
	CooldownTimer = CooldownTime;
}

void UHookshotComponent::EndCooldown()
{
	CurrentState = EHookshotState::Ready;
	UE_LOG(LogTemp, Log, TEXT("Hookshot: Ready"));
}

FVector UHookshotComponent::GetFireStartLocation() const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		// 从角色眼睛高度发射
		FVector Location = Character->GetActorLocation();
		Location.Z += Character->BaseEyeHeight;
		return Location;
	}
	return FVector::ZeroVector;
}

FVector UHookshotComponent::GetFireDirection() const
{
	// 使用鼠标瞄准方向
	return CachedMouseAimDirection.IsNearlyZero() ? FVector::ForwardVector : CachedMouseAimDirection;
}
