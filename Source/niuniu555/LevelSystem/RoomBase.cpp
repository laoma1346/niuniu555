#include "RoomBase.h"
#include "RoomManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogRoomSystem);

ARoomBase::ARoomBase()
{
	PrimaryActorTick.bCanEverTick = false;
	RoomType = ERoomType::None;
	RoomState = ERoomState::NotEntered;
}

void ARoomBase::BeginPlay()
{
	Super::BeginPlay();

	// 添加World空检查
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogRoomSystem, Error, TEXT("[%s] BeginPlay: World无效"), *GetName());
		return;
	}

	LogRoom(FString::Printf(TEXT("房间初始化: %s (类型: %s)"), 
		*RoomName, *GetRoomTypeName(RoomType)));

	// 延迟注册到 RoomManager，确保所有子系统初始化完成
	// ✅ 使用成员函数绑定而非Lambda，避免后台线程崩溃
	FTimerDelegate RegisterDelegate;
	RegisterDelegate.BindUObject(this, &ARoomBase::ExecuteDelayedRegistration);
	World->GetTimerManager().SetTimer(RegisterDelayHandle, RegisterDelegate, 0.2f, false);
}

void ARoomBase::ExecuteDelayedRegistration()
{
	// ✅ 检查自身是否仍然有效
	if (!IsValid(this))
	{
		return;
	}

	// 注册到RoomManager
	if (URoomManager* RM = URoomManager::Get(this))
	{
		RoomManagerPtr = RM;  // 缓存引用
		RM->RegisterRoom(this);
		LogRoom(TEXT("房间注册到管理器成功"));
	}
	else
	{
		UE_LOG(LogRoomSystem, Warning, TEXT("[%s] 无法获取RoomManager，房间未注册"), *GetName());
	}
}

void ARoomBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// ✅ 重要：先执行自己的清理，再调用Super::EndPlay
	
	// 清除所有属于该Actor的定时器
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}

	// 使用缓存的管理器指针注销（避免重新查找已销毁的子系统）
	if (IsValid(RoomManagerPtr))
	{
		RoomManagerPtr->UnregisterRoom(this);
		RoomManagerPtr = nullptr;  // 清空引用
	}

	// 广播销毁事件
	OnRoomDestroyed.Broadcast(this);

	LogRoom(TEXT("房间销毁"));

	// 最后调用Super
	Super::EndPlay(EndPlayReason);
}

void ARoomBase::EnterRoom()
{
	if (RoomState == ERoomState::Locked)
	{
		LogRoom(TEXT("进入房间失败：房间已锁定"));
		return;
	}

	// ✅ 安全检查
	if (!IsValid(this))
	{
		UE_LOG(LogTemp, Error, TEXT("[RoomBase] EnterRoom: this无效"));
		return;
	}

	RoomState = ERoomState::InProgress;
	
	// 调用蓝图实现
	OnEnterRoom();
	
	// 广播事件前再次检查
	if (IsValid(this))
	{
		OnRoomEntered.Broadcast(this);
	}

	LogRoom(TEXT("进入房间"));
}

void ARoomBase::CompleteRoom()
{
	if (RoomState != ERoomState::InProgress)
	{
		LogRoom(TEXT("完成房间失败：房间不在进行中"));
		return;
	}

	// ✅ 安全检查
	if (!IsValid(this))
	{
		return;
	}

	RoomState = ERoomState::Completed;
	OnCompleteRoom();
	
	// ✅ 广播前再次检查
	if (IsValid(this))
	{
		OnRoomCompleted.Broadcast(this);
	}

	LogRoom(TEXT("完成房间"));
}

void ARoomBase::SkipRoom()
{
	if (RoomState == ERoomState::Completed)
	{
		LogRoom(TEXT("跳过房间失败：房间已完成"));
		return;
	}

	// ✅ 安全检查
	if (!IsValid(this))
	{
		return;
	}

	RoomState = ERoomState::Skipped;
	OnSkipRoom();
	
	// ✅ 广播前再次检查
	if (IsValid(this))
	{
		OnRoomSkipped.Broadcast(this);
	}

	LogRoom(TEXT("跳过房间"));
}

void ARoomBase::ResetRoom()
{
	RoomState = ERoomState::NotEntered;

	LogRoom(TEXT("重置房间"));
}

bool ARoomBase::CanEnter() const
{
	return RoomState == ERoomState::NotEntered || 
		   RoomState == ERoomState::InProgress;
}

void ARoomBase::OnEnterRoom_Implementation()
{
	// 基础实现，子类可以覆盖
	// 可以在这里播放进入动画、加载场景等
}

void ARoomBase::OnCompleteRoom_Implementation()
{
	// 基础实现，子类可以覆盖
	// 可以在这里发放奖励、开启出口等
}

void ARoomBase::OnSkipRoom_Implementation()
{
	// 基础实现，子类可以覆盖
}

FString ARoomBase::GetRoomTypeName(ERoomType Type)
{
	switch (Type)
	{
	case ERoomType::None: return TEXT("无");
	case ERoomType::Start: return TEXT("起点");
	case ERoomType::Combat: return TEXT("战斗房");
	case ERoomType::Event: return TEXT("事件房");
	case ERoomType::Shop: return TEXT("商店房");
	case ERoomType::Skill: return TEXT("技能房");
	case ERoomType::Boss: return TEXT("BOSS房");
	default: return TEXT("未知");
	}
}

FString ARoomBase::GetRoomStateName(ERoomState State)
{
	switch (State)
	{
	case ERoomState::NotEntered: return TEXT("未进入");
	case ERoomState::InProgress: return TEXT("进行中");
	case ERoomState::Completed: return TEXT("已完成");
	case ERoomState::Skipped: return TEXT("已跳过");
	case ERoomState::Locked: return TEXT("已锁定");
	default: return TEXT("未知");
	}
}

FLinearColor ARoomBase::GetRoomTypeColor(ERoomType Type)
{
	switch (Type)
	{
	case ERoomType::Start: return FLinearColor::Green;
	case ERoomType::Combat: return FLinearColor::Red;
	case ERoomType::Event: return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // 橙色
	case ERoomType::Shop: return FLinearColor::Yellow;
	case ERoomType::Skill: return FLinearColor(0.5f, 0.0f, 1.0f, 1.0f); // 紫色
	case ERoomType::Boss: return FLinearColor::Black;
	default: return FLinearColor::White;
	}
}

void ARoomBase::LogRoom(const FString& Message)
{
	UE_LOG(LogRoomSystem, Log, TEXT("[%s] %s"), *GetName(), *Message);
}
