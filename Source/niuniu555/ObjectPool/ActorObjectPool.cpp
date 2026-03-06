// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActorObjectPool.h"

void UActorObjectPool::InitializeActorPool(int32 InPoolSize, TSubclassOf<AActor> InActorClass)
{
	ActorClass = InActorClass;
	// World 会在 CreateNewObject 中自动获取
	World = nullptr;

	// 调用基类初始化
	Initialize(InPoolSize, nullptr);
}

AActor* UActorObjectPool::GetActor()
{
	UObject* Object = GetObject();
	return Cast<AActor>(Object);
}

void UActorObjectPool::ReturnActor(AActor* Actor)
{
	if (!Actor)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReturnActor: Actor is null"));
		return;
	}

	// 检查Actor是否在当前使用中的集合
	if (!InUseObjects.Contains(Actor))
	{
		UE_LOG(LogTemp, Warning, TEXT("ReturnActor: Actor is not in use, cannot return"));
		return;
	}

	// 隐藏Actor
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);

	// 归还到池（调用基类方法）
	ReturnObject(Actor);

	UE_LOG(LogTemp, Log, TEXT("ReturnActor: Actor returned to pool. Available: %d, InUse: %d"), 
		GetAvailableCount(), GetInUseCount());
}

void UActorObjectPool::ReturnAnyActor()
{
	// 从使用中的集合取出一个Actor
	if (InUseObjects.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReturnAnyActor: No actors in use to return"));
		return;
	}

	// 获取第一个使用中的Actor
	AActor* ActorToReturn = nullptr;
	for (UObject* Obj : InUseObjects)
	{
		ActorToReturn = Cast<AActor>(Obj);
		if (ActorToReturn)
		{
			break;
		}
	}

	if (ActorToReturn)
	{
		ReturnActor(ActorToReturn);
		UE_LOG(LogTemp, Log, TEXT("ReturnAnyActor: Returned one actor. Available: %d, InUse: %d"),
			GetAvailableCount(), GetInUseCount());
	}
}

UObject* UActorObjectPool::CreateNewObject()
{
	// 如果 World 为空，尝试获取
	if (!World)
	{
		World = GetWorld();
	}

	if (World && ActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* NewActor = World->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (NewActor)
		{
			// 初始状态设置为隐藏
			NewActor->SetActorHiddenInGame(true);
			NewActor->SetActorEnableCollision(false);
			NewActor->SetActorTickEnabled(false);

			if (ParentActor)
			{
				NewActor->AttachToActor(ParentActor, FAttachmentTransformRules::KeepRelativeTransform);
			}
		}

		return NewActor;
	}
	return nullptr;
}
