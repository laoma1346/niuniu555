#include "CombatRoom.h"
#include "EnemySystem/EnemyBase.h"
#include "EnemySystem/EnemySpawner.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ACombatRoom::ACombatRoom()
{
	RoomType = ERoomType::Combat;
	CombatType = ECombatRoomType::Normal;
}

void ACombatRoom::BeginPlay()
{
	Super::BeginPlay();

	// 缓存总波次数
	TotalWaves = WaveConfigs.Num();
	CurrentWave = 0;
	AliveEnemyCount = 0;

	// 延迟查找生成器，确保所有Actor初始化完成（保存定时器句柄）
	GetWorldTimerManager().SetTimer(FindSpawnersTimerHandle, this, &ACombatRoom::FindEnemySpawners, 0.5f, false);
}

void ACombatRoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ✅ 安全检查
	if (!IsValid(this) || !IsValid(GetWorld()))
	{
		return;
	}

	// ✅ 性能优化：降低检查频率，每0.5秒检查一次
	static float AccumulatedTime = 0.0f;
	AccumulatedTime += DeltaTime;
	
	if (AccumulatedTime < 0.5f)
	{
		return;
	}
	AccumulatedTime = 0.0f;

	// 清理已死亡的敌人（WeakPointer失效）
	bool bAnyRemoved = false;
	for (int32 i = AliveEnemies.Num() - 1; i >= 0; --i)
	{
		if (AliveEnemies.IsValidIndex(i) && !AliveEnemies[i].IsValid())
		{
			AliveEnemies.RemoveAt(i);
			bAnyRemoved = true;
		}
	}

	if (bAnyRemoved)
	{
		AliveEnemyCount = AliveEnemies.Num();
		CheckWaveComplete();
	}
}

void ACombatRoom::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// ✅ 标记正在清理中
	bIsCleaningUp = true;

	// ✅ 重要：先执行自己的清理，再调用Super::EndPlay
	
	// 清理所有定时器
	if (IsValid(GetWorld()))
	{
		GetWorldTimerManager().ClearTimer(FindSpawnersTimerHandle);
		GetWorldTimerManager().ClearTimer(WaveDelayTimerHandle);
		GetWorldTimerManager().ClearTimer(WaveCheckTimerHandle);
		
		// 清理所有生成定时器
		for (FTimerHandle& Handle : SpawnTimerHandles)
		{
			GetWorldTimerManager().ClearTimer(Handle);
		}
	}
	SpawnTimerHandles.Empty();
	
	// 清理活动波次配置
	ActiveWaveConfigs.Empty();
	CurrentSpawnGroupIndex = 0;

	// ✅ 解绑所有敌人死亡事件（使用副本遍历避免修改时迭代）
	TArray<TWeakObjectPtr<AEnemyBase>> EnemiesToUnbind = BoundEnemies;
	for (TWeakObjectPtr<AEnemyBase>& WeakEnemy : EnemiesToUnbind)
	{
		if (WeakEnemy.IsValid() && IsValid(WeakEnemy.Get()))
		{
			WeakEnemy->OnDeath.RemoveDynamic(this, &ACombatRoom::OnEnemyDeath);
		}
	}
	BoundEnemies.Empty();

	// 最后调用Super（Super会清理RoomBase的定时器）
	Super::EndPlay(EndPlayReason);
}

void ACombatRoom::OnEnterRoom_Implementation()
{
	Super::OnEnterRoom_Implementation();

	// ✅ 安全检查：确保World有效
	if (!IsValid(GetWorld()))
	{
		UE_LOG(LogTemp, Error, TEXT("[CombatRoom] OnEnterRoom: World无效"));
		return;
	}

	// 检查波次配置
	if (CombatType != ECombatRoomType::Boss && WaveConfigs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatRoom] 警告：没有配置波次！请检查 WaveConfigs"));
	}

	LogRoom(FString::Printf(TEXT("进入战斗房间 (类型: %s, 波次数: %d)"),
		CombatType == ECombatRoomType::Boss ? TEXT("BOSS") : 
		CombatType == ECombatRoomType::Elite ? TEXT("精英") : TEXT("普通"),
		TotalWaves));

	// 开始第一波
	if (CombatType == ECombatRoomType::Boss)
	{
		// BOSS房直接刷BOSS
		SpawnBoss();
	}
	else if (WaveConfigs.Num() > 0)
	{
		StartWave(0);
	}

	// 启动波次检查定时器（0.3秒检查一次，提高响应速度）
	GetWorldTimerManager().SetTimer(WaveCheckTimerHandle, this, &ACombatRoom::CheckWaveComplete, 0.3f, true);
}

