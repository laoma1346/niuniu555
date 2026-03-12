#include "ShopRoom.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFlow/PlayerDataSubsystem.h"

AShopRoom::AShopRoom()
{
	RoomType = ERoomType::Shop;

	// 创建触发器
	ShopTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("ShopTrigger"));
	ShopTrigger->SetupAttachment(RootComponent);
	ShopTrigger->SetSphereRadius(200.0f);
	ShopTrigger->SetCollisionProfileName(TEXT("Trigger"));

	ShopName = TEXT("神秘商店");
	bGenerateRandomItems = true;
	bAllowRefresh = true;
	RefreshPrice = 50;
	MaxRefreshCount = 3;
	DiscountRate = 1.0f;
	RefreshCount = 0;
	bShopOpen = false;
}

void AShopRoom::BeginPlay()
{
	Super::BeginPlay();

	// 绑定触发器事件
	if (ShopTrigger)
	{
		ShopTrigger->OnComponentBeginOverlap.AddDynamic(this, &AShopRoom::OnPlayerEnterShopRange);
		ShopTrigger->OnComponentEndOverlap.AddDynamic(this, &AShopRoom::OnPlayerExitShopRange);
	}

	// 生成商品
	GenerateItems();
}

void AShopRoom::OnEnterRoom_Implementation()
{
	Super::OnEnterRoom_Implementation();

	LogRoom(FString::Printf(TEXT("进入商店房间: %s"), *ShopName));

	// 自动打开商店（如果玩家已经在范围内）
	if (!bShopOpen && IsValid(ShopTrigger))
	{
		// 检查是否有玩家在范围内
		TArray<AActor*> OverlappingActors;
		ShopTrigger->GetOverlappingActors(OverlappingActors, APawn::StaticClass());

		for (AActor* Actor : OverlappingActors)
		{
			if (Actor && Actor == UGameplayStatics::GetPlayerPawn(this, 0))
			{
				OpenShop();
				break;
			}
		}
	}
}

void AShopRoom::OnCompleteRoom_Implementation()
{
	Super::OnCompleteRoom_Implementation();

	// 关闭商店
	CloseShop();

	LogRoom(TEXT("商店房间完成"));
}

void AShopRoom::GenerateItems()
{
	CurrentItems.Empty();
	RefreshCount = 0;

	if (bGenerateRandomItems)
	{
		GenerateItemListInternal();
	}
	else
	{
		CurrentItems = PresetItems;
	}

	LogRoom(FString::Printf(TEXT("生成 %d 件商品"), CurrentItems.Num()));
}

void AShopRoom::GenerateItemListInternal()
{
	int32 ItemCount = FMath::RandRange(ItemCountRange.X, ItemCountRange.Y);

	for (int32 i = 0; i < ItemCount; ++i)
	{
		if (ItemPool.Num() > 0)
		{
			FShopItem Item = GenerateRandomItem();
			CurrentItems.Add(Item);
		}
	}
}

bool AShopRoom::RefreshItems()
{
	if (!CanRefresh())
	{
		LogRoom(TEXT("刷新失败：已达最大刷新次数"));
		return false;
	}

	if (!CheckPlayerGold(GetNextRefreshPrice()))
	{
		LogRoom(TEXT("刷新失败：金币不足"));
		return false;
	}

	// 扣除刷新费用
	if (!DeductPlayerGold(GetNextRefreshPrice()))
	{
		return false;
	}

	// 重新生成商品
	CurrentItems.Empty();
	GenerateItemListInternal();

	RefreshCount++;

	LogRoom(FString::Printf(TEXT("刷新商品 (%d/%d)"), RefreshCount, MaxRefreshCount));

	OnShopRefreshed.Broadcast(CurrentItems);

	return true;
}

bool AShopRoom::PurchaseItem(int32 ItemIndex)
{
	if (!CurrentItems.IsValidIndex(ItemIndex))
	{
		return false;
	}

	FShopItem& Item = CurrentItems[ItemIndex];

	if (Item.bSold)
	{
		LogRoom(TEXT("购买失败：商品已售出"));
		return false;
	}

	int32 FinalPrice = GetDiscountedPrice(Item);

	if (!CheckPlayerGold(FinalPrice))
	{
		LogRoom(TEXT("购买失败：金币不足"));
		return false;
	}

	// 扣除金币
	if (!DeductPlayerGold(FinalPrice))
	{
		return false;
	}

	// 标记为已售出
	Item.bSold = true;

	// TODO: 给予玩家物品
	LogRoom(FString::Printf(TEXT("购买商品: %s (价格: %d)"), *Item.ItemName, FinalPrice));

	OnItemPurchased.Broadcast(ItemIndex, Item);

	return true;
}

