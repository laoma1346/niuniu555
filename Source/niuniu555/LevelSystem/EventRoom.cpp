#include "EventRoom.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFlow/PlayerDataSubsystem.h"

AEventRoom::AEventRoom()
{
	RoomType = ERoomType::Event;
	bRandomEvent = true;
	FixedEventIndex = 0;
	CurrentEventIndex = -1;
	bEventActive = false;
	SelectedOptionIndex = -1;
}

void AEventRoom::BeginPlay()
{
	Super::BeginPlay();

	// 如果没有预设事件，创建一些默认事件
	if (AvailableEvents.Num() == 0)
	{
		CreateDefaultEvents();
	}
}

void AEventRoom::CreateDefaultEvents()
{
	// 创建祝福事件
	FEventConfig BlessingEvent;
	BlessingEvent.EventType = EEventType::Blessing;
	BlessingEvent.EventTitle = TEXT("神谕祝福");
	BlessingEvent.EventDescription = TEXT("财神显灵，赐予你祝福。选择一项祝福获得增益效果。");
	BlessingEvent.bCanSkip = false;

	FEventOption Option1;
	Option1.OptionName = TEXT("力量祝福");
	Option1.OptionDescription = TEXT("攻击力+10%，持续整局");
	Option1.bCostsGold = false;
	Option1.bCostsFragments = false;
	BlessingEvent.Options.Add(Option1);

	FEventOption Option2;
	Option2.OptionName = TEXT("生命祝福");
	Option2.OptionDescription = TEXT("最大生命值+20%，持续整局");
	Option2.bCostsGold = false;
	Option2.bCostsFragments = false;
	BlessingEvent.Options.Add(Option2);

	FEventOption Option3;
	Option3.OptionName = TEXT("财富祝福");
	Option3.OptionDescription = TEXT("金币获取率+25%，持续整局");
	Option3.bCostsGold = false;
	Option3.bCostsFragments = false;
	BlessingEvent.Options.Add(Option3);

	AvailableEvents.Add(BlessingEvent);

	// 创建诅咒事件
	FEventConfig CurseEvent;
	CurseEvent.EventType = EEventType::Curse;
	CurseEvent.EventTitle = TEXT("邪神诅咒");
	CurseEvent.EventDescription = TEXT("你触碰了禁忌之物，必须承受代价。");
	CurseEvent.bCanSkip = false;

	FEventOption CurseOption1;
	CurseOption1.OptionName = TEXT("生命献祭");
	CurseOption1.OptionDescription = TEXT("失去20%最大生命值");
	CurseOption1.bCostsGold = false;
	CurseEvent.Options.Add(CurseOption1);

	FEventOption CurseOption2;
	CurseOption2.OptionName = TEXT("财富献祭");
	CurseOption2.OptionDescription = TEXT("失去50%当前金币");
	CurseOption2.bCostsGold = false;
	CurseEvent.Options.Add(CurseOption2);

	AvailableEvents.Add(CurseEvent);

	// 创建宝藏事件
	FEventConfig TreasureEvent;
	TreasureEvent.EventType = EEventType::Treasure;
	TreasureEvent.EventTitle = TEXT("神秘宝箱");
	TreasureEvent.EventDescription = TEXT("你发现了一个古老的宝箱。");
	TreasureEvent.bCanSkip = true;

	FEventOption TreasureOption1;
	TreasureOption1.OptionName = TEXT("开启宝箱");
	TreasureOption1.OptionDescription = TEXT("获得随机奖励");
	TreasureOption1.bCostsGold = false;
	TreasureEvent.Options.Add(TreasureOption1);

	FEventOption TreasureOption2;
	TreasureOption2.OptionName = TEXT("离开");
	TreasureOption2.OptionDescription = TEXT("不冒险，不获得");
	TreasureOption2.bCostsGold = false;
	TreasureEvent.Options.Add(TreasureOption2);

	AvailableEvents.Add(TreasureEvent);

	// 创建商店事件
	FEventConfig ShopEvent;
	ShopEvent.EventType = EEventType::Shop;
	ShopEvent.EventTitle = TEXT("神秘商人");
	ShopEvent.EventDescription = TEXT("一位神秘商人拦住了你的去路。他有珍贵的物品出售。");
	ShopEvent.bCanSkip = true;

	FEventOption ShopOption;
	ShopOption.OptionName = TEXT("查看商品");
	ShopOption.OptionDescription = TEXT("浏览商人的货物");
	ShopOption.bCostsGold = false;
	ShopEvent.Options.Add(ShopOption);

	AvailableEvents.Add(ShopEvent);

	// 创建赌博事件
	FEventConfig GambleEvent;
	GambleEvent.EventType = EEventType::Gamble;
	GambleEvent.EventTitle = TEXT("命运轮盘");
	GambleEvent.EventDescription = TEXT("你愿意赌上一切换取更大的回报吗？");
	GambleEvent.bCanSkip = true;

	FEventOption GambleOption1;
	GambleOption1.OptionName = TEXT("押注金币");
	GambleOption1.OptionDescription = TEXT("50%几率翻倍，50%几率失去");
	GambleOption1.bCostsGold = true;
	GambleOption1.GoldCost = 100;
	GambleEvent.Options.Add(GambleOption1);

	FEventOption GambleOption2;
	GambleOption2.OptionName = TEXT("离开");
	GambleOption2.OptionDescription = TEXT("不赌为赢");
	GambleOption2.bCostsGold = false;
	GambleEvent.Options.Add(GambleOption2);

	AvailableEvents.Add(GambleEvent);
}