void ACombatRoom::OnCompleteRoom_Implementation()
{
	Super::OnCompleteRoom_Implementation();

	// 清除定时器
	GetWorldTimerManager().ClearTimer(WaveCheckTimerHandle);

	// 发放奖励
	GiveRewards();

	LogRoom(TEXT("战斗房间完成"));
}

void ACombatRoom::SpawnBoss()
{
	// ✅ 安全检查
	if (!IsValid(GetWorld()))
	{
		UE_LOG(LogTemp, Error, TEXT("[CombatRoom] SpawnBoss: World无效"));
		return;
	}

	if (!BossConfig.BossClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatRoom] BOSS配置无效，未设置BossClass"));
		return;
	}

	LogRoom(FString::Printf(TEXT("生成BOSS: %s"), *BossConfig.BossName));

	// 使用第一个生成器生成BOSS，如果没有则在当前位置生成
	FVector SpawnLocation = GetActorLocation();
	if (EnemySpawners.Num() > 0 && IsValid(EnemySpawners[0]))
	{
		SpawnLocation = EnemySpawners[0]->GetActorLocation();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyBase* Boss = GetWorld()->SpawnActor<AEnemyBase>(BossConfig.BossClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (IsValid(Boss))
	{
		RegisterSpawnedEnemy(Boss);
		LogRoom(FString::Printf(TEXT("BOSS %s 生成成功"), *BossConfig.BossName));
	}
}

void ACombatRoom::StartWave(int32 WaveIndex)
{
	// ✅ 安全检查
	if (!IsValid(GetWorld()))
	{
		UE_LOG(LogTemp, Error, TEXT("[CombatRoom] StartWave: World无效"));
		return;
	}

	if (!WaveConfigs.IsValidIndex(WaveIndex))
	{
		return;
	}

	CurrentWave = WaveIndex;
	const FCombatWaveConfig& Config = WaveConfigs[WaveIndex];

	LogRoom(FString::Printf(TEXT("开始波次 %d: %s"), WaveIndex, *Config.WaveName));
	
	// 检查SpawnGroups是否为空
	if (Config.SpawnGroups.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatRoom] 波次 %d 没有配置SpawnGroups！请在WaveConfigs中配置敌人生成组。"), WaveIndex);
		// 如果没有配置，直接标记完成
		bWaveSpawnComplete = true;
		return;
	}
	
	int32 TotalEnemies = 0;
	for (const auto& Group : Config.SpawnGroups)
	{
		TotalEnemies += Group.Count;
	}
	LogRoom(FString::Printf(TEXT("波次 %d 配置: %d个生成组, 总敌人数量: %d, 延迟: %.1f秒"), 
		WaveIndex, Config.SpawnGroups.Num(), TotalEnemies, Config.DelayBeforeStart));

	bWaveInProgress = true;
	bWaveSpawnComplete = false;  // 重置生成完成标志
	CurrentSpawnGroupIndex = 0;  // 重置生成组索引
	
	// 保存波次配置到临时存储
	ActiveWaveConfigs = WaveConfigs;

	// 延迟开始生成 - 使用安全的成员函数绑定而不是Lambda
	FTimerDelegate SpawnDelegate;
	SpawnDelegate.BindUObject(this, &ACombatRoom::ProcessDelayedSpawn);
	GetWorldTimerManager().SetTimer(WaveDelayTimerHandle, SpawnDelegate, Config.DelayBeforeStart, false);

	OnWaveStarted.Broadcast(WaveIndex);
}

void ACombatRoom::ProcessDelayedSpawn()
{
	// 检查当前CombatRoom是否有效
	if (!IsValid(this))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatRoom] ProcessDelayedSpawn: CombatRoom无效"));
		return;
	}
	
	// 检查当前波次配置是否有效
	if (!ActiveWaveConfigs.IsValidIndex(CurrentWave))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatRoom] ProcessDelayedSpawn: 波次配置无效"));
		bWaveSpawnComplete = true;
		return;
	}
	
	const FCombatWaveConfig& Config = ActiveWaveConfigs[CurrentWave];
	
	// 生成敌人
	int32 SpawnedGroups = 0;
	for (const FEnemySpawnGroup& Group : Config.SpawnGroups)
	{
		if (IsValid(this))
		{
			SpawnEnemyGroup(Group);
			SpawnedGroups++;
		}
	}
	
	// 标记敌人生成完成
	if (IsValid(this))
	{
		bWaveSpawnComplete = true;
		UE_LOG(LogTemp, Log, TEXT("[CombatRoom] 波次敌人生成完成，共生成 %d 个组"), SpawnedGroups);
	}
}

