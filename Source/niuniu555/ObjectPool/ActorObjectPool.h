// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ObjectPoolBase.h"
#include "ActorObjectPool.generated.h"

/**
 * Actor对象池 - 用于管理Actor对象（特效、伤害数字等）
 */
UCLASS(Blueprintable, BlueprintType)
class NIUNIU555_API UActorObjectPool : public UObjectPoolBase
{
	GENERATED_BODY()

public:
	// 初始化Actor对象池（World会自动获取，不需要传入）
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void InitializeActorPool(int32 InPoolSize, TSubclassOf<AActor> InActorClass);

	// 获取Actor
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	AActor* GetActor();

	// 归还指定Actor
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ReturnActor(AActor* Actor);

	// 归还任意一个使用中的Actor（不需要指定，自动选一个）
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ReturnAnyActor();

	// 设置父Actor（生成的Actor会附加到此Actor下）
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void SetParentActor(AActor* InParent) { ParentActor = InParent; }

protected:
	UPROPERTY()
	TObjectPtr<UWorld> World;

	UPROPERTY()
	TObjectPtr<AActor> ParentActor;

	UPROPERTY()
	TSubclassOf<AActor> ActorClass;

	// 重写创建方法
	virtual UObject* CreateNewObject() override;
};
