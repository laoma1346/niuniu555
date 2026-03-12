#pragma once

#include "CoreMinimal.h"
#include "LevelTypes.h"
#include "RoomBase.h"
#include "ShopRoom.generated.h"

// 商店房间专用事件委托（LevelTypes.h中没有，在此定义）
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopOpenedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopClosedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShopItemPurchasedDelegate, int32, ItemIndex, const FShopItem&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopRefreshedDelegate, const TArray<FShopItem>&, NewItems);

/**
 * 商店房间
 * 购买装备、消耗品、服务等
 */
UCLASS(Blueprintable)
class NIUNIU555_API AShopRoom : public ARoomBase
{
	GENERATED_BODY()

public:
	AShopRoom();

protected:
	virtual void BeginPlay() override;
	virtual void OnEnterRoom_Implementation() override;
	virtual void OnCompleteRoom_Implementation() override;

public:
	// ========== 商店配置 ==========

	/** 商店名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "商店配置")
	FString ShopName = TEXT("神秘商店");

	/** 预设商品列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "商店配置")
	TArray<FShopItem> PresetItems;

	/** 商品池（随机生成用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "商店配置")
	TArray<TSubclassOf<class AActor>> ItemPool;

	/** 商品数量范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "商店配置")
	FIntPoint ItemCountRange = FIntPoint(3, 6);

	/** 是否随机生成商品 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "商店配置")
	bool bGenerateRandomItems = true;

	/** 是否允许刷新商品 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "商店配置")
	bool bAllowRefresh = true;

	/** 刷新价格 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "商店配置", meta = (EditCondition = "bAllowRefresh"))
	int32 RefreshPrice = 50;

	/** 刷新次数限制（0表示无限） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "商店配置", meta = (EditCondition = "bAllowRefresh"))
	int32 MaxRefreshCount = 3;

	/** 折扣率（1.0=原价，0.8=8折） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "商店配置", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float DiscountRate = 1.0f;

	// ========== 运行时状态 ==========

	/** 当前商品列表 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "商店状态")
	TArray<FShopItem> CurrentItems;

	/** 已刷新次数 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "商店状态")
	int32 RefreshCount = 0;

	/** 商店是否打开 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "商店状态")
	bool bShopOpen = false;

	// ========== 商品管理 ==========

	/**
	 * 生成商品列表
	 */
	UFUNCTION(BlueprintCallable, Category = "商品管理")
	void GenerateItems();

	/**
	 * 刷新商品
	 * @return 是否刷新成功
	 */
	UFUNCTION(BlueprintCallable, Category = "商品管理")
	bool RefreshItems();

	/**
	 * 购买商品
	 * @param ItemIndex 商品索引
	 * @return 是否购买成功
	 */
	UFUNCTION(BlueprintCallable, Category = "商品管理")
	bool PurchaseItem(int32 ItemIndex);

	/**
	 * 出售物品
	 * @param Item 要出售的物品
	 * @return 出售获得的金币
	 */
	UFUNCTION(BlueprintCallable, Category = "商品管理")
	int32 SellItem(class AActor* Item);

	/**
	 * 获取商品实际价格（考虑折扣）
	 */
	UFUNCTION(BlueprintPure, Category = "商品管理")
	int32 GetDiscountedPrice(const FShopItem& Item) const;

	/**
	 * 检查是否还能刷新
	 */
	UFUNCTION(BlueprintPure, Category = "商品管理")
	bool CanRefresh() const;

	/**
	 * 获取下次刷新价格
	 */
	UFUNCTION(BlueprintPure, Category = "商品管理")
	int32 GetNextRefreshPrice() const;

	/**
	 * 输出商店信息到日志
	 */
	UFUNCTION(BlueprintCallable, Category = "商品管理")
	void LogShopInfo() const;

	// ========== 商品生成 ==========

	/**
	 * 从商品池生成随机商品
	 */
	UFUNCTION(BlueprintCallable, Category = "商品生成")
	FShopItem GenerateRandomItem();

	/**	 * 计算商品推荐价格
	 */
	UFUNCTION(BlueprintPure, Category = "商品生成")
	int32 CalculateItemPrice(TSubclassOf<class AActor> ItemClass) const;

	// ========== 交互 ==========

	/**
	 * 打开商店界面
	 */
	UFUNCTION(BlueprintCallable, Category = "交互")
	void OpenShop();

	/**
	 * 关闭商店界面
	 */
	UFUNCTION(BlueprintCallable, Category = "交互")
	void CloseShop();

	/**
	 * 完成购物并离开商店（完成房间）
	 */
	UFUNCTION(BlueprintCallable, Category = "交互")
	void FinishShopping();

	/**
	 * 玩家进入商店范围
	 */
	UFUNCTION()
	void OnPlayerEnterShopRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * 玩家离开商店范围
	 */
	UFUNCTION()
	void OnPlayerExitShopRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// ========== 事件委托 ==========

	/** 商店打开事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnShopOpenedDelegate OnShopOpened;

	/** 商店关闭事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnShopClosedDelegate OnShopClosed;

	/** 商品购买事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnShopItemPurchasedDelegate OnItemPurchased;

	/** 商品刷新事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnShopRefreshedDelegate OnShopRefreshed;

protected:
	/** 触发器组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "组件")
	class USphereComponent* ShopTrigger;

	/** 生成商品列表（内部） */
	void GenerateItemListInternal();

	/** 检查玩家金币是否足够 */
	bool CheckPlayerGold(int32 Amount) const;

	/** 扣除玩家金币 */
	bool DeductPlayerGold(int32 Amount);

	/** 给予玩家金币 */
	void GivePlayerGold(int32 Amount);

	/** 获取玩家角色 */
	class APawn* GetPlayerCharacter() const;
};
