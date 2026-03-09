#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentTypes.h"
#include "EquipmentManagerComponent.generated.h"

class AEquipmentBase;

/**
 * 装备管理器组件
 * 用于管理角色的装备槽位和装备的穿戴状态
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Equipment), meta = (BlueprintSpawnableComponent, DisplayName = "装备管理器"))
class NIUNIU555_API UEquipmentManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipmentManagerComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * 装备槽位映射
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备槽位")
	TMap<EEquipmentSlot, AEquipmentBase*> EquippedItems;

	/**
	 * 装备角色
	 */
	UPROPERTY(BlueprintReadOnly, Category = "装备")
	AActor* OwnerActor;

	/**
	 * 装备物品到指定槽位
	 * @param Equipment 装备物品
	 * @param Slot 装备槽位
	 * @return 是否装备成功
	 */
	UFUNCTION(BlueprintCallable, Category = "装备管理")
	bool EquipItem(AEquipmentBase* Equipment, EEquipmentSlot Slot);

	/**
	 * 从槽位卸下装备
	 * @param Slot 装备槽位
	 * @return 卸下的装备
	 */
	UFUNCTION(BlueprintCallable, Category = "装备管理")
	AEquipmentBase* UnequipItem(EEquipmentSlot Slot);

	/**
	 * 获取指定槽位的装备
	 * @param Slot 装备槽位
	 * @return 装备物品
	 */
	UFUNCTION(BlueprintCallable, Category = "装备管理")
	AEquipmentBase* GetEquippedItem(EEquipmentSlot Slot) const;

	/**
	 * 检查槽位是否已装备
	 * @param Slot 装备槽位
	 * @return 是否已装备
	 */
	UFUNCTION(BlueprintCallable, Category = "装备管理")
	bool IsSlotEquipped(EEquipmentSlot Slot) const;

	/**
	 * 获取所有已装备的物品
	 * @return 已装备的物品列表
	 */
	UFUNCTION(BlueprintCallable, Category = "装备管理")
	TArray<AEquipmentBase*> GetAllEquippedItems() const;

	/**
	 * 计算套装效果
	 * @return 套装效果列表
	 */
	UFUNCTION(BlueprintCallable, Category = "装备管理")
	TMap<FString, int32> CalculateSetBonuses() const;

	/**
	 * 应用所有装备的属性
	 */
	UFUNCTION(BlueprintCallable, Category = "装备管理")
	void ApplyAllEquipmentStats();

	/**
	 * 移除所有装备的属性
	 */
	UFUNCTION(BlueprintCallable, Category = "装备管理")
	void RemoveAllEquipmentStats();

	/**
	 * 刷新装备属性
	 */
	UFUNCTION(BlueprintCallable, Category = "装备管理")
	void RefreshEquipmentStats();

protected:
	/**
	 * 初始化装备槽位
	 */
	void InitializeEquipmentSlots();

	/**
	 * 检查装备是否可以装备到指定槽位
	 * @param Equipment 装备物品
	 * @param Slot 装备槽位
	 * @return 是否可以装备
	 */
	bool CanEquipToSlot(AEquipmentBase* Equipment, EEquipmentSlot Slot) const;

	/**
	 * 处理装备替换逻辑
	 * @param OldEquipment 旧装备
	 * @param NewEquipment 新装备
	 * @param Slot 装备槽位
	 * @return 是否替换成功
	 */
	bool HandleEquipmentReplacement(AEquipmentBase* OldEquipment, AEquipmentBase* NewEquipment, EEquipmentSlot Slot);
};