void AEventRoom::OnEnterRoom_Implementation()
{
	Super::OnEnterRoom_Implementation();

	LogRoom(TEXT("进入事件房间"));

	// 选择事件
	SelectEvent();
}

void AEventRoom::OnCompleteRoom_Implementation()
{
	Super::OnCompleteRoom_Implementation();

	bEventActive = false;

	LogRoom(TEXT("事件房间完成"));
}

void AEventRoom::SelectEvent()
{
	if (AvailableEvents.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EventRoom] 没有可用事件"));
		CompleteRoom();
		return;
	}

	if (bRandomEvent)
	{
		CurrentEventIndex = GetRandomEventIndex();
	}
	else
	{
		CurrentEventIndex = FMath::Clamp(FixedEventIndex, 0, AvailableEvents.Num() - 1);
	}

	CurrentEvent = AvailableEvents[CurrentEventIndex];
	bEventActive = true;
	SelectedOptionIndex = -1;

	LogRoom(FString::Printf(TEXT("激活事件: %s"), *CurrentEvent.EventTitle));

	OnEventActivated.Broadcast(CurrentEvent);
}

void AEventRoom::ChooseOption(int32 OptionIndex)
{
	if (!bEventActive)
	{
		return;
	}

	if (!CurrentEvent.Options.IsValidIndex(OptionIndex))
	{
		return;
	}

	if (!IsOptionAvailable(OptionIndex))
	{
		LogRoom(TEXT("选择失败：资源不足"));
		return;
	}

	const FEventOption& SelectedOption = CurrentEvent.Options[OptionIndex];
	SelectedOptionIndex = OptionIndex;

	LogRoom(FString::Printf(TEXT("选择选项 %d: %s"), OptionIndex, *SelectedOption.OptionName));

	// 扣除资源
	if (SelectedOption.bCostsGold && SelectedOption.GoldCost > 0)
	{
		DeductPlayerGold(SelectedOption.GoldCost);
	}
	if (SelectedOption.bCostsFragments && SelectedOption.FragmentCost > 0)
	{
		DeductPlayerFragments(SelectedOption.FragmentCost);
	}

	OnOptionSelected.Broadcast(OptionIndex);

	// 执行选项效果
	ExecuteOptionEffect(OptionIndex);

	// 完成房间
	OnEventCompleted.Broadcast(true);
	CompleteRoom();
}

void AEventRoom::SkipEvent()
{
	if (!CanSkip())
	{
		return;
	}

	LogRoom(TEXT("跳过事件"));

	bEventActive = false;
	OnEventCompleted.Broadcast(false);
	CompleteRoom();
}

bool AEventRoom::CanSkip() const
{
	return bEventActive && CurrentEvent.bCanSkip;
}

