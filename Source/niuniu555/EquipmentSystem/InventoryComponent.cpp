#include "InventoryComponent.h"
#include "EquipmentBase.h"
#include "EquipmentManagerComponent.h"

DEFINE_LOG_CATEGORY(LogInventorySystem);

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 初始化背包
	InitializeInventory();
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 获取所有者
	OwnerActor = GetOwner();

	// 获取装备管理器
	if (OwnerActor)
	{
		EquipmentManager = OwnerActor->FindComponentByClass<UEquipmentManagerComponent>();
	}
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UInventoryComponent::InitializeInventory()
{
	// 清空背包
	InventoryItems.Empty();
}

bool UInventoryComponent::AddItem(AEquipmentBase* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		return false;
	}

	// 检查背包是否有空间
	if (!HasSpace())
	{
		return false;
	}

	// 【装备不可叠加】每个装备实例占用一个独立槽位
	// 即使相同BaseID的装备也是不同实例，不能叠加
	
	// 查找空槽位
	int32 EmptySlot = FindEmptySlot();

	if (EmptySlot != -1)
	{
		// 添加新物品
		FInventoryItem NewItem;
		NewItem.Item = Item;
		NewItem.Quantity = 1;  // 装备固定数量为1
		NewItem.SlotIndex = EmptySlot;
		InventoryItems.Add(NewItem);
		RefreshSlotIndices();
		
		// 禁用物品的物理和碰撞（已进入背包）
		Item->SetActorHiddenInGame(true);
		Item->SetActorEnableCollision(false);
		
		// 【日志】添加成功
		FString ItemName = Item->InstanceData.EquipmentName.IsEmpty() ? TEXT("未知装备") : Item->InstanceData.EquipmentName;
		UE_LOG(LogInventorySystem, Log, TEXT("[Inventory] 添加装备成功: %s 到槽位 %d"), *ItemName, EmptySlot);
		
		return true;
	}

	UE_LOG(LogInventorySystem, Warning, TEXT("[Inventory] 添加装备失败：背包已满"));
	return false;
}

int32 UInventoryComponent::RemoveItem(AEquipmentBase* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		return 0;
	}

	int32 RemovedQuantity = 0;
	int32 ItemSlot = FindItemSlot(Item);

	if (ItemSlot != -1)
	{
		FInventoryItem& InventoryItem = InventoryItems[ItemSlot];
		int32 RemoveAmount = FMath::Min(Quantity, InventoryItem.Quantity);
		
		InventoryItem.Quantity -= RemoveAmount;
		RemovedQuantity = RemoveAmount;

		// 如果数量为0，移除物品
		if (InventoryItem.Quantity <= 0)
		{
			InventoryItems.RemoveAt(ItemSlot);
			RefreshSlotIndices();
		}
	}

	return RemovedQuantity;
}

int32 UInventoryComponent::RemoveItemFromSlot(int32 SlotIndex, int32 Quantity)
{
	if (SlotIndex < 0 || SlotIndex >= InventoryItems.Num() || Quantity <= 0)
	{
		return 0;
	}

	FInventoryItem& InventoryItem = InventoryItems[SlotIndex];
	int32 RemoveAmount = FMath::Min(Quantity, InventoryItem.Quantity);
	
	InventoryItem.Quantity -= RemoveAmount;

	// 如果数量为0，移除物品
	if (InventoryItem.Quantity <= 0)
	{
		InventoryItems.RemoveAt(SlotIndex);
		RefreshSlotIndices();
	}

	return RemoveAmount;
}

FInventoryItem UInventoryComponent::GetItemAtSlot(int32 SlotIndex) const
{
	if (SlotIndex >= 0 && SlotIndex < InventoryItems.Num())
	{
		return InventoryItems[SlotIndex];
	}

	FInventoryItem EmptyItem;
	EmptyItem.SlotIndex = -1;
	return EmptyItem;
}

bool UInventoryComponent::HasSpace() const
{
	return InventoryItems.Num() < InventoryCapacity;
}

int32 UInventoryComponent::GetRemainingSpace() const
{
	return InventoryCapacity - InventoryItems.Num();
}

int32 UInventoryComponent::GetItemCount() const
{
	return InventoryItems.Num();
}

int32 UInventoryComponent::FindItemSlot(AEquipmentBase* Item) const
{
	for (int32 i = 0; i < InventoryItems.Num(); i++)
	{
		if (InventoryItems[i].Item == Item)
		{
			return i;
		}
	}

	return -1;
}

