#pragma once

#include "CoreMinimal.h"
#include "LevelTypes.h"
#include "RoomBase.h"
#include "SkillRoom.generated.h"

// 技能房事件委托（LevelTypes.h中没有，在此定义）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillUpgraded, int32, SkillIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillReset, int32, SpentFragments);

/**
 * 技能房
 * 升级技能、重置技能、学习新技能
 */
UCLASS(Blueprintable)
class NIUNIU555_API ASkillRoom : public ARoomBase
{
	GENERATED_BODY()

public:
	ASkillRoom();

protected:
	virtual void BeginPlay() override;
	virtual void OnEnterRoom_Implementation() override;
	virtual void OnCompleteRoom_Implementation() override;

public:
	// ========== 房间配置 ==========

	/** 技能房显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "技能房配置")
	FString SkillRoomDisplayName = TEXT("技能神殿");

	/** 是否允许重置技能 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "技能房配置")
	bool bAllowSkillReset = true;

	/** 重置基础价格 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "技能房配置", meta = (EditCondition = "bAllowSkillReset"))
	int32 ResetBasePrice = 100;

	/** 可学习的技能列表（预留，与Phase 7技能树系统联动） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "技能房配置")
	TArray<TSubclassOf<UObject>> AvailableSkills;

	// ========== 运行时状态 ==========

	/** 当前重置次数（价格递增） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "技能房状态")
	int32 ResetCount = 0;

	// ========== 技能操作 ==========

	/**
	 * 升级技能
	 * @param SkillIndex 技能索引
	 * @return 是否升级成功
	 */
	UFUNCTION(BlueprintCallable, Category = "技能操作")
	bool UpgradeSkill(int32 SkillIndex);

	/**
	 * 重置所有技能
	 * @return 是否重置成功
	 */
	UFUNCTION(BlueprintCallable, Category = "技能操作")
	bool ResetAllSkills();

	/**
	 * 学习新技能
	 * @param SkillClass 技能类
	 * @return 是否学习成功
	 */
	UFUNCTION(BlueprintCallable, Category = "技能操作")
	bool LearnNewSkill(TSubclassOf<UObject> SkillClass);

	/**
	 * 获取当前重置价格
	 */
	UFUNCTION(BlueprintPure, Category = "技能操作")
	int32 GetCurrentResetPrice() const;

	/**
	 * 检查是否可以重置
	 */
	UFUNCTION(BlueprintPure, Category = "技能操作")
	bool CanResetSkills() const;

	/**
	 * 获取玩家可用神格碎片
	 */
	UFUNCTION(BlueprintPure, Category = "技能操作")
	int32 GetPlayerFragments() const;

	/**
	 * 扣除神格碎片
	 */
	UFUNCTION(BlueprintCallable, Category = "技能操作")
	bool SpendFragments(int32 Amount);

	/**
	 * 返还神格碎片（重置时）
	 */
	UFUNCTION(BlueprintCallable, Category = "技能操作")
	void RefundFragments(int32 Amount);

	// ========== 事件委托 ==========

	/** 技能升级事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnSkillUpgraded OnSkillUpgraded;

	/** 技能重置事件 */
	UPROPERTY(BlueprintAssignable, Category = "事件")
	FOnSkillReset OnSkillReset;

protected:
	/** 触发器组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "组件")
	class USphereComponent* SkillTrigger;

	/** 玩家进入范围 */
	UFUNCTION()
	void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 玩家离开范围 */
	UFUNCTION()
	void OnPlayerExitRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
