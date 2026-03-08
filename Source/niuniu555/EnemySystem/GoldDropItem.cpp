// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoldDropItem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AGoldDropItem::AGoldDropItem(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 金币默认配置
    GoldAmount = 10;
    bEnableAutoMagnet = true;
    MagnetRange = 400.0f;  // 金币吸附范围更大
    MagnetSpeed = 1000.0f;
    LifeTime = 20.0f;  // 金币存在时间稍短

    // 【修复】设置默认金币网格体（球体）
    if (MeshComponent)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Sphere"));
        if (MeshFinder.Succeeded())
        {
            MeshComponent->SetStaticMesh(MeshFinder.Object);
            MeshComponent->SetRelativeScale3D(FVector(0.3f));  // 缩小到合适大小
        }
    }
}

void AGoldDropItem::InitializeDrop(const FVector& SpawnLocation, const FVector& SpawnImpulse)
{
    Super::InitializeDrop(SpawnLocation, SpawnImpulse);

    UE_LOG(LogDropItem, Verbose, TEXT("[GoldDropItem] 金币掉落物初始化，数量: %d"), GoldAmount);
}

void AGoldDropItem::SetGoldAmount(int32 Amount)
{
    GoldAmount = FMath::Max(1, Amount);
}

void AGoldDropItem::OnPickup_Implementation(APawn* Player)
{
    UE_LOG(LogDropItem, Log, TEXT("[GoldDropItem] 玩家 %s 拾取 %d 金币"), *Player->GetName(), GoldAmount);

    // 播放拾取音效（带空值保护）
    if (PickupSound.IsValid())
    {
        if (USoundBase* Sound = PickupSound.Get())
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
        }
    }

    // 播放拾取特效（带自动销毁）
    if (PickupEffect.IsValid())
    {
        if (UParticleSystem* Effect = PickupEffect.Get())
        {
            UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(), Effect, GetActorLocation());
            if (ParticleComp)
            {
                ParticleComp->bAutoDestroy = true;
            }
        }
    }

    // 应用金币到玩家
    ApplyGoldToPlayer(Player, GoldAmount);

    // 调用父类实现（广播事件和销毁）
    Super::OnPickup_Implementation(Player);
}

void AGoldDropItem::ApplyGoldToPlayer_Implementation(APawn* Player, int32 Amount)
{
    // 基础实现：输出日志
    // 实际项目中应该调用玩家的金币系统增加金币
    UE_LOG(LogDropItem, Log, TEXT("[GoldDropItem] 向玩家 %s 添加 %d 金币（需要在子类或蓝图中实现具体逻辑）"),
        *Player->GetName(), Amount);

    // TODO: 调用玩家的金币系统
    // 例如：PlayerState->AddGold(Amount);
}
