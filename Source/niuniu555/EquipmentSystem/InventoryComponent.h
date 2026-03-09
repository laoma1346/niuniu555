#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentTypes.h"
#include "InventoryComponent.generated.h"

// 背包系统日志
DECLARE_LOG_CATEGORY_EXTERN(LogInventorySystem, Log, All);

class AEquipmentBase;
class UEquipmentManagerComponent;

/**
 * 背包物品结构体
 */
USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "背包物品")
	AEquipmentBase* Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "背包物品")
	int32 Quantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "背包物品")
	int32 SlotIndex;
};

/**
 * 背包管理器组件
 * 用于管理角色的物品存储、分类和操作
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent, DisplayName = "背包组件"))
class NIUNIU555_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * 背包容量
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "背包设置")
	int32 InventoryCapacity = 20;

	/**
	 * 背包物品列表
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "背包物品")
	TArray<FInventoryItem> InventoryItems;

	/**
	 * 装备管理器引用
	 */
	UPROPERTY(BlueprintReadOnly, Category = "装备管理")
	UEquipmentManagerComponent* EquipmentManager;

	/**
	 * 背包所有者
	 */
	UPROPERTY(BlueprintReadOnly, Category = "背包")
	AActor* OwnerActor;

	/**
	 * 添加物品到背包
	 * @param Item 物品
	 * @param Quantity 数量
	 * @return 是否添加成功
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	bool AddItem(AEquipmentBase* Item, int32 Quantity = 1);

	/**
	 * 从背包移除物品
	 * @param Item 物品
	 * @param Quantity 数量
	 * @return 实际移除的数量
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	int32 RemoveItem(AEquipmentBase* Item, int32 Quantity = 1);

	/**
	 * 从背包移除指定槽位的物品
	 * @param SlotIndex 槽位索引
	 * @param Quantity 数量
	 * @return 实际移除的数量
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	int32 RemoveItemFromSlot(int32 SlotIndex, int32 Quantity = 1);

	/**
	 * 获取背包中的物品
	 * @param SlotIndex 槽位索引
	 * @return 物品
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	FInventoryItem GetItemAtSlot(int32 SlotIndex) const;

	/**
	 * 检查背包是否有空间
	 * @return 是否有空间
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	bool HasSpace() const;

	/**
	 * 获取背包剩余空间
	 * @return 剩余空间
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	int32 GetRemainingSpace() const;

	/**
	 * 获取背包中的物品数量
	 * @return 物品数量
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	int32 GetItemCount() const;

	/**
	 * 查找物品在背包中的位置
	 * @param Item 物品
	 * @return 槽位索引，如果不存在返回-1
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	int32 FindItemSlot(AEquipmentBase* Item) const;

	/**
	 * 整理背包
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	void SortInventory();

	/**
	 * 按品质排序背包
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	void SortByQuality();

	/**
	 * 按类型排序背包
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	void SortByType();

	/**
	 * 清空背包
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	void ClearInventory();

	/**
	 * 装备背包中的物品
	 * @param SlotIndex 背包槽位索引
	 * @param EquipmentSlot 装备槽位
	 * @return 是否装备成功
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	bool EquipItemFromInventory(int32 SlotIndex, EEquipmentSlot EquipmentSlot);

	/**
	 * 卸下装备到背包
	 * @param EquipmentSlot 装备槽位
	 * @return 是否卸下成功
	 */
	UFUNCTION(BlueprintCallable, Category = "背包管理")
	bool UnequipItemToInventory(EEquipmentSlot EquipmentSlot);

protected:
	/**
	 * 初始化背包
	 */
	void InitializeInventory();

	/**
	 * 查找空槽位
	 * @return 空槽位索引，如果没有返回-1
	 */
	int32 FindEmptySlot() const;

	/**
	 * 查找相同物品的槽位
	 * @param Item 物品
	 * @return 槽位索引，如果不存在返回-1
	 */
	int32 FindSameItemSlot(AEquipmentBase* Item) const;

	/**
	 * 刷新背包槽位索引
	 */
	void RefreshSlotIndices();
};
