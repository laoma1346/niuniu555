#include "EquipmentBase.h"
#include "EquipmentDataAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "AttributeSystem/AttributeComponent.h"
#include "Kismet/GameplayStatics.h"

// 装备系统日志
DEFINE_LOG_CATEGORY(LogEquipmentSystem);

AEquipmentBase::AEquipmentBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建碰撞组件
	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;

	// 创建装备网格组件
	EquipmentMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EquipmentMeshComponent"));
	EquipmentMeshComponent->SetupAttachment(RootComponent);

	// 初始化变量
	OwnerActor = nullptr;
	CurrentSlot = EEquipmentSlot::MAX;
}

void AEquipmentBase::BeginPlay()
{
	Super::BeginPlay();

	// 【安全检查】确保组件有效
	if (!IsValid(CollisionComponent))
	{
		UE_LOG(LogEquipmentSystem, Error, TEXT("[EquipmentBase] BeginPlay失败：CollisionComponent无效"));
		return;
	}

	// 设置碰撞属性
	CollisionComponent->SetCollisionProfileName(TEXT("Pickup"));
	CollisionComponent->SetGenerateOverlapEvents(true);

	// 更新装备外观（如果数据已设置）
	if (EquipmentData)
	{
		UpdateEquipmentAppearance();
	}
}

void AEquipmentBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AEquipmentBase::EquipToSlot(EEquipmentSlot InSlot, AActor* InOwner)
{
	if (!CanEquipToSlot(InSlot, InOwner))
	{
		return false;
	}

	// 设置装备状态
	InstanceData.bIsEquipped = true;
	CurrentSlot = InSlot;
	OwnerActor = InOwner;

	// 应用装备属性
	ApplyEquipmentStats(InOwner);

	// 播放装备音效
	PlayEquipSound();

	// 隐藏装备实体（如果需要）
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	return true;
}

bool AEquipmentBase::UnequipFromSlot()
{
	if (!CanUnequipFromSlot())
	{
		return false;
	}

	// 移除装备属性
	if (OwnerActor)
	{
		RemoveEquipmentStats(OwnerActor);
	}

	// 重置装备状态
	InstanceData.bIsEquipped = false;
	CurrentSlot = EEquipmentSlot::MAX;
	OwnerActor = nullptr;

	// 播放卸下音效
	PlayUnequipSound();

	// 显示装备实体
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	return true;
}

void AEquipmentBase::ApplyEquipmentStats(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	// 获取目标属性组件
	UAttributeComponent* AttributeComp = Target->FindComponentByClass<UAttributeComponent>();
	if (!AttributeComp)
	{
		UE_LOG(LogEquipmentSystem, Warning, TEXT("[EquipmentBase] 目标 %s 没有属性组件"), *Target->GetName());
		return;
	}

	// 获取装备总属性（包含强化加成）
	FEquipmentBaseStats TotalStats = GetTotalStats();

	// 【应用属性加成】使用附加值方式，不影响基础值
	// 攻击力
	if (TotalStats.Attack != 0.0f)
	{
		AttributeComp->AddAdditiveValue(EAttributeType::Attack, TotalStats.Attack);
	}

	// 防御力
	if (TotalStats.Defense != 0.0f)
	{
		AttributeComp->AddAdditiveValue(EAttributeType::Defense, TotalStats.Defense);
	}

	// 最大生命值
	if (TotalStats.MaxHealth != 0.0f)
	{
		AttributeComp->AddAdditiveValue(EAttributeType::MaxHealth, TotalStats.MaxHealth);
	}

	// 暴击率
	if (TotalStats.CriticalChance != 0.0f)
	{
		AttributeComp->AddAdditiveValue(EAttributeType::CriticalRate, TotalStats.CriticalChance);
	}

	// 暴击伤害
	if (TotalStats.CriticalDamage != 0.0f)
	{
		AttributeComp->AddAdditiveValue(EAttributeType::CriticalDamage, TotalStats.CriticalDamage);
	}

	// 移动速度
	if (TotalStats.MovementSpeed != 0.0f)
	{
		AttributeComp->AddAdditiveValue(EAttributeType::MoveSpeed, TotalStats.MovementSpeed);
	}

	// 金币获取率
	if (TotalStats.GoldFind != 0.0f)
	{
		AttributeComp->AddAdditiveValue(EAttributeType::GoldGainRate, TotalStats.GoldFind);
	}

	UE_LOG(LogEquipmentSystem, Log, TEXT("[EquipmentBase] %s 装备属性已应用到 %s"), *InstanceData.EquipmentName, *Target->GetName());
}

