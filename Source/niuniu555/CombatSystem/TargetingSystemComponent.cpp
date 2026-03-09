#include "TargetingSystemComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogTargetingSystem);

UTargetingSystemComponent::UTargetingSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UTargetingSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// 获取拥有者
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		OwnerController = Character->GetController();
		CameraComponent = Character->FindComponentByClass<UCameraComponent>();
	}

	UE_LOG(LogTargetingSystem, Log, TEXT("[%s] 准星与锁定系统初始化完成"), *GetOwner()->GetName());
}

void UTargetingSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新锁定状态
	if (IsLockedOn())
	{
		UpdateLockOn();
	}

	// 更新准星扩散
	UpdateCrosshairSpread(DeltaTime);

	// 调试显示
	if (bShowDebug)
	{
		DrawDebugInfo();
	}
}

void UTargetingSystemComponent::ToggleLockOn()
{
	if (IsLockedOn())
	{
		Unlock();
	}
	else
	{
		TryLockOn();
	}
}

bool UTargetingSystemComponent::TryLockOn()
{
	AActor* Target = FindBestLockOnTarget();
	if (Target)
	{
		CurrentTarget.TargetActor = Target;
		CurrentTarget.Distance = GetTargetDistance(Target);
		CurrentTarget.Angle = GetTargetAngle(Target);
		CurrentTarget.bIsVisible = CheckTargetVisibility(Target);

		OnTargetChanged.Broadcast(Target);
		OnLockOnStateChanged.Broadcast(true);

		UE_LOG(LogTargetingSystem, Log, TEXT("[%s] 锁定目标: %s"), 
			*GetOwner()->GetName(), *Target->GetName());

		return true;
	}

	UE_LOG(LogTargetingSystem, Verbose, TEXT("[%s] 未找到可锁定目标"), *GetOwner()->GetName());
	return false;
}

void UTargetingSystemComponent::Unlock()
{
	if (CurrentTarget.IsValid())
	{
		UE_LOG(LogTargetingSystem, Log, TEXT("[%s] 解除锁定: %s"), 
			*GetOwner()->GetName(), *CurrentTarget.TargetActor->GetName());

		CurrentTarget.TargetActor = nullptr;
		OnTargetChanged.Broadcast(nullptr);
		OnLockOnStateChanged.Broadcast(false);
	}
}

void UTargetingSystemComponent::SwitchToNextTarget()
{
	if (!IsLockedOn()) return;

	AActor* NewTarget = FindTargetInDirection(true);
	if (NewTarget && NewTarget != CurrentTarget.TargetActor)
	{
		CurrentTarget.TargetActor = NewTarget;
		CurrentTarget.Distance = GetTargetDistance(NewTarget);
		CurrentTarget.Angle = GetTargetAngle(NewTarget);
		CurrentTarget.bIsVisible = CheckTargetVisibility(NewTarget);

		OnTargetChanged.Broadcast(NewTarget);

		UE_LOG(LogTargetingSystem, Log, TEXT("[%s] 切换到下一个目标: %s"), 
			*GetOwner()->GetName(), *NewTarget->GetName());
	}
}

void UTargetingSystemComponent::SwitchToPreviousTarget()
{
	if (!IsLockedOn()) return;

	AActor* NewTarget = FindTargetInDirection(false);
	if (NewTarget && NewTarget != CurrentTarget.TargetActor)
	{
		CurrentTarget.TargetActor = NewTarget;
		CurrentTarget.Distance = GetTargetDistance(NewTarget);
		CurrentTarget.Angle = GetTargetAngle(NewTarget);
		CurrentTarget.bIsVisible = CheckTargetVisibility(NewTarget);

		OnTargetChanged.Broadcast(NewTarget);

		UE_LOG(LogTargetingSystem, Log, TEXT("[%s] 切换到上一个目标: %s"), 
			*GetOwner()->GetName(), *NewTarget->GetName());
	}
}

FVector UTargetingSystemComponent::GetTargetLockLocation() const
{
	if (!CurrentTarget.IsValid()) return FVector::ZeroVector;

	FVector Location = CurrentTarget.TargetActor->GetActorLocation();
	Location.Z += TargetHeightOffset;

	return Location;
}