void ACombatRoom::SpawnEnemyGroup(const FEnemySpawnGroup& Group)
{
	// ✅ 安全检查
	if (!IsValid(GetWorld()))
	{
		UE_LOG(LogTemp, Error, TEXT("[CombatRoom] SpawnEnemyGroup: World无效"));
		return;
	}

	if (!Group.EnemyClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatRoom] SpawnEnemyGroup: EnemyClass为空"));
		return;
	}

	// 选择生成位置
	FVector SpawnLocation = GetActorLocation();
	if (Group.SpawnerIndex >= 0 && Group.SpawnerIndex < EnemySpawners.Num() && IsValid(EnemySpawners[Group.SpawnerIndex]))
	{
		SpawnLocation = EnemySpawners[Group.SpawnerIndex]->GetActorLocation();
	}
	else if (EnemySpawners.Num() > 0)
	{
		// 随机选择一个生成器
		int32 RandomIndex = FMath::RandRange(0, EnemySpawners.Num() - 1);
		if (IsValid(EnemySpawners[RandomIndex]))
		{
			SpawnLocation = EnemySpawners[RandomIndex]->GetActorLocation();
		}
	}

	// 立即生成所有敌人（不使用Lambda定时器，避免崩溃）
	for (int32 i = 0; i < Group.Count; ++i)
	{
		// 添加随机偏移
		FVector RandomOffset = FVector(FMath::RandRange(-100.0f, 100.0f), FMath::RandRange(-100.0f, 100.0f), 0.0f);
		FVector FinalLocation = SpawnLocation + RandomOffset;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AEnemyBase* Enemy = GetWorld()->SpawnActor<AEnemyBase>(Group.EnemyClass, FinalLocation, FRotator::ZeroRotator, SpawnParams);
		if (IsValid(Enemy))
		{
			RegisterSpawnedEnemy(Enemy);
		}
	}
}

void ACombatRoom::StartNextWave()
{
	UE_LOG(LogTemp, Log, TEXT("[CombatRoom] StartNextWave 被调用! CurrentWave=%d, TotalWaves=%d"), CurrentWave, TotalWaves);
	
	if (CurrentWave + 1 < TotalWaves)
	{
		UE_LOG(LogTemp, Log, TEXT("[CombatRoom] 准备开始下一波: %d"), CurrentWave + 1);
		StartWave(CurrentWave + 1);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[CombatRoom] 所有波次已完成，调用CompleteRoom()"));
		// 所有波次完成
		CompleteRoom();
	}
}

void ACombatRoom::CheckWaveComplete()
{
	// ✅ 安全检查
	if (!IsValid(this) || !IsValid(GetWorld()))
	{
		return;
	}

	if (!bWaveInProgress)
	{
		return;
	}

	// 关键修复：等待敌人生成完成后再检查
	if (!bWaveSpawnComplete)
	{
		return;
	}

	// 添加调试日志
	UE_LOG(LogTemp, Log, TEXT("[CombatRoom] CheckWaveComplete: CurrentWave=%d, AliveEnemyCount=%d, bWaveSpawnComplete=%s"), 
		CurrentWave, AliveEnemyCount, bWaveSpawnComplete ? TEXT("true") : TEXT("false"));

	// BOSS房特殊处理
	if (CombatType == ECombatRoomType::Boss)
	{
		if (AliveEnemyCount == 0)
		{
			bWaveInProgress = false;
			CompleteRoom();
		}
		return;
	}

	if (!WaveConfigs.IsValidIndex(CurrentWave))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatRoom] CheckWaveComplete: CurrentWave %d 不是有效索引!"), CurrentWave);
		return;
	}

	const FCombatWaveConfig& Config = WaveConfigs[CurrentWave];

	UE_LOG(LogTemp, Log, TEXT("[CombatRoom] CheckWaveComplete: bWaitForClear=%s, AliveEnemyCount=%d"), 
		Config.bWaitForClear ? TEXT("true") : TEXT("false"), AliveEnemyCount);

	// 检查是否等待清空
	if (Config.bWaitForClear && AliveEnemyCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[CombatRoom] CheckWaveComplete: 等待清空敌人，AliveEnemyCount=%d > 0"), AliveEnemyCount);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[CombatRoom] CheckWaveComplete: 检查目标达成..."));

	// 检查目标达成
	if (CheckObjectiveComplete())
	{
		UE_LOG(LogTemp, Log, TEXT("[CombatRoom] CheckWaveComplete: 目标达成! 准备结束波次 %d"), CurrentWave);
		bWaveInProgress = false;
		OnWaveCompleted.Broadcast(CurrentWave);

		// 自动开始下一波
		UE_LOG(LogTemp, Log, TEXT("[CombatRoom] CheckWaveComplete: 调用StartNextWave()"));
		StartNextWave();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[CombatRoom] CheckWaveComplete: 目标未达成"));
	}
}

