#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "EquipmentTypes.h"
#include "EquipmentBase.generated.h"

// 装备系统日志
DECLARE_LOG_CATEGORY_EXTERN(LogEquipmentSystem, Log, All);

class UEquipmentDataAsset;
class USkeletalMeshComponent;

/**
 * 装备基类
 * 表示游戏中的装备实例
 */
UCLASS(Blueprintable, BlueprintType)
class NIUNIU555_API AEquipmentBase : public AActor
{
	GENERATED_BODY()

public:
	AEquipmentBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/**
	 * 装备数据资产
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备数据")
	UEquipmentDataAsset* EquipmentData;

	/**
	 * 装备实例数据
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	FEquipmentInstanceData InstanceData;

	/**
	 * 装备网格组件
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "组件")
	USkeletalMeshComponent* EquipmentMeshComponent;

	/**
	 * 碰撞组件
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "组件")
	UCapsuleComponent* CollisionComponent;

	/**
	 * 装备到指定槽位
	 * @param InSlot 装备槽位
	 * @param InOwner 装备所有者
	 * @return 是否装备成功
	 */
	UFUNCTION(BlueprintCallable, Category = "装备")
	bool EquipToSlot(EEquipmentSlot InSlot, AActor* InOwner);

	/**
	 * 从槽位卸下
	 * @return 是否卸下成功
	 */
	UFUNCTION(BlueprintCallable, Category = "装备")
	bool UnequipFromSlot();

	/**
	 * 应用装备属性
	 * @param Target 目标对象
	 */
	UFUNCTION(BlueprintCallable, Category = "装备")
	void ApplyEquipmentStats(AActor* Target);

	/**
	 * 移除装备属性
	 * @param Target 目标对象
	 */
	UFUNCTION(BlueprintCallable, Category = "装备")
	void RemoveEquipmentStats(AActor* Target);

	/**
	 * 消耗耐久度
	 * @param Amount 消耗数量
	 * @return 是否耐久度耗尽
	 */
	UFUNCTION(BlueprintCallable, Category = "装备")
	bool ConsumeDurability(int32 Amount);

	/**
	 * 修理装备
	 * @param GoldAmount 投入的金币
	 * @return 修理后的耐久度
	 */
	UFUNCTION(BlueprintCallable, Category = "装备")
	int32 RepairEquipment(int32 GoldAmount);

	/**
	 * 分解装备
	 * @return 获得的材料数量
	 */
	UFUNCTION(BlueprintCallable, Category = "装备")
	int32 ScrapEquipment();

	/**
	 * 获取装备总属性
	 * @return 装备总属性
	 */
	UFUNCTION(BlueprintCallable, Category = "装备")
	FEquipmentBaseStats GetTotalStats() const;

	/**
	 * 装备所有者
	 */
	UPROPERTY(BlueprintReadOnly, Category = "装备")
	AActor* OwnerActor;

	/**
	 * 当前装备的槽位
	 */
	UPROPERTY(BlueprintReadOnly, Category = "装备")
	EEquipmentSlot CurrentSlot;

protected:
	/**
	 * 播放装备音效
	 */
	void PlayEquipSound();

	/**
	 * 播放卸下音效
	 */
	void PlayUnequipSound();

	/**
	 * 更新装备外观
	 */
	void UpdateEquipmentAppearance();

	/**
	 * 检查装备是否可以装备
	 * @param InSlot 装备槽位
	 * @param InOwner 装备所有者
	 * @return 是否可以装备
	 */
	bool CanEquipToSlot(EEquipmentSlot InSlot, AActor* InOwner) const;

	/**
	 * 检查装备是否可以卸下
	 * @return 是否可以卸下
	 */
	bool CanUnequipFromSlot() const;

public:
	/**
	 * 从数据资产生成装备实例【静态工厂方法】
	 * @param WorldContext 世界上下文对象（通常传入GetWorld()或任意 UObject）
	 * @param DataAsset 装备数据资产
	 * @param SpawnLocation 生成位置
	 * @param SpawnRotation 生成旋转
	 * @return 生成的装备实例
	 */
	UFUNCTION(BlueprintCallable, Category = "装备|工厂", meta = (WorldContext = "WorldContext"))
	static AEquipmentBase* SpawnEquipmentFromDataAsset(UObject* WorldContext, UEquipmentDataAsset* DataAsset, 
		const FVector& SpawnLocation = FVector::ZeroVector, const FRotator& SpawnRotation = FRotator::ZeroRotator);

	/**
	 * 初始化装备实例数据
	 * @param DataAsset 装备数据资产
	 */
	UFUNCTION(BlueprintCallable, Category = "装备")
	void InitializeFromDataAsset(UEquipmentDataAsset* DataAsset);

	/**
	 * 获取装备当前耐久度百分比
	 * @return 耐久度百分比 (0.0 - 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "装备")
	float GetDurabilityPercent() const;

	/**
	 * 检查装备是否已损坏
	 * @return 是否损坏
	 */
	UFUNCTION(BlueprintPure, Category = "装备")
	bool IsBroken() const;
};
