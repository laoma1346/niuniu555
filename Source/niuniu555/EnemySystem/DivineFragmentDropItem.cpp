// Copyright Epic Games, Inc. All Rights Reserved.

#include "DivineFragmentDropItem.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogDivineFragment, Log, All);

ADivineFragmentDropItem::ADivineFragmentDropItem(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 【重要】不要在构造函数中操作FragmentColors TMap！
    // 这会导致编辑器在蓝图预览/编译时崩溃。
    // 颜色配置请在蓝图默认值(BP_DivineFragment)中设置。
    
    // 【修复】设置默认碎片网格体（球体）
    if (MeshComponent)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Sphere"));
        if (MeshFinder.Succeeded())
        {
            MeshComponent->SetStaticMesh(MeshFinder.Object);
            MeshComponent->SetRelativeScale3D(FVector(0.25f));  // 碎片比金币小一点
        }
    }
}

void ADivineFragmentDropItem::InitializeDrop(const FVector& SpawnLocation, const FVector& SpawnImpulse)
{
    Super::InitializeDrop(SpawnLocation, SpawnImpulse);

    UE_LOG(LogDivineFragment, Log, TEXT("【碎片掉落】初始化 %s x%d 在 %s"),
        *UEnum::GetValueAsString(FragmentType), FragmentCount, *SpawnLocation.ToString());
}

void ADivineFragmentDropItem::SetFragmentType(EDivineFragmentType InType)
{
    FragmentType = InType;
}

void ADivineFragmentDropItem::SetFragmentCount(int32 Count)
{
    FragmentCount = FMath::Max(1, Count);
}

void ADivineFragmentDropItem::OnPickup_Implementation(APawn* Player)
{
    // 【修复】防重复保护检查（提前到最前面）
    if (bAlreadyPicked)
    {
        return;
    }
    
    // 应用碎片到玩家
    ApplyFragmentToPlayer(Player, FragmentType, FragmentCount);

    // 播放音效
    if (PickupSound.IsValid())
    {
        USoundBase* Sound = PickupSound.LoadSynchronous();
        if (Sound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
        }
    }

    // 播放拾取特效（带自动销毁）
    if (PickupEffect.IsValid())
    {
        UParticleSystem* Effect = PickupEffect.LoadSynchronous();
        if (Effect)
        {
            UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(), Effect, GetActorLocation());
            if (ParticleComp)
            {
                ParticleComp->bAutoDestroy = true;
            }
        }
    }

    // 输出拾取日志（只输出一次）
    UE_LOG(LogDivineFragment, Log, TEXT("【碎片拾取】玩家 %s 获得 %s x%d"),
        *Player->GetName(), *UEnum::GetValueAsString(FragmentType), FragmentCount);
}

void ADivineFragmentDropItem::ApplyFragmentToPlayer_Implementation(APawn* Player, EDivineFragmentType InType, int32 Count)
{
    // TODO: 集成到玩家资源管理器
    // 这里先输出日志，后续会连接到玩家货币系统
    UE_LOG(LogDivineFragment, Log, TEXT("【碎片应用】玩家 %s 获得 %s x%d - 待连接到资源管理器"),
        *Player->GetName(), *UEnum::GetValueAsString(InType), Count);

    // 蓝图扩展点：可以在这里广播事件或调用玩家货币接口
    // 例如：PlayerResourceComp->AddFragment(InType, Count);
}

FLinearColor ADivineFragmentDropItem::GetFragmentColor() const
{
    if (FragmentColors.Contains(FragmentType))
    {
        return FragmentColors[FragmentType];
    }
    return FLinearColor::White;
}
