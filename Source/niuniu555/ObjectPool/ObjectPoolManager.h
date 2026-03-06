// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ObjectPoolManager.generated.h"

class UObjectPoolBase;

// 对象池类型枚举
UENUM(BlueprintType)
enum class EObjectPoolType : uint8
{
	None			UMETA(DisplayName = "无"),
	VisualEffect	UMETA(DisplayName = "视觉特效"),
	DamageNumber	UMETA(DisplayName = "伤害数字"),
	DropItem		UMETA(DisplayName = "掉落物"),
	Projectile		UMETA(DisplayName = "弹道"),
	Enemy			UMETA(DisplayName = "敌人")
};

/**
 * 对象池管理器 - 管理所有对象池
 */
UCLASS()
class NIUNIU555_API UObjectPoolManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 注册对象池
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void RegisterPool(EObjectPoolType PoolType, UObjectPoolBase* Pool);

	// 注销对象池
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void UnregisterPool(EObjectPoolType PoolType);

	// 获取对象
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	UObject* GetObject(EObjectPoolType PoolType);

	// 归还对象
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ReturnObject(EObjectPoolType PoolType, UObject* Object);

	// 清空所有对象池
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ClearAllPools();

	// 获取对象池
	UFUNCTION(BlueprintPure, Category = "ObjectPool")
	UObjectPoolBase* GetPool(EObjectPoolType PoolType) const;

protected:
	UPROPERTY()
	TMap<EObjectPoolType, TObjectPtr<UObjectPoolBase>> Pools;
};