AActor* UTargetingSystemComponent::GetSoftLockTarget() const
{
	TArray<FLockOnTargetInfo> SortedTargets = GetSortedTargetsByAngle();
	
	for (const FLockOnTargetInfo& Info : SortedTargets)
	{
		if (Info.Angle <= SoftLockAngle && Info.bIsVisible)
		{
			return Info.TargetActor;
		}
	}

	return nullptr;
}

bool UTargetingSystemComponent::HasSoftLockTarget() const
{
	return GetSoftLockTarget() != nullptr;
}

void UTargetingSystemComponent::AddCrosshairSpread(float Amount)
{
	CurrentCrosshairSpread = FMath::Clamp(CurrentCrosshairSpread + Amount, 0.0f, 1.0f);
}

void UTargetingSystemComponent::ResetCrosshairSpread()
{
	CurrentCrosshairSpread = 0.0f;
}

TArray<AActor*> UTargetingSystemComponent::GetAllLockableTargets() const
{
	TArray<AActor*> AllTargets;

	// 获取所有带有LockableTag的Actor
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), LockableTag, AllTargets);

	// 过滤无效目标
	TArray<AActor*> ValidTargets;
	for (AActor* Target : AllTargets)
	{
		if (CanLockOnTarget(Target))
		{
			ValidTargets.Add(Target);
		}
	}

	return ValidTargets;
}

bool UTargetingSystemComponent::CanLockOnTarget(AActor* Target) const
{
	if (!Target || Target == GetOwner()) return false;

	// 检查距离
	float Distance = GetTargetDistance(Target);
	if (Distance > MaxLockOnDistance) return false;

	// 检查角度
	float Angle = GetTargetAngle(Target);
	if (Angle > MaxLockOnAngle) return false;

	// 检查是否有标签
	if (!Target->ActorHasTag(LockableTag)) return false;

	return true;
}

float UTargetingSystemComponent::GetTargetAngle(AActor* Target) const
{
	if (!Target) return 360.0f;

	FVector ToTarget = Target->GetActorLocation() - GetEyeLocation();
	ToTarget.Normalize();

	FVector ViewDir = GetViewDirection();

	float Angle = FMath::Acos(FVector::DotProduct(ViewDir, ToTarget));
	return FMath::RadiansToDegrees(Angle);
}

float UTargetingSystemComponent::GetTargetDistance(AActor* Target) const
{
	if (!Target) return FLT_MAX;

	return FVector::Distance(GetEyeLocation(), Target->GetActorLocation());
}

bool UTargetingSystemComponent::IsTargetInView(AActor* Target) const
{
	return GetTargetAngle(Target) <= MaxLockOnAngle;
}

AActor* UTargetingSystemComponent::FindBestLockOnTarget() const
{
	TArray<FLockOnTargetInfo> SortedTargets = GetSortedTargetsByAngle();

	// 返回角度最小且可见的目标
	for (const FLockOnTargetInfo& Info : SortedTargets)
	{
		if (Info.bIsVisible)
		{
			return Info.TargetActor;
		}
	}

	return nullptr;
}

AActor* UTargetingSystemComponent::FindTargetInDirection(bool bRight) const
{
	if (!CurrentTarget.IsValid()) return nullptr;

	TArray<FLockOnTargetInfo> AllTargets = GetSortedTargetsByAngle();
	int32 CurrentIndex = -1;

	// 找到当前目标的索引
	for (int32 i = 0; i < AllTargets.Num(); ++i)
	{
		if (AllTargets[i].TargetActor == CurrentTarget.TargetActor)
		{
			CurrentIndex = i;
			break;
		}
	}

	if (CurrentIndex == -1) return nullptr;

	// 在当前目标附近查找
	for (int32 i = 0; i < AllTargets.Num(); ++i)
	{
		if (i == CurrentIndex) continue;

		// 检查角度差异
		float AngleDiff = AllTargets[i].Angle - AllTargets[CurrentIndex].Angle;
		
		// 右侧目标角度更大，左侧角度更小
		if (bRight && AngleDiff > 0 && AngleDiff < SwitchTargetAngleThreshold)
		{
			return AllTargets[i].TargetActor;
		}
		if (!bRight && AngleDiff < 0 && FMath::Abs(AngleDiff) < SwitchTargetAngleThreshold)
		{
			return AllTargets[i].TargetActor;
		}
	}

	return nullptr;
}