void UInventoryComponent::SortInventory()
{
	// 按品质和类型排序
	InventoryItems.Sort([](const FInventoryItem& A, const FInventoryItem& B)
	{
		// 先按品质排序
		if (A.Item && B.Item)
		{
			if (A.Item->InstanceData.Quality != B.Item->InstanceData.Quality)
			{
				return (uint8)A.Item->InstanceData.Quality > (uint8)B.Item->InstanceData.Quality;
			}

			// 再按类型排序
			if (A.Item->InstanceData.Type != B.Item->InstanceData.Type)
			{
				return (uint8)A.Item->InstanceData.Type < (uint8)B.Item->InstanceData.Type;
			}

			// 最后按名称排序
			return A.Item->InstanceData.EquipmentName < B.Item->InstanceData.EquipmentName;
		}

		return false;
	});

	RefreshSlotIndices();
}

void UInventoryComponent::SortByQuality()
{
	// 按品质排序
	InventoryItems.Sort([](const FInventoryItem& A, const FInventoryItem& B)
	{
		if (A.Item && B.Item)
		{
			return (uint8)A.Item->InstanceData.Quality > (uint8)B.Item->InstanceData.Quality;
		}

		return false;
	});

	RefreshSlotIndices();
}

void UInventoryComponent::SortByType()
{
	// 按类型排序
	InventoryItems.Sort([](const FInventoryItem& A, const FInventoryItem& B)
	{
		if (A.Item && B.Item)
		{
			if (A.Item->InstanceData.Type != B.Item->InstanceData.Type)
			{
				return (uint8)A.Item->InstanceData.Type < (uint8)B.Item->InstanceData.Type;
			}

			// 再按品质排序
			return (uint8)A.Item->InstanceData.Quality > (uint8)B.Item->InstanceData.Quality;
		}

		return false;
	});

	RefreshSlotIndices();
}

void UInventoryComponent::ClearInventory()
{
	InventoryItems.Empty();
}

bool UInventoryComponent::EquipItemFromInventory(int32 SlotIndex, EEquipmentSlot EquipmentSlot)
{
	if (!EquipmentManager || SlotIndex < 0 || SlotIndex >= InventoryItems.Num())
	{
		return false;
	}

	FInventoryItem& InventoryItem = InventoryItems[SlotIndex];

	// 装备物品
	if (EquipmentManager->EquipItem(InventoryItem.Item, EquipmentSlot))
	{
		// 从背包移除物品
		FString ItemName = InventoryItem.Item ? InventoryItem.Item->InstanceData.EquipmentName : TEXT("未知装备");
		UE_LOG(LogInventorySystem, Log, TEXT("[Inventory] 装备成功: %s 从背包槽位 %d 装备到装备槽位 %d"), 
			*ItemName, SlotIndex, (int32)EquipmentSlot);
		
		RemoveItemFromSlot(SlotIndex, 1);
		return true;
	}

	UE_LOG(LogInventorySystem, Warning, TEXT("[Inventory] 装备失败：无法装备到槽位 %d"), (int32)EquipmentSlot);
	return false;
}

bool UInventoryComponent::UnequipItemToInventory(EEquipmentSlot EquipmentSlot)
{
	if (!EquipmentManager || !HasSpace())
	{
		return false;
	}

	// 卸下装备
	AEquipmentBase* Equipment = EquipmentManager->UnequipItem(EquipmentSlot);

	if (Equipment)
	{
		// 添加到背包
		return AddItem(Equipment, 1);
	}

	return false;
}

int32 UInventoryComponent::FindEmptySlot() const
{
	if (InventoryItems.Num() < InventoryCapacity)
	{
		return InventoryItems.Num();
	}

	return -1;
}

int32 UInventoryComponent::FindSameItemSlot(AEquipmentBase* Item) const
{
	// 安全：检查输入是否有效
	if (!Item || !Item->EquipmentData)
	{
		return -1;
	}

	// 这里简化处理，实际应该比较物品的BaseID
	// 因为不同实例可能是相同类型的物品
	for (int32 i = 0; i < InventoryItems.Num(); i++)
	{
		// 安全：检查物品是否有效
		if (InventoryItems[i].Item && InventoryItems[i].Item->EquipmentData)
		{
			if (InventoryItems[i].Item->EquipmentData == Item->EquipmentData)
			{
				return i;
			}
		}
	}

	return -1;
}

void UInventoryComponent::RefreshSlotIndices()
{
	// 刷新所有槽位索引
	for (int32 i = 0; i < InventoryItems.Num(); i++)
	{
		InventoryItems[i].SlotIndex = i;
	}
}