void AEquipmentBase::RemoveEquipmentStats(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	// 获取目标属性组件
	UAttributeComponent* AttributeComp = Target->FindComponentByClass<UAttributeComponent>();
	if (!AttributeComp)
	{
		return;
	}

	// 获取装备总属性（包含强化加成）
	FEquipmentBaseStats TotalStats = GetTotalStats();

	// 【移除属性加成】移除对应的附加值
	// 攻击力
	if (TotalStats.Attack != 0.0f)
	{
		AttributeComp->RemoveAdditiveValue(EAttributeType::Attack, TotalStats.Attack);
	}

	// 防御力
	if (TotalStats.Defense != 0.0f)
	{
		AttributeComp->RemoveAdditiveValue(EAttributeType::Defense, TotalStats.Defense);
	}

	// 最大生命值
	if (TotalStats.MaxHealth != 0.0f)
	{
		AttributeComp->RemoveAdditiveValue(EAttributeType::MaxHealth, TotalStats.MaxHealth);
	}

	// 暴击率
	if (TotalStats.CriticalChance != 0.0f)
	{
		AttributeComp->RemoveAdditiveValue(EAttributeType::CriticalRate, TotalStats.CriticalChance);
	}

	// 暴击伤害
	if (TotalStats.CriticalDamage != 0.0f)
	{
		AttributeComp->RemoveAdditiveValue(EAttributeType::CriticalDamage, TotalStats.CriticalDamage);
	}

	// 移动速度
	if (TotalStats.MovementSpeed != 0.0f)
	{
		AttributeComp->RemoveAdditiveValue(EAttributeType::MoveSpeed, TotalStats.MovementSpeed);
	}

	// 金币获取率
	if (TotalStats.GoldFind != 0.0f)
	{
		AttributeComp->RemoveAdditiveValue(EAttributeType::GoldGainRate, TotalStats.GoldFind);
	}

	UE_LOG(LogEquipmentSystem, Log, TEXT("[EquipmentBase] %s 装备属性已从 %s 移除"), *InstanceData.EquipmentName, *Target->GetName());
}

bool AEquipmentBase::ConsumeDurability(int32 Amount)
{
	InstanceData.Durability = FMath::Max(0, InstanceData.Durability - Amount);

	// 检查耐久度是否耗尽
	if (InstanceData.Durability <= 0)
	{
		// 耐久度耗尽，可能需要卸下装备
		if (InstanceData.bIsEquipped && OwnerActor)
		{
			UnequipFromSlot();
		}
		return true;
	}

	return false;
}

int32 AEquipmentBase::RepairEquipment(int32 GoldAmount)
{
	if (!EquipmentData)
	{
		return InstanceData.Durability;
	}

	int32 RepairCost = EquipmentData->CalculateRepairCost(InstanceData.Durability);

	if (GoldAmount < RepairCost)
	{
		// 金币不足，无法完全修理
		float RepairPercentage = (float)GoldAmount / (float)RepairCost;
		int32 RepairAmount = FMath::RoundToInt((InstanceData.MaxDurability - InstanceData.Durability) * RepairPercentage);
		InstanceData.Durability += RepairAmount;
	}
	else
	{
		// 完全修理
		InstanceData.Durability = InstanceData.MaxDurability;
	}

	return InstanceData.Durability;
}