int32 AShopRoom::SellItem(AActor* Item)
{
	if (!Item)
	{
		return 0;
	}

	// TODO: 计算物品价值
	int32 SellPrice = 10;

	// 给予金币
	GivePlayerGold(SellPrice);

	// 销毁物品
	Item->Destroy();

	LogRoom(FString::Printf(TEXT("出售物品，获得 %d 金币"), SellPrice));

	return SellPrice;
}

int32 AShopRoom::GetDiscountedPrice(const FShopItem& Item) const
{
	return FMath::RoundToInt(Item.GoldPrice * DiscountRate);
}

bool AShopRoom::CanRefresh() const
{
	if (!bAllowRefresh)
	{
		return false;
	}

	if (MaxRefreshCount > 0 && RefreshCount >= MaxRefreshCount)
	{
		return false;
	}

	return true;
}

int32 AShopRoom::GetNextRefreshPrice() const
{
	// 每次刷新价格递增
	return RefreshPrice * (RefreshCount + 1);
}

void AShopRoom::LogShopInfo() const
{
	UE_LOG(LogTemp, Log, TEXT("========== 商店信息 =========="));
	UE_LOG(LogTemp, Log, TEXT("商店名称: %s"), *ShopName);
	UE_LOG(LogTemp, Log, TEXT("商店打开: %s"), bShopOpen ? TEXT("是") : TEXT("否"));
	UE_LOG(LogTemp, Log, TEXT("商品数量: %d"), CurrentItems.Num());
	UE_LOG(LogTemp, Log, TEXT("折扣率: %.0f%%"), DiscountRate * 100.0f);
	
	if (bAllowRefresh)
	{
		UE_LOG(LogTemp, Log, TEXT("刷新次数: %d/%d"), RefreshCount, MaxRefreshCount > 0 ? MaxRefreshCount : 999);
		UE_LOG(LogTemp, Log, TEXT("下次刷新价格: %d"), GetNextRefreshPrice());
	}
	
	UE_LOG(LogTemp, Log, TEXT("---------- 商品列表 ----------"));
	for (int32 i = 0; i < CurrentItems.Num(); ++i)
	{
		const FShopItem& Item = CurrentItems[i];
		int32 FinalPrice = GetDiscountedPrice(Item);
		
		if (Item.bSold)
		{
			UE_LOG(LogTemp, Log, TEXT("  [%d] [已售出] %s"), i, *Item.ItemName);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("  [%d] %s - 金币:%d 碎片:%d"), 
				i, *Item.ItemName, Item.GoldPrice, Item.FragmentPrice);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("=============================="));
}

FShopItem AShopRoom::GenerateRandomItem()
{
	FShopItem Item;

	if (ItemPool.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, ItemPool.Num() - 1);
		Item.ItemClass = ItemPool[RandomIndex];
		Item.ItemName = FString::Printf(TEXT("随机物品 %d"), RandomIndex);
		Item.GoldPrice = CalculateItemPrice(Item.ItemClass);
		Item.bSold = false;
	}

	return Item;
}

int32 AShopRoom::CalculateItemPrice(TSubclassOf<AActor> ItemClass) const
{
	// TODO: 根据物品等级、品质计算价格
	// 临时实现：随机价格
	return FMath::RandRange(50, 200);
}

void AShopRoom::OpenShop()
{
	if (bShopOpen)
	{
		return;
	}

	bShopOpen = true;

	LogRoom(TEXT("打开商店"));

	OnShopOpened.Broadcast();
}

void AShopRoom::CloseShop()
{
	if (!bShopOpen)
	{
		return;
	}

	bShopOpen = false;

	LogRoom(TEXT("关闭商店"));

	OnShopClosed.Broadcast();
}

void AShopRoom::FinishShopping()
{
	LogRoom(TEXT("完成购物，离开商店"));
	
	// 关闭商店界面
	CloseShop();
	
	// 完成房间
	CompleteRoom();
}

void AShopRoom::OnPlayerEnterShopRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (!PlayerPawn)
	{
		return;
	}

	// 检查是否是本地玩家
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC || PlayerPawn != PC->GetPawn())
	{
		return;
	}

	LogRoom(TEXT("玩家进入商店范围"));

	// 自动打开商店
	OpenShop();
}

void AShopRoom::OnPlayerExitShopRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (!PlayerPawn)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC || PlayerPawn != PC->GetPawn())
	{
		return;
	}

	LogRoom(TEXT("玩家离开商店范围"));

	// 可选：自动关闭商店
	// CloseShop();
}

bool AShopRoom::CheckPlayerGold(int32 Amount) const
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		return DataSys->HasEnoughGold(Amount);
	}
	return false;
}

bool AShopRoom::DeductPlayerGold(int32 Amount)
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		return DataSys->TryDeductGold(Amount);
	}
	return false;
}

void AShopRoom::GivePlayerGold(int32 Amount)
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		DataSys->AddGold(Amount);
	}
}

APawn* AShopRoom::GetPlayerCharacter() const
{
	return UGameplayStatics::GetPlayerPawn(this, 0);
}