bool AEventRoom::IsOptionAvailable(int32 OptionIndex) const
{
	if (!CurrentEvent.Options.IsValidIndex(OptionIndex))
	{
		return false;
	}

	const FEventOption& Option = CurrentEvent.Options[OptionIndex];

	// 检查玩家资源是否足够
	if (Option.bCostsGold && Option.GoldCost > 0)
	{
		if (!HasEnoughGold(Option.GoldCost))
		{
			return false;
		}
	}
	if (Option.bCostsFragments && Option.FragmentCost > 0)
	{
		if (!HasEnoughFragments(Option.FragmentCost))
		{
			return false;
		}
	}

	return true;
}

void AEventRoom::LogCurrentEventInfo() const
{
	if (!bEventActive)
	{
		UE_LOG(LogTemp, Log, TEXT("[EventRoom] 当前没有激活的事件"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("========== 当前事件信息 =========="));
	UE_LOG(LogTemp, Log, TEXT("事件名称: %s"), *CurrentEvent.EventTitle);
	UE_LOG(LogTemp, Log, TEXT("事件描述: %s"), *CurrentEvent.EventDescription);
	UE_LOG(LogTemp, Log, TEXT("事件类型: %d"), (int32)CurrentEvent.EventType);
	UE_LOG(LogTemp, Log, TEXT("可跳过: %s"), CurrentEvent.bCanSkip ? TEXT("是") : TEXT("否"));
	UE_LOG(LogTemp, Log, TEXT("选项数量: %d"), CurrentEvent.Options.Num());
	
	for (int32 i = 0; i < CurrentEvent.Options.Num(); ++i)
	{
		const FEventOption& Option = CurrentEvent.Options[i];
		FString CostStr = TEXT("");
		if (Option.bCostsGold && Option.GoldCost > 0)
		{
			CostStr += FString::Printf(TEXT(" [金币:%d]"), Option.GoldCost);
		}
		if (Option.bCostsFragments && Option.FragmentCost > 0)
		{
			CostStr += FString::Printf(TEXT(" [碎片:%d]"), Option.FragmentCost);
		}
		if (CostStr.IsEmpty())
		{
			CostStr = TEXT(" [免费]");
		}
		
		bool bAvailable = IsOptionAvailable(i);
		UE_LOG(LogTemp, Log, TEXT("  [%d] %s - %s%s %s"), 
			i, 
			*Option.OptionName, 
			*Option.OptionDescription,
			*CostStr,
			bAvailable ? TEXT("(可购买)") : TEXT("(资源不足)"));
	}
	
	UE_LOG(LogTemp, Log, TEXT("=================================="));
}

void AEventRoom::ApplyReward(const FEventOption& Option)
{
	LogRoom(FString::Printf(TEXT("应用奖励: %s"), *Option.OptionName));

	// 根据选项描述发放奖励
	if (Option.OptionDescription.Contains(TEXT("金币")))
	{
		// 解析金币数量
		int32 GoldAmount = 50; // 默认奖励
		GivePlayerGold(GoldAmount);
	}
	else if (Option.OptionDescription.Contains(TEXT("攻击力")))
	{
		// 授予攻击力Buff
		GrantTemporaryBuff(FName("AttackBoost"), -1.0f); // -1表示永久
	}
	else if (Option.OptionDescription.Contains(TEXT("生命值")))
	{
		// 授予生命值Buff
		GrantTemporaryBuff(FName("HealthBoost"), -1.0f);
	}
	else if (Option.OptionDescription.Contains(TEXT("金币获取率")))
	{
		// 授予金币获取率Buff
		GrantTemporaryBuff(FName("GoldFindBoost"), -1.0f);
	}
	else
	{
		// 默认奖励：金币
		GivePlayerGold(50);
	}
}

void AEventRoom::ApplyPenalty(const FEventOption& Option)
{
	LogRoom(FString::Printf(TEXT("应用惩罚: %s"), *Option.OptionName));

	// 根据选项描述施加惩罚
	if (Option.OptionDescription.Contains(TEXT("生命值")))
	{
		// 减少生命值
		ApplyHealthPenalty(0.2f); // 减少20%
	}
	else if (Option.OptionDescription.Contains(TEXT("金币")))
	{
		// 减少金币
		int32 CurrentGold = GetPlayerGold();
		int32 GoldToLose = FMath::RoundToInt(CurrentGold * 0.5f);
		DeductPlayerGold(GoldToLose);
	}
}

void AEventRoom::GrantTemporaryBuff(const FName& BuffName, float Duration)
{
	LogRoom(FString::Printf(TEXT("授予Buff: %s (持续时间: %.1f秒)"), *BuffName.ToString(), Duration));

	// TODO: 与Buff系统集成
	// 临时实现：输出日志
	UE_LOG(LogTemp, Log, TEXT("[EventRoom] Buff %s 已应用"), *BuffName.ToString());

	// Buff类型处理
	if (BuffName == FName("AttackBoost"))
	{
		UE_LOG(LogTemp, Log, TEXT("[EventRoom] 攻击力+10%%"));
	}
	else if (BuffName == FName("HealthBoost"))
	{
		UE_LOG(LogTemp, Log, TEXT("[EventRoom] 最大生命值+20%%"));
	}
	else if (BuffName == FName("GoldFindBoost"))
	{
		UE_LOG(LogTemp, Log, TEXT("[EventRoom] 金币获取率+25%%"));
	}
}

void AEventRoom::ApplyHealthPenalty(float Percent)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->GetPawn())
	{
		// TODO: 与AttributeSystem集成，减少最大生命值
		UE_LOG(LogTemp, Log, TEXT("[EventRoom] 最大生命值减少 %.0f%%"), Percent * 100.0f);
	}
}