int32 AEquipmentBase::ScrapEquipment()
{
	if (!EquipmentData)
	{
		return 0;
	}

	// 分解装备，返回材料数量
	int32 MaterialAmount = EquipmentData->ScrapMaterialAmount;

	// 销毁装备
	Destroy();

	return MaterialAmount;
}

FEquipmentBaseStats AEquipmentBase::GetTotalStats() const
{
	FEquipmentBaseStats TotalStats = InstanceData.BaseStats;

	// TODO: 计算词条带来的属性加成
	// 这里需要遍历词条，将词条的属性加成添加到总属性中

	// TODO: 计算强化等级带来的属性加成
	// 强化等级每级增加10%的基础属性
	float EnhancementMultiplier = 1.0f + (InstanceData.EnhancementLevel * 0.1f);
	TotalStats.Attack *= EnhancementMultiplier;
	TotalStats.Defense *= EnhancementMultiplier;
	TotalStats.MaxHealth *= EnhancementMultiplier;

	return TotalStats;
}

void AEquipmentBase::PlayEquipSound()
{
	if (EquipmentData && EquipmentData->EquipSound)
	{
		// 在装备位置播放音效
		UGameplayStatics::PlaySoundAtLocation(this, EquipmentData->EquipSound, GetActorLocation());
	}
}

void AEquipmentBase::PlayUnequipSound()
{
	if (EquipmentData && EquipmentData->UnequipSound)
	{
		// 在装备位置播放音效
		UGameplayStatics::PlaySoundAtLocation(this, EquipmentData->UnequipSound, GetActorLocation());
	}
}

void AEquipmentBase::UpdateEquipmentAppearance()
{
	// 【安全检查】确保组件有效
	if (!IsValid(EquipmentMeshComponent) || !IsValid(CollisionComponent))
	{
		UE_LOG(LogEquipmentSystem, Warning, TEXT("[EquipmentBase] 更新外观失败：组件无效"));
		return;
	}

	if (!EquipmentData)
	{
		UE_LOG(LogEquipmentSystem, Verbose, TEXT("[EquipmentBase] 更新外观跳过：EquipmentData为空"));
		return;
	}

	// 设置装备网格
	if (EquipmentData->EquipmentMesh)
	{
		EquipmentMeshComponent->SetSkeletalMesh(EquipmentData->EquipmentMesh);
		UE_LOG(LogEquipmentSystem, Verbose, TEXT("[EquipmentBase] 设置装备网格：%s"), *EquipmentData->EquipmentMesh->GetName());
	}

	// 设置装备材质
	if (EquipmentData->EquipmentMaterial)
	{
		EquipmentMeshComponent->SetMaterial(0, EquipmentData->EquipmentMaterial);
	}

	// 设置碰撞体积（使用新的API，并添加空值检查）
	if (USkeletalMesh* MeshAsset = EquipmentMeshComponent->GetSkeletalMeshAsset())
	{
		FVector Bounds = MeshAsset->GetBounds().BoxExtent;
		if (Bounds.Z > 0 && (Bounds.X > 0 || Bounds.Y > 0))
		{
			CollisionComponent->SetCapsuleHalfHeight(Bounds.Z);
			CollisionComponent->SetCapsuleRadius(FMath::Max(Bounds.X, Bounds.Y));
		}
	}
}

bool AEquipmentBase::CanEquipToSlot(EEquipmentSlot InSlot, AActor* InOwner) const
{
	if (!InOwner || !EquipmentData)
	{
		return false;
	}

	// 检查槽位是否匹配
	if (InstanceData.Slot != InSlot)
	{
		return false;
	}

	// 检查装备是否已经装备
	if (InstanceData.bIsEquipped)
	{
		return false;
	}

	// 检查装备是否损坏
	if (InstanceData.Durability <= 0)
	{
		return false;
	}

	return true;
}