bool ACombatRoom::AreAllWavesComplete() const
{
	// BOSS房特殊处理
	if (CombatType == ECombatRoomType::Boss)
	{
		return !bWaveInProgress && AliveEnemyCount == 0;
	}
	
	// 没有配置波次（非BOSS房），视为完成
	if (TotalWaves == 0)
	{
		return true;
	}
	
	// 所有波次已启动且完成，且没有存活敌人
	// CurrentWave从0开始，所以最后一波是TotalWaves - 1
	// 需要确保：
	// 1. 当前波次是最后一波或之后
	// 2. 波次不在进行中
	// 3. 没有存活敌人
	return CurrentWave >= TotalWaves - 1 && !bWaveInProgress && AliveEnemyCount == 0;
}

void ACombatRoom::RegisterSpawnedEnemy(AActor* Enemy)
{
	// ✅ 安全检查
	if (!IsValid(Enemy))
	{
		return;
	}

	AliveEnemies.Add(Enemy);
	AliveEnemyCount = AliveEnemies.Num();

	// 绑定死亡事件
	AEnemyBase* EnemyBase = Cast<AEnemyBase>(Enemy);
	if (IsValid(EnemyBase))
	{
		// ✅ 绑定OnDeath事件到回调函数
		EnemyBase->OnDeath.AddDynamic(this, &ACombatRoom::OnEnemyDeath);
		BoundEnemies.Add(EnemyBase);
		
		LogRoom(FString::Printf(TEXT("绑定敌人死亡事件: %s"), *Enemy->GetName()));
	}

	LogRoom(FString::Printf(TEXT("注册敌人 %s，当前存活: %d"), *Enemy->GetName(), AliveEnemyCount));
}

void ACombatRoom::UnregisterEnemy(AActor* Enemy)
{
	UE_LOG(LogTemp, Log, TEXT("[CombatRoom] UnregisterEnemy 被调用: %s"), *Enemy->GetName());
	
	// ✅ 安全检查
	if (!IsValid(Enemy))
	{
		return;
	}

	// 从TWeakObjectPtr数组中移除（需要正确比较）
	bool bRemoved = false;
	for (int32 i = AliveEnemies.Num() - 1; i >= 0; --i)
	{
		if (AliveEnemies.IsValidIndex(i) && AliveEnemies[i].Get() == Enemy)
		{
			AliveEnemies.RemoveAt(i);
			bRemoved = true;
			UE_LOG(LogTemp, Log, TEXT("[CombatRoom] 从AliveEnemies中移除敌人: %s"), *Enemy->GetName());
			break;
		}
	}

	if (bRemoved)
	{
		AliveEnemyCount = AliveEnemies.Num();
		UE_LOG(LogTemp, Log, TEXT("[CombatRoom] UnregisterEnemy后AliveEnemyCount=%d"), AliveEnemyCount);
		
		OnEnemyKilled.Broadcast(Enemy, AliveEnemyCount);

		// 如果是目标类型敌人，更新进度
		if (ObjectiveType == ECombatObjectiveType::KillCount ||
			ObjectiveType == ECombatObjectiveType::KillBoss)
		{
			UpdateObjectiveProgress(1.0f);
		}

		LogRoom(FString::Printf(TEXT("敌人死亡 %s，剩余: %d"), *Enemy->GetName(), AliveEnemyCount));
		
		// 立即检查波次是否完成
		CheckWaveComplete();
	}
}