void UTargetingSystemComponent::UpdateCrosshairSpread(float DeltaTime)
{
	// 基础扩散恢复
	if (CurrentCrosshairSpread > 0.0f)
	{
		CurrentCrosshairSpread = FMath::Max(0.0f, CurrentCrosshairSpread - CrosshairRecoverySpeed * DeltaTime);
	}

	// TODO: 根据移动速度增加扩散
	// TODO: 根据攻击状态增加扩散
}

bool UTargetingSystemComponent::CheckTargetVisibility(AActor* Target) const
{
	if (!Target) return false;

	FVector Start = GetEyeLocation();
	FVector End = Target->GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	// 如果没有击中或者击中了目标本身，说明可见
	return !bHit || HitResult.GetActor() == Target;
}

void UTargetingSystemComponent::UpdateLockOn()
{
	if (!CurrentTarget.IsValid()) return;

	// 更新距离和角度
	CurrentTarget.Distance = GetTargetDistance(CurrentTarget.TargetActor);
	CurrentTarget.Angle = GetTargetAngle(CurrentTarget.TargetActor);
	CurrentTarget.bIsVisible = CheckTargetVisibility(CurrentTarget.TargetActor);

	// 检查是否应该解除锁定
	if (CurrentTarget.Distance > MaxLockOnDistance * 1.5f || // 距离过远
		CurrentTarget.Angle > MaxLockOnAngle * 2.0f) // 角度过大
	{
		Unlock();
	}
}

FVector UTargetingSystemComponent::GetEyeLocation() const
{
	if (CameraComponent)
	{
		return CameraComponent->GetComponentLocation();
	}

	return GetOwner()->GetActorLocation() + FVector(0, 0, 160.0f); // 默认眼睛高度
}

FVector UTargetingSystemComponent::GetViewDirection() const
{
	if (CameraComponent)
	{
		return CameraComponent->GetForwardVector();
	}

	if (OwnerController)
	{
		FRotator Rotation = OwnerController->GetControlRotation();
		return Rotation.Vector();
	}

	return GetOwner()->GetActorForwardVector();
}

void UTargetingSystemComponent::DrawDebugInfo()
{
	// 绘制锁定范围
	FVector Center = GetEyeLocation();
	FVector Forward = GetViewDirection();

	// 绘制角度锥形
	FColor ConeColor = IsLockedOn() ? FColor::Green : FColor::Yellow;
	DrawDebugCone(GetWorld(), Center, Forward, MaxLockOnDistance, 
		FMath::DegreesToRadians(MaxLockOnAngle), FMath::DegreesToRadians(MaxLockOnAngle), 
		12, ConeColor, false, -1.0f, 0, 2.0f);

	// 绘制当前目标
	if (CurrentTarget.IsValid())
	{
		DrawDebugSphere(GetWorld(), GetTargetLockLocation(), 50.0f, 12, FColor::Red, false, -1.0f, 0, 2.0f);
		DrawDebugLine(GetWorld(), GetEyeLocation(), GetTargetLockLocation(), FColor::Red, false, -1.0f, 0, 2.0f);
	}
}

TArray<FLockOnTargetInfo> UTargetingSystemComponent::GetSortedTargetsByAngle() const
{
	TArray<FLockOnTargetInfo> Result;

	TArray<AActor*> AllTargets = GetAllLockableTargets();
	for (AActor* Target : AllTargets)
	{
		FLockOnTargetInfo Info;
		Info.TargetActor = Target;
		Info.Distance = GetTargetDistance(Target);
		Info.Angle = GetTargetAngle(Target);
		Info.bIsVisible = CheckTargetVisibility(Target);

		Result.Add(Info);
	}

	// 按角度排序（从小到大）
	Result.Sort([](const FLockOnTargetInfo& A, const FLockOnTargetInfo& B)
	{
		return A.Angle < B.Angle;
	});

	return Result;
}
