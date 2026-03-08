// Copyright Epic Games, Inc. All Rights Reserved.

#include "EquipmentDropItem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AEquipmentDropItem::AEquipmentDropItem(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 装备默认配置
    Quality = EEquipmentQuality::Common;
    bEnableAutoMagnet = true;
    MagnetRange = 250.0f;  // 装备吸附范围较小
    MagnetSpeed = 600.0f;
    LifeTime = 45.0f;  // 装备存在时间更长

    // 【修复】设置默认装备网格体（立方体）
    if (MeshComponent)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Cube"));
        if (MeshFinder.Succeeded())
        {
            MeshComponent->SetStaticMesh(MeshFinder.Object);
            MeshComponent->SetRelativeScale3D(FVector(0.5f));  // 适当大小
        }
    }
    
    // 【注意】品质颜色映射在蓝图默认值中配置，不在C++构造函数中初始化
}

void AEquipmentDropItem::InitializeDrop(const FVector& SpawnLocation, const FVector& SpawnImpulse)
{
    Super::InitializeDrop(SpawnLocation, SpawnImpulse);

    // 根据品质更新颜色
    UpdateMeshColor();

    UE_LOG(LogDropItem, Verbose, TEXT("[EquipmentDropItem] 装备掉落物初始化，品质: %d"), 
        static_cast<int32>(Quality));
}

void AEquipmentDropItem::SetEquipmentQuality(EEquipmentQuality InQuality)
{
    Quality = InQuality;
    UpdateMeshColor();
}

FLinearColor AEquipmentDropItem::GetQualityColor() const
{
    if (const FLinearColor* Color = QualityColors.Find(Quality))
    {
        return *Color;
    }
    return FLinearColor::White;
}

void AEquipmentDropItem::UpdateMeshColor()
{
    if (!MeshComponent)
    {
        return;
    }

    // 创建动态材质实例并设置颜色
    // 注意：这里假设网格体使用可以设置颜色的材质
    // 实际项目中可能需要根据具体材质参数调整
    FLinearColor Color = GetQualityColor();
    
    // 如果有动态材质实例，可以设置参数
    // 这里简化处理，仅记录日志
    UE_LOG(LogDropItem, Verbose, TEXT("[EquipmentDropItem] 设置装备颜色: R=%.2f G=%.2f B=%.2f"),
        Color.R, Color.G, Color.B);
}

void AEquipmentDropItem::OnPickup_Implementation(APawn* Player)
{
    UE_LOG(LogDropItem, Log, TEXT("[EquipmentDropItem] 玩家 %s 拾取 %s 品质装备"),
        *Player->GetName(), *UEnum::GetValueAsString(Quality));

    // 播放拾取音效（带空值保护）
    if (PickupSound.IsValid())
    {
        if (USoundBase* Sound = PickupSound.Get())
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
        }
    }

    // 播放品质对应的特效（带自动销毁）
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

    // 应用装备到玩家
    ApplyEquipmentToPlayer(Player, Quality);

    // 调用父类实现
    Super::OnPickup_Implementation(Player);
}

void AEquipmentDropItem::ApplyEquipmentToPlayer_Implementation(APawn* Player, EEquipmentQuality InQuality)
{
    // 基础实现：输出日志
    // 实际项目中应该调用玩家的装备系统
    UE_LOG(LogDropItem, Log, TEXT("[EquipmentDropItem] 向玩家 %s 添加 %s 品质装备（需要在子类或蓝图中实现具体逻辑）"),
        *Player->GetName(), *UEnum::GetValueAsString(InQuality));

    // TODO: 调用玩家的装备系统
    // 例如：PlayerState->AddEquipment(InQuality);
}