bool AEquipmentBase::CanUnequipFromSlot() const
{
	return InstanceData.bIsEquipped && OwnerActor != nullptr;
}

AEquipmentBase* AEquipmentBase::SpawnEquipmentFromDataAsset(UObject* WorldContext, UEquipmentDataAsset* DataAsset, 
	const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	// 【安全检查】检查输入参数
	if (!WorldContext)
	{
		UE_LOG(LogEquipmentSystem, Warning, TEXT("[EquipmentBase] 生成装备失败：WorldContext为空"));
		return nullptr;
	}

	if (!DataAsset)
	{
		UE_LOG(LogEquipmentSystem, Warning, TEXT("[EquipmentBase] 生成装备失败：DataAsset为空"));
		return nullptr;
	}

	// 【获取World】使用 GEngine 作为备选获取 World
	UWorld* World = WorldContext->GetWorld();
	if (!World && GEngine)
	{
		// 如果 WorldContext 没有 World，尝试从 GameViewport 获取
		World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	}

	if (!World)
	{
		UE_LOG(LogEquipmentSystem, Error, TEXT("[EquipmentBase] 生成装备失败：无法获取World，WorldContext=%s"), 
			*WorldContext->GetName());
		return nullptr;
	}

	// 【生成装备Actor】
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	// 设置生成者（用于权限检查）
	SpawnParams.Instigator = Cast<APawn>(WorldContext);

	UE_LOG(LogEquipmentSystem, Log, TEXT("[EquipmentBase] 开始生成装备：%s 在位置 %s"), 
		*DataAsset->EquipmentName, *SpawnLocation.ToString());

	AEquipmentBase* Equipment = World->SpawnActor<AEquipmentBase>(AEquipmentBase::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
	if (!Equipment)
	{
		UE_LOG(LogEquipmentSystem, Error, TEXT("[EquipmentBase] 生成装备失败：SpawnActor返回nullptr"));
		return nullptr;
	}

	// 【初始化装备数据】
	Equipment->InitializeFromDataAsset(DataAsset);
	
	UE_LOG(LogEquipmentSystem, Log, TEXT("[EquipmentBase] 成功生成装备：%s"), *DataAsset->EquipmentName);

	return Equipment;
}

void AEquipmentBase::InitializeFromDataAsset(UEquipmentDataAsset* DataAsset)
{
	if (!DataAsset)
	{
		UE_LOG(LogEquipmentSystem, Warning, TEXT("[EquipmentBase] 初始化失败：DataAsset为空"));
		return;
	}

	// 保存数据资产引用
	EquipmentData = DataAsset;

	// 生成实例数据
	InstanceData = DataAsset->GenerateEquipmentInstance();

	// 检查实例数据是否有效
	if (InstanceData.EquipmentName.IsEmpty())
	{
		UE_LOG(LogEquipmentSystem, Warning, TEXT("[EquipmentBase] 警告：装备名称为空"));
		InstanceData.EquipmentName = TEXT("未命名装备");
	}

	// 更新外观（延迟一帧执行，避免构造函数中调用导致的崩溃）
	// 使用异步任务确保组件已完全初始化
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [this]()
	{
		if (IsValid(this))
		{
			UpdateEquipmentAppearance();
		}
	}, 0.01f, false);

	UE_LOG(LogEquipmentSystem, Log, TEXT("[EquipmentBase] 装备初始化完成：%s (品质: %d)"), 
		*InstanceData.EquipmentName, (int32)InstanceData.Quality);
}

float AEquipmentBase::GetDurabilityPercent() const
{
	if (InstanceData.MaxDurability <= 0)
	{
		return 0.0f;
	}
	return (float)InstanceData.Durability / (float)InstanceData.MaxDurability;
}

bool AEquipmentBase::IsBroken() const
{
	return InstanceData.Durability <= 0;
}
