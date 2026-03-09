#pragma once

#include "CoreMinimal.h"
#include "EquipmentTypes.generated.h"

/**
 * 装备品质枚举
 */
UENUM(BlueprintType)
enum class EEquipmentQuality : uint8
{
	Common = 0 UMETA(DisplayName = "普通"),     // 白色
	Uncommon = 1 UMETA(DisplayName = "优秀"),   // 蓝色
	Rare = 2 UMETA(DisplayName = "稀有"),       // 金色
	Epic = 3 UMETA(DisplayName = "史诗"),       // 暗金
	Legendary = 4 UMETA(DisplayName = "传说"),  // 神话
	MAX UMETA(Hidden)
};

/**
 * 装备槽位枚举
 */
UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	MainHand = 0 UMETA(DisplayName = "主手"),
	OffHand = 1 UMETA(DisplayName = "副手"),
	Helmet = 2 UMETA(DisplayName = "头盔"),
	Chest = 3 UMETA(DisplayName = "胸甲"),
	Legs = 4 UMETA(DisplayName = "护腿"),
	Accessory1 = 5 UMETA(DisplayName = "饰品1"),
	Accessory2 = 6 UMETA(DisplayName = "饰品2"),
	MAX UMETA(Hidden)
};

/**
 * 装备类型枚举
 */
UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	Weapon = 0 UMETA(DisplayName = "武器"),
	Armor = 1 UMETA(DisplayName = "防具"),
	Accessory = 2 UMETA(DisplayName = "饰品"),
	MAX UMETA(Hidden)
};

/**
 * 装备基础属性结构体
 */
USTRUCT(BlueprintType)
struct FEquipmentBaseStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础属性")
	float Attack = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础属性")
	float Defense = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础属性")
	float MaxHealth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础属性")
	float CriticalChance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础属性")
	float CriticalDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础属性")
	float MovementSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础属性")
	float GoldFind = 0.0f;
};

/**
 * 装备ID结构体
 */
USTRUCT(BlueprintType)
struct FEquipmentID
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备ID")
	FString UniqueID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备ID")
	FString BaseID;

	void GenerateUniqueID();
};

/**
 * 装备词条结构体
 */
USTRUCT(BlueprintType)
struct FEquipmentAffix
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "词条")
	FString AffixID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "词条")
	FString AffixName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "词条")
	FString AffixDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "词条")
	TMap<FString, float> AttributeModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "词条")
	float Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "词条")
	bool bIsPrefix = false;
};

/**
 * 装备套装信息结构体
 */
USTRUCT(BlueprintType)
struct FEquipmentSetInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "套装")
	FString SetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "套装")
	FString SetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "套装")
	int32 RequiredPieces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "套装")
	TMap<FString, float> SetBonuses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "套装")
	FString SetBonusDescription;
};

/**
 * 装备实例数据结构体
 */
USTRUCT(BlueprintType)
struct FEquipmentInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	FEquipmentID EquipmentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	EEquipmentQuality Quality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	EEquipmentSlot Slot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	EEquipmentType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	FString EquipmentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	FString EquipmentDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	FEquipmentBaseStats BaseStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	TArray<FEquipmentAffix> Affixes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	FEquipmentSetInfo SetInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	int32 Durability = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	int32 MaxDurability = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	int32 EnhancementLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备实例")
	bool bIsEquipped = false;
};