void ACombatRoom::OnEnemyDeath()
{
	UE_LOG(LogTemp, Log, TEXT("[CombatRoom] OnEnemyDeath 被调用!"));
	
	// ✅ 安全检查：如果正在清理中或对象已无效，直接返回
	if (!IsValid(this) || bIsCleaningUp)
	{
		return;
	}

	// ✅ 额外检查：确保World仍然有效
	if (!IsValid(GetWorld()))
	{
		return;
	}

	// 这个回调主要处理WeakPtr清理，但我们主要依靠UnregisterEnemy来处理
	// 这里只做额外的安全清理
	bool bAnyRemoved = false;
	for (int32 i = AliveEnemies.Num() - 1; i >= 0; --i)
	{
		if (AliveEnemies.IsValidIndex(i) && !AliveEnemies[i].IsValid())
		{
			AliveEnemies.RemoveAt(i);
			bAnyRemoved = true;
		}
	}

	if (bAnyRemoved && IsValid(this) && !bIsCleaningUp)
	{
		AliveEnemyCount = AliveEnemies.Num();
		LogRoom(FString::Printf(TEXT("敌人死亡清理后，剩余: %d"), AliveEnemyCount));
		CheckWaveComplete();
	}
}

void ACombatRoom::UpdateObjectiveProgress(float Delta)
{
	CurrentObjectiveProgress += Delta;

	if (ObjectiveValue > 0.0f)
	{
		float Percent = FMath::Clamp(CurrentObjectiveProgress / ObjectiveValue, 0.0f, 1.0f);
		OnObjectiveProgressUpdated.Broadcast(Percent);
	}
}

bool ACombatRoom::CheckObjectiveComplete() const
{
	switch (ObjectiveType)
	{
	case ECombatObjectiveType::KillAll:
		return AliveEnemyCount == 0;

	case ECombatObjectiveType::KillCount:
	case ECombatObjectiveType::KillBoss:
		return CurrentObjectiveProgress >= ObjectiveValue;

	default:
		return false;
	}
}

void ACombatRoom::GiveRewards()
{
	// ✅ 安全检查
	if (!IsValid(this) || !IsValid(GetWorld()))
	{
		return;
	}

	// 计算奖励倍数
	float Multiplier = 1.0f;
	if (CombatType == ECombatRoomType::Elite)
	{
		Multiplier = 1.5f;
	}
	else if (CombatType == ECombatRoomType::Boss)
	{
		Multiplier = 3.0f;
	}

	int32 FinalGold = FMath::RoundToInt(BaseGoldReward * Multiplier);
	int32 FinalExp = FMath::RoundToInt(BaseExpReward * Multiplier);

	LogRoom(FString::Printf(TEXT("发放奖励: 金币 %d, 经验 %d"), FinalGold, FinalExp));

	// 发放奖励给玩家
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (IsValid(PC) && IsValid(PC->GetPawn()))
	{
		// TODO: 实现玩家奖励接收接口
		// 可以通过GameInstance或PlayerState来发放奖励
		UE_LOG(LogTemp, Log, TEXT("[CombatRoom] 向玩家发放奖励: 金币 %d, 经验 %d"), FinalGold, FinalExp);
	}
}

void ACombatRoom::FindEnemySpawners()
{
	EnemySpawners.Empty();

	// 安全获取 World
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatRoom] FindEnemySpawners: World 无效"));
		return;
	}

	// 查找场景中的所有生成器
	for (TActorIterator<AEnemySpawner> It(World); It; ++It)
	{
		if (IsValid(*It))  // 检查每个指针的有效性
		{
			EnemySpawners.Add(*It);
		}
	}

	LogRoom(FString::Printf(TEXT("找到 %d 个敌人生成器"), EnemySpawners.Num()));

	// 警告：如果没有找到生成器，提示用户
	if (EnemySpawners.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatRoom] 未找到任何敌人生成器！请在场景中添加EnemySpawner，或使用RegisterEnemySpawner手动注册。"));
	}
}

void ACombatRoom::RegisterEnemySpawner(AEnemySpawner* Spawner)
{
	if (!Spawner)
	{
		return;
	}

	if (!EnemySpawners.Contains(Spawner))
	{
		EnemySpawners.Add(Spawner);
		LogRoom(FString::Printf(TEXT("手动注册敌人生成器: %s"), *Spawner->GetName()));
	}
}

void ACombatRoom::ClearEnemySpawners()
{
	EnemySpawners.Empty();
	LogRoom(TEXT("清除所有敌人生成器注册"));
}
