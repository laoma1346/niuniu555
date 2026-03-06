// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ObjectPoolBase.generated.h"

// 对象池接口
UINTERFACE(MinimalAPI)
class UPoolableObject : public UInterface
{
	GENERATED_BODY()
};

class NIUNIU555_API IPoolableObject
{
	GENERATED_BODY()

public:
	// 对象被取出时调用
	UFUNCTION(BlueprintNativeEvent, Category = "ObjectPool")
	void OnSpawnFromPool();

	// 对象被回收时调用
	UFUNCTION(BlueprintNativeEvent, Category = "ObjectPool")
	void OnReturnToPool();
};

/**
 * 对象池基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class NIUNIU555_API UObjectPoolBase : public UObject
{
	GENERATED_BODY()

public:
	// 初始化对象池
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	virtual void Initialize(int32 InPoolSize, TSubclassOf<UObject> InObjectClass);

	// 获取对象
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	virtual UObject* GetObject();

	// 归还对象
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	virtual void ReturnObject(UObject* Object);

	// 预生成对象
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	virtual void PreWarm(int32 Count);

	// 清空对象池
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	virtual void Clear();

	// 获取池中可用对象数量
	UFUNCTION(BlueprintPure, Category = "ObjectPool")
	int32 GetAvailableCount() const { return AvailableObjects.Num(); }

	// 获取池中使用中对象数量
	UFUNCTION(BlueprintPure, Category = "ObjectPool")
	int32 GetInUseCount() const { return InUseObjects.Num(); }

protected:
	// 对象类
	UPROPERTY()
	TSubclassOf<UObject> ObjectClass;

	// 可用对象队列
	UPROPERTY()
	TArray<TObjectPtr<UObject>> AvailableObjects;

	// 使用中对象集合
	UPROPERTY()
	TSet<TObjectPtr<UObject>> InUseObjects;

	// 创建新对象
	virtual UObject* CreateNewObject();
};
