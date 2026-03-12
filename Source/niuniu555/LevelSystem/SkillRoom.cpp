#include "SkillRoom.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFlow/PlayerDataSubsystem.h"

ASkillRoom::ASkillRoom()
{
	RoomType = ERoomType::Skill;

	// 创建触发器
	SkillTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("SkillTrigger"));
	SkillTrigger->SetupAttachment(RootComponent);
	SkillTrigger->SetSphereRadius(200.0f);
	SkillTrigger->SetCollisionProfileName(TEXT("Trigger"));

	SkillRoomDisplayName = TEXT("技能神殿");
	bAllowSkillReset = true;
	ResetBasePrice = 100;
	ResetCount = 0;
}

void ASkillRoom::BeginPlay()
{
	Super::BeginPlay();

	// 绑定触发器事件
	if (SkillTrigger)
	{
		SkillTrigger->OnComponentBeginOverlap.AddDynamic(this, &ASkillRoom::OnPlayerEnterRange);
		SkillTrigger->OnComponentEndOverlap.AddDynamic(this, &ASkillRoom::OnPlayerExitRange);
	}
}

void ASkillRoom::OnEnterRoom_Implementation()
{
	Super::OnEnterRoom_Implementation();

	LogRoom(FString::Printf(TEXT("进入技能房: %s"), *SkillRoomDisplayName));
}

void ASkillRoom::OnCompleteRoom_Implementation()
{
	Super::OnCompleteRoom_Implementation();

	LogRoom(TEXT("技能房完成"));
}

bool ASkillRoom::UpgradeSkill(int32 SkillIndex)
{
	if (!AvailableSkills.IsValidIndex(SkillIndex))
	{
		return false;
	}

	// TODO: 与技能系统集成，检查并升级技能
	// 临时实现
	int32 UpgradeCost = 50; // 升级消耗
	if (GetPlayerFragments() < UpgradeCost)
	{
		LogRoom(TEXT("升级失败：神格碎片不足"));
		return false;
	}

	if (!SpendFragments(UpgradeCost))
	{
		return false;
	}

	LogRoom(FString::Printf(TEXT("技能升级成功: %d"), SkillIndex));
	OnSkillUpgraded.Broadcast(SkillIndex);

	return true;
}

bool ASkillRoom::ResetAllSkills()
{
	if (!bAllowSkillReset)
	{
		LogRoom(TEXT("重置失败：此房间不允许重置技能"));
		return false;
	}

	int32 ResetPrice = GetCurrentResetPrice();
	if (GetPlayerFragments() < ResetPrice)
	{
		LogRoom(TEXT("重置失败：神格碎片不足"));
		return false;
	}

	if (!SpendFragments(ResetPrice))
	{
		return false;
	}

	// TODO: 返还所有已花费的神格碎片
	int32 RefundAmount = 0; // 计算返还数量
	RefundFragments(RefundAmount);

	ResetCount++;

	LogRoom(FString::Printf(TEXT("技能重置成功，返还 %d 神格碎片"), RefundAmount));
	OnSkillReset.Broadcast(RefundAmount);

	return true;
}

bool ASkillRoom::LearnNewSkill(TSubclassOf<UObject> SkillClass)
{
	if (!SkillClass)
	{
		return false;
	}

	// TODO: 检查玩家是否已学习该技能
	// TODO: 扣除神格碎片学习新技能

	int32 LearnCost = 100; // 学习消耗
	if (GetPlayerFragments() < LearnCost)
	{
		LogRoom(TEXT("学习失败：神格碎片不足"));
		return false;
	}

	if (!SpendFragments(LearnCost))
	{
		return false;
	}

	LogRoom(FString::Printf(TEXT("学习新技能: %s"), *SkillClass->GetName()));
	return true;
}

int32 ASkillRoom::GetCurrentResetPrice() const
{
	// 每次重置价格递增
	return ResetBasePrice * (ResetCount + 1);
}

bool ASkillRoom::CanResetSkills() const
{
	if (!bAllowSkillReset)
	{
		return false;
	}

	return GetPlayerFragments() >= GetCurrentResetPrice();
}

int32 ASkillRoom::GetPlayerFragments() const
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		return DataSys->GetFragments();
	}
	return 0;
}

bool ASkillRoom::SpendFragments(int32 Amount)
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		return DataSys->TryDeductFragments(Amount);
	}
	return false;
}

void ASkillRoom::RefundFragments(int32 Amount)
{
	if (UPlayerDataSubsystem* DataSys = UPlayerDataSubsystem::Get(this))
	{
		DataSys->AddFragments(Amount);
	}
}

void ASkillRoom::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

	LogRoom(TEXT("玩家进入技能房范围"));
}

void ASkillRoom::OnPlayerExitRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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

	LogRoom(TEXT("玩家离开技能房范围"));
}
