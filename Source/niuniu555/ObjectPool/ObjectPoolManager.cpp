// Copyright Epic Games, Inc. All Rights Reserved.

#include "ObjectPoolManager.h"
#include "ObjectPoolBase.h"

void UObjectPoolManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Pools.Empty();
	UE_LOG(LogTemp, Log, TEXT("ObjectPoolManager: Initialized"));
}

void UObjectPoolManager::Deinitialize()
{
	ClearAllPools();
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("ObjectPoolManager: Deinitialized"));
}

void UObjectPoolManager::RegisterPool(EObjectPoolType PoolType, UObjectPoolBase* Pool)
{
	if (Pool)
	{
		Pools.Add(PoolType, Pool);
		UE_LOG(LogTemp, Log, TEXT("ObjectPoolManager: Registered pool %d"), (int32)PoolType);
	}
}

void UObjectPoolManager::UnregisterPool(EObjectPoolType PoolType)
{
	if (UObjectPoolBase* Pool = Pools.FindRef(PoolType))
	{
		Pool->Clear();
		Pools.Remove(PoolType);
	}
}

UObject* UObjectPoolManager::GetObject(EObjectPoolType PoolType)
{
	if (UObjectPoolBase* Pool = Pools.FindRef(PoolType))
	{
		return Pool->GetObject();
	}
	return nullptr;
}

void UObjectPoolManager::ReturnObject(EObjectPoolType PoolType, UObject* Object)
{
	if (UObjectPoolBase* Pool = Pools.FindRef(PoolType))
	{
		Pool->ReturnObject(Object);
	}
}

void UObjectPoolManager::ClearAllPools()
{
	for (auto& Pair : Pools)
	{
		if (Pair.Value)
		{
			Pair.Value->Clear();
		}
	}
	Pools.Empty();
}

UObjectPoolBase* UObjectPoolManager::GetPool(EObjectPoolType PoolType) const
{
	return Pools.FindRef(PoolType);
}
