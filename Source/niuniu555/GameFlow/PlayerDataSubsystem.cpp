#include "PlayerDataSubsystem.h"

void UPlayerDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// 初始化默认值
	CurrentGold = 0;
	CurrentFragments = 0;
	CurrentHealthPercent = 1.0f;
	CurrentManaPercent = 1.0f;
	
	UE_LOG(LogTemp, Log, TEXT("[PlayerDataSubsystem] 初始化完成"));
}

void UPlayerDataSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("[PlayerDataSubsystem] 反初始化"));
	Super::Deinitialize();
}

UPlayerDataSubsystem* UPlayerDataSubsystem::Get(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UPlayerDataSubsystem>();
}

// ========== 金币操作 ==========

void UPlayerDataSubsystem::AddGold(int32 Amount)
{
	int32 OldGold = CurrentGold;
	CurrentGold = FMath::Max(0, CurrentGold + Amount);
	
	if (OldGold != CurrentGold)
	{
		OnGoldChanged.Broadcast(CurrentGold, CurrentGold - OldGold);
		UE_LOG(LogTemp, Log, TEXT("[PlayerDataSubsystem] 金币变化: %d -> %d (变化: %+d)"), 
			OldGold, CurrentGold, CurrentGold - OldGold);
	}
}

void UPlayerDataSubsystem::SetGold(int32 Amount)
{
	int32 OldGold = CurrentGold;
	CurrentGold = FMath::Max(0, Amount);
	
	if (OldGold != CurrentGold)
	{
		OnGoldChanged.Broadcast(CurrentGold, CurrentGold - OldGold);
	}
}

bool UPlayerDataSubsystem::TryDeductGold(int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}
	
	if (CurrentGold < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerDataSubsystem] 扣除金币失败: 需要 %d, 当前 %d"), Amount, CurrentGold);
		return false;
	}
	
	AddGold(-Amount);
	return true;
}

// ========== 神格碎片操作 ==========

void UPlayerDataSubsystem::AddFragments(int32 Amount)
{
	int32 OldFragments = CurrentFragments;
	CurrentFragments = FMath::Max(0, CurrentFragments + Amount);
	
	if (OldFragments != CurrentFragments)
	{
		OnFragmentsChanged.Broadcast(CurrentFragments, CurrentFragments - OldFragments);
		UE_LOG(LogTemp, Log, TEXT("[PlayerDataSubsystem] 神格碎片变化: %d -> %d (变化: %+d)"), 
			OldFragments, CurrentFragments, CurrentFragments - OldFragments);
	}
}

void UPlayerDataSubsystem::SetFragments(int32 Amount)
{
	int32 OldFragments = CurrentFragments;
	CurrentFragments = FMath::Max(0, Amount);
	
	if (OldFragments != CurrentFragments)
	{
		OnFragmentsChanged.Broadcast(CurrentFragments, CurrentFragments - OldFragments);
	}
}

bool UPlayerDataSubsystem::TryDeductFragments(int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}
	
	if (CurrentFragments < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerDataSubsystem] 扣除神格碎片失败: 需要 %d, 当前 %d"), Amount, CurrentFragments);
		return false;
	}
	
	AddFragments(-Amount);
	return true;
}

// ========== 生命值操作 ==========

void UPlayerDataSubsystem::SetHealthPercent(float Percent)
{
	float OldPercent = CurrentHealthPercent;
	CurrentHealthPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
	
	if (!FMath::IsNearlyEqual(OldPercent, CurrentHealthPercent))
	{
		OnHealthChanged.Broadcast(CurrentHealthPercent);
	}
}

void UPlayerDataSubsystem::ModifyHealth(float DeltaPercent)
{
	SetHealthPercent(CurrentHealthPercent + DeltaPercent);
}

// ========== 法力值操作 ==========

void UPlayerDataSubsystem::SetManaPercent(float Percent)
{
	float OldPercent = CurrentManaPercent;
	CurrentManaPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
	
	if (!FMath::IsNearlyEqual(OldPercent, CurrentManaPercent))
	{
		OnManaChanged.Broadcast(CurrentManaPercent);
	}
}

// ========== 跨房间数据保持 ==========

void UPlayerDataSubsystem::SavePlayerData()
{
	SavedGold = CurrentGold;
	SavedFragments = CurrentFragments;
	SavedHealthPercent = CurrentHealthPercent;
	SavedManaPercent = CurrentManaPercent;
	
	UE_LOG(LogTemp, Log, TEXT("[PlayerDataSubsystem] 保存玩家数据: 金币=%d, 碎片=%d, 血量=%.0f%%"),
		SavedGold, SavedFragments, SavedHealthPercent * 100.0f);
}

void UPlayerDataSubsystem::RestorePlayerData()
{
	CurrentGold = SavedGold;
	CurrentFragments = SavedFragments;
	CurrentHealthPercent = SavedHealthPercent;
	CurrentManaPercent = SavedManaPercent;
	
	UE_LOG(LogTemp, Log, TEXT("[PlayerDataSubsystem] 恢复玩家数据: 金币=%d, 碎片=%d, 血量=%.0f%%"),
		CurrentGold, CurrentFragments, CurrentHealthPercent * 100.0f);
	
	// 广播所有变化事件
	OnGoldChanged.Broadcast(CurrentGold, 0);
	OnFragmentsChanged.Broadcast(CurrentFragments, 0);
	OnHealthChanged.Broadcast(CurrentHealthPercent);
	OnManaChanged.Broadcast(CurrentManaPercent);
}

void UPlayerDataSubsystem::ResetAllData()
{
	CurrentGold = 0;
	CurrentFragments = 0;
	CurrentHealthPercent = 1.0f;
	CurrentManaPercent = 1.0f;
	
	SavedGold = 0;
	SavedFragments = 0;
	SavedHealthPercent = 1.0f;
	SavedManaPercent = 1.0f;
	
	UE_LOG(LogTemp, Log, TEXT("[PlayerDataSubsystem] 重置所有数据"));
	
	OnGoldChanged.Broadcast(0, 0);
	OnFragmentsChanged.Broadcast(0, 0);
	OnHealthChanged.Broadcast(1.0f);
	OnManaChanged.Broadcast(1.0f);
}
