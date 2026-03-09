#include "EquipmentManagerComponent.h"
#include "EquipmentBase.h"

UEquipmentManagerComponent::UEquipmentManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 初始化装备槽位
	InitializeEquipmentSlots();
}

void UEquipmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// 获取所有者
	OwnerActor = GetOwner();

	// 应用初始装备属性
	ApplyAllEquipmentStats();
}

void UEquipmentManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UEquipmentManagerComponent::InitializeEquipmentSlots()
{
	// 初始化所有装备槽位
	EquippedItems.Empty();

	// 添加所有槽位
	EquippedItems.Add(EEquipmentSlot::MainHand, nullptr);
	EquippedItems.Add(EEquipmentSlot::OffHand, nullptr);
	EquippedItems.Add(EEquipmentSlot::Helmet, nullptr);
	EquippedItems.Add(EEquipmentSlot::Chest, nullptr);
	EquippedItems.Add(EEquipmentSlot::Legs, nullptr);
	EquippedItems.Add(EEquipmentSlot::Accessory1, nullptr);
	EquippedItems.Add(EEquipmentSlot::Accessory2, nullptr);
}

bool UEquipmentManagerComponent::EquipItem(AEquipmentBase* Equipment, EEquipmentSlot Slot)
{
	if (!Equipment || !OwnerActor)
	{
		return false;
	}

	// 检查是否可以装备
	if (!CanEquipToSlot(Equipment, Slot))
	{
		return false;
	}

	// 检查槽位是否已有装备
	AEquipmentBase* OldEquipment = GetEquippedItem(Slot);

	// 处理装备替换
	if (OldEquipment)
	{
		if (!HandleEquipmentReplacement(OldEquipment, Equipment, Slot))
		{
			return false;
		}
	}
	else
	{
		// 直接装备
		if (Equipment->EquipToSlot(Slot, OwnerActor))
		{
			EquippedItems[Slot] = Equipment;
			RefreshEquipmentStats();
			return true;
		}
	}

	return false;
}

AEquipmentBase* UEquipmentManagerComponent::UnequipItem(EEquipmentSlot Slot)
{
	AEquipmentBase* Equipment = GetEquippedItem(Slot);

	if (Equipment)
	{
		if (Equipment->UnequipFromSlot())
		{
			EquippedItems[Slot] = nullptr;
			RefreshEquipmentStats();
			return Equipment;
		}
	}

	return nullptr;
}

AEquipmentBase* UEquipmentManagerComponent::GetEquippedItem(EEquipmentSlot Slot) const
{
	// 直接查找并返回值
	for (const TPair<EEquipmentSlot, AEquipmentBase*>& Pair : EquippedItems)
	{
		if (Pair.Key == Slot)
		{
			return Pair.Value;
		}
	}
	return nullptr;
}

bool UEquipmentManagerComponent::IsSlotEquipped(EEquipmentSlot Slot) const
{
	AEquipmentBase* Equipment = GetEquippedItem(Slot);
	return Equipment != nullptr;
}

TArray<AEquipmentBase*> UEquipmentManagerComponent::GetAllEquippedItems() const
{
	TArray<AEquipmentBase*> EquippedList;

	for (const TPair<EEquipmentSlot, AEquipmentBase*>& Pair : EquippedItems)
	{
		if (Pair.Value)
		{
			EquippedList.Add(Pair.Value);
		}
	}

	return EquippedList;
}

TMap<FString, int32> UEquipmentManagerComponent::CalculateSetBonuses() const
{
	TMap<FString, int32> SetPieceCounts;

	// 统计每个套装的件数
	for (const TPair<EEquipmentSlot, AEquipmentBase*>& Pair : EquippedItems)
	{
		AEquipmentBase* Equipment = Pair.Value;
		if (Equipment && !Equipment->InstanceData.SetInfo.SetID.IsEmpty())
		{
			int32* Count = SetPieceCounts.Find(Equipment->InstanceData.SetInfo.SetID);
			int32 CurrentCount = Count ? *Count : 0;
			SetPieceCounts.Add(Equipment->InstanceData.SetInfo.SetID, CurrentCount + 1);
		}
	}

	return SetPieceCounts;
}

void UEquipmentManagerComponent::ApplyAllEquipmentStats()
{
	if (!OwnerActor)
	{
		return;
	}

	// 应用所有装备的属性
	for (const TPair<EEquipmentSlot, AEquipmentBase*>& Pair : EquippedItems)
	{
		AEquipmentBase* Equipment = Pair.Value;
		if (Equipment)
		{
			Equipment->ApplyEquipmentStats(OwnerActor);
		}
	}
}

void UEquipmentManagerComponent::RemoveAllEquipmentStats()
{
	if (!OwnerActor)
	{
		return;
	}

	// 移除所有装备的属性
	for (const TPair<EEquipmentSlot, AEquipmentBase*>& Pair : EquippedItems)
	{
		AEquipmentBase* Equipment = Pair.Value;
		if (Equipment)
		{
			Equipment->RemoveEquipmentStats(OwnerActor);
		}
	}
}

void UEquipmentManagerComponent::RefreshEquipmentStats()
{
	// 先移除所有属性，再重新应用
	RemoveAllEquipmentStats();
	ApplyAllEquipmentStats();
}

bool UEquipmentManagerComponent::CanEquipToSlot(AEquipmentBase* Equipment, EEquipmentSlot Slot) const
{
	if (!Equipment || !OwnerActor)
	{
		return false;
	}

	// 检查槽位是否匹配
	if (Equipment->InstanceData.Slot != Slot)
	{
		return false;
	}

	// 检查装备是否已经装备
	if (Equipment->InstanceData.bIsEquipped)
	{
		return false;
	}

	// 检查装备是否损坏
	if (Equipment->InstanceData.Durability <= 0)
	{
		return false;
	}

	return true;
}

bool UEquipmentManagerComponent::HandleEquipmentReplacement(AEquipmentBase* OldEquipment, AEquipmentBase* NewEquipment, EEquipmentSlot Slot)
{
	// 先卸下旧装备
	if (OldEquipment->UnequipFromSlot())
	{
		// 再装备新装备
		if (NewEquipment->EquipToSlot(Slot, OwnerActor))
		{
			EquippedItems[Slot] = NewEquipment;
			RefreshEquipmentStats();
			return true;
		}
		else
		{
			// 新装备装备失败，重新装备旧装备
			OldEquipment->EquipToSlot(Slot, OwnerActor);
			EquippedItems[Slot] = OldEquipment;
		}
	}

	return false;
}
