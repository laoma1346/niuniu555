// Copyright Epic Games, Inc. All Rights Reserved.

#include "ObjectPoolBase.h"

void UObjectPoolBase::Initialize(int32 InPoolSize, TSubclassOf<UObject> InObjectClass)
{
	ObjectClass = InObjectClass;
	AvailableObjects.Empty();
	InUseObjects.Empty();

	if (ObjectClass)
	{
		PreWarm(InPoolSize);
		UE_LOG(LogTemp, Log, TEXT("ObjectPool initialized with class %s, size %d"), 
			*ObjectClass->GetName(), InPoolSize);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectPool initialized without valid class!"));
	}
}

UObject* UObjectPoolBase::GetObject()
{
	UObject* Object = nullptr;

	if (AvailableObjects.Num() > 0)
	{
		Object = AvailableObjects.Pop();
	}
	else
	{
		Object = CreateNewObject();
	}

	if (Object)
	{
		InUseObjects.Add(Object);

		// 调用接口方法
		if (IPoolableObject* Poolable = Cast<IPoolableObject>(Object))
		{
			Poolable->Execute_OnSpawnFromPool(Object);
		}
	}

	return Object;
}

void UObjectPoolBase::ReturnObject(UObject* Object)
{
	if (!Object || !InUseObjects.Contains(Object))
	{
		return;
	}

	InUseObjects.Remove(Object);

	// 调用接口方法
	if (IPoolableObject* Poolable = Cast<IPoolableObject>(Object))
	{
		Poolable->Execute_OnReturnToPool(Object);
	}

	AvailableObjects.Add(Object);
}

void UObjectPoolBase::PreWarm(int32 Count)
{
	for (int32 i = 0; i < Count; ++i)
	{
		UObject* NewObject = CreateNewObject();
		if (NewObject)
		{
			AvailableObjects.Add(NewObject);
		}
	}
}

void UObjectPoolBase::Clear()
{
	AvailableObjects.Empty();
	InUseObjects.Empty();
}

UObject* UObjectPoolBase::CreateNewObject()
{
	if (ObjectClass)
	{
		return NewObject<UObject>(GetOuter(), ObjectClass);
	}
	return nullptr;
}
