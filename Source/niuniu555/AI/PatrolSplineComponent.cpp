// Copyright Epic Games, Inc. All Rights Reserved.

#include "PatrolSplineComponent.h"
#include "Components/SplineComponent.h"

UPatrolSplineComponent::UPatrolSplineComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentPointIndex = 0;
    Direction = 1;
    bLoop = true;
    bPingPong = true;
    PointAcceptanceRadius = 100.0f;
}

FVector UPatrolSplineComponent::GetNextPatrolPoint()
{
    if (!SplineComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PatrolSplineComponent] 没有配置样条组件"));
        return GetOwner()->GetActorLocation();
    }

    int32 NumPoints = SplineComponent->GetNumberOfSplinePoints();
    if (NumPoints == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PatrolSplineComponent] 样条没有点"));
        return GetOwner()->GetActorLocation();
    }

    // 获取当前点位置
    FVector CurrentLocation = GetCurrentPointLocation();

    // 计算下一个点索引
    int32 NextIndex = CurrentPointIndex + Direction;

    // 检查是否到达边界
    if (NextIndex < 0 || NextIndex >= NumPoints)
    {
        if (bPingPong)
        {
            // 往返模式：反向
            Direction *= -1;
            NextIndex = CurrentPointIndex + Direction;
            
            // 确保索引有效
            if (NextIndex < 0)
            {
                NextIndex = 0;
                Direction = 1;
            }
            else if (NextIndex >= NumPoints)
            {
                NextIndex = NumPoints - 1;
                Direction = -1;
            }
        }
        else if (bLoop)
        {
            // 循环模式：回到起点
            if (NextIndex < 0)
            {
                NextIndex = NumPoints - 1;
            }
            else
            {
                NextIndex = 0;
            }
        }
        else
        {
            // 不循环：停止在终点
            OnPatrolCompleted.Broadcast();
            return CurrentLocation;
        }
    }

    CurrentPointIndex = NextIndex;

    // 广播事件
    FVector PointLocation = GetCurrentPointLocation();
    OnPatrolPointReached.Broadcast(CurrentPointIndex, PointLocation);

    return PointLocation;
}

FVector UPatrolSplineComponent::GetCurrentPointLocation() const
{
    if (!SplineComponent)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }

    return SplineComponent->GetLocationAtSplinePoint(CurrentPointIndex, ESplineCoordinateSpace::World);
}

void UPatrolSplineComponent::SetSplineComponent(USplineComponent* Spline)
{
    SplineComponent = Spline;
    CurrentPointIndex = 0;
    Direction = 1;
}

bool UPatrolSplineComponent::IsAtEnd() const
{
    if (!SplineComponent)
    {
        return true;
    }

    int32 NumPoints = SplineComponent->GetNumberOfSplinePoints();
    if (NumPoints == 0)
    {
        return true;
    }

    if (Direction > 0)
    {
        return CurrentPointIndex >= NumPoints - 1;
    }
    else
    {
        return CurrentPointIndex <= 0;
    }
}

void UPatrolSplineComponent::ReverseDirection()
{
    Direction *= -1;
}

void UPatrolSplineComponent::ResetToStart()
{
    CurrentPointIndex = 0;
    Direction = 1;
}

float UPatrolSplineComponent::GetPatrolProgress() const
{
    if (!SplineComponent)
    {
        return 0.0f;
    }

    int32 NumPoints = SplineComponent->GetNumberOfSplinePoints();
    if (NumPoints <= 1)
    {
        return 1.0f;
    }

    return static_cast<float>(CurrentPointIndex) / static_cast<float>(NumPoints - 1);
}