int32 AEventRoom::GetPlayerGold() const
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		return DataSys->GetGold();
	}
	return 0;
}

int32 AEventRoom::GetPlayerFragments() const
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		return DataSys->GetFragments();
	}
	return 0;
}

bool AEventRoom::HasEnoughGold(int32 Amount) const
{
	return GetPlayerGold() >= Amount;
}

bool AEventRoom::HasEnoughFragments(int32 Amount) const
{
	return GetPlayerFragments() >= Amount;
}

bool AEventRoom::DeductPlayerGold(int32 Amount)
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		return DataSys->TryDeductGold(Amount);
	}
	return false;
}

bool AEventRoom::DeductPlayerFragments(int32 Amount)
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		return DataSys->TryDeductFragments(Amount);
	}
	return false;
}

void AEventRoom::GivePlayerGold(int32 Amount)
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		DataSys->AddGold(Amount);
	}
}

int32 AEventRoom::GetRandomEventIndex() const
{
	if (AvailableEvents.Num() == 0)
	{
		return -1;
	}

	return FMath::RandRange(0, AvailableEvents.Num() - 1);
}

void AEventRoom::ExecuteOptionEffect(int32 OptionIndex)
{
	if (!CurrentEvent.Options.IsValidIndex(OptionIndex))
	{
		return;
	}

	const FEventOption& Option = CurrentEvent.Options[OptionIndex];

	switch (CurrentEvent.EventType)
	{
	case EEventType::Blessing:
		HandleBlessingEvent(Option);
		ApplyReward(Option);
		break;

	case EEventType::Curse:
		HandleCurseEvent(Option);
		ApplyPenalty(Option);
		break;

	case EEventType::Treasure:
		if (OptionIndex == 0) // 开启宝箱
		{
			HandleTreasureEvent(Option);
			// 随机奖励
			int32 RandomReward = FMath::RandRange(50, 200);
			GivePlayerGold(RandomReward);
		}
		break;

	case EEventType::Shop:
		HandleShopEvent(Option);
		break;

	case EEventType::Gamble:
		HandleGambleEvent(Option);
		// 赌博逻辑
		if (OptionIndex == 0) // 押注
		{
			bool bWin = FMath::RandBool();
			if (bWin)
			{
				GivePlayerGold(200); // 赢得200金币
				UE_LOG(LogTemp, Log, TEXT("[EventRoom] 赌博获胜！"));
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("[EventRoom] 赌博失败..."));
			}
		}
		break;

	default:
		break;
	}
}
