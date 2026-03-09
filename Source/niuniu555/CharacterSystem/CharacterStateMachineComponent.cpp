#include "CharacterStateMachineComponent.h"

DEFINE_LOG_CATEGORY(LogCharacterStateMachine);

UCharacterStateMachineComponent::UCharacterStateMachineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	CurrentState = ECharacterState::None;
	PreviousState = ECharacterState::None;
}

void UCharacterStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();

	// 初始化状态为Idle
	ForceChangeState(ECharacterState::Idle);

	LogStateMachine(TEXT("状态机初始化完成，初始状态: Idle"));
}

void UCharacterStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新状态持续时间
	CurrentStateDuration = GetWorld()->GetTimeSeconds() - StateStartTime;

	// 更新调试显示
	if (bShowDebugInfo)
	{
		UpdateDebugDisplay();
	}
}

bool UCharacterStateMachineComponent::IsCombatReady() const
{
	return CurrentState == ECharacterState::Idle || 
		   CurrentState == ECharacterState::Move || 
		   CurrentState == ECharacterState::Attack;
}

bool UCharacterStateMachineComponent::IsInHitStun() const
{
	return CurrentState == ECharacterState::Hit || 
		   CurrentState == ECharacterState::Stun;
}

bool UCharacterStateMachineComponent::IsInvincible() const
{
	// 闪避和受击/眩晕时通常是无敌的
	return CurrentState == ECharacterState::Dodge || 
		   CurrentState == ECharacterState::Hit || 
		   CurrentState == ECharacterState::Stun;
}

bool UCharacterStateMachineComponent::CanAttack() const
{
	// 可以在Idle、Move、Attack状态攻击（连招）
	return CurrentState == ECharacterState::Idle || 
		   CurrentState == ECharacterState::Move || 
		   CurrentState == ECharacterState::Attack;
}

bool UCharacterStateMachineComponent::CanDodge() const
{
	// 可以在Idle、Move、Attack状态闪避
	// 不能在受击、眩晕、死亡、闪避中再次闪避
	return CurrentState == ECharacterState::Idle || 
		   CurrentState == ECharacterState::Move || 
		   CurrentState == ECharacterState::Attack ||
		   CurrentState == ECharacterState::Skill;
}

bool UCharacterStateMachineComponent::CanMove() const
{
	// 可以在Idle、Move状态移动
	// Attack、Skill状态可能需要特殊处理（允许缓慢移动）
	return CurrentState == ECharacterState::Idle || 
		   CurrentState == ECharacterState::Move ||
		   CurrentState == ECharacterState::Attack; // 攻击中允许缓慢移动
}

bool UCharacterStateMachineComponent::CanUseSkill() const
{
	return CurrentState == ECharacterState::Idle || 
		   CurrentState == ECharacterState::Move || 
		   CurrentState == ECharacterState::Attack;
}

bool UCharacterStateMachineComponent::CanInteract() const
{
	// 只能在Idle或Move状态交互
	return CurrentState == ECharacterState::Idle || 
		   CurrentState == ECharacterState::Move;
}

bool UCharacterStateMachineComponent::CanSwitchWeapon() const
{
	// 可以在Idle、Move状态切换
	// Attack状态也可以（触发切换取消）
	return CurrentState == ECharacterState::Idle || 
		   CurrentState == ECharacterState::Move || 
		   CurrentState == ECharacterState::Attack;
}

bool UCharacterStateMachineComponent::CanPerformAction(ECharacterState RequiredState) const
{
	return CurrentState == RequiredState;
}

bool UCharacterStateMachineComponent::TryChangeState(ECharacterState NewState, bool bForce)
{
	if (NewState == CurrentState)
	{
		return true; // 已经是这个状态，视为成功
	}

	// 检查状态转换是否合法
	if (!bForce && !CanTransitionTo(NewState))
	{
		LogStateMachine(FString::Printf(TEXT("状态切换被拒绝: %s -> %s"), 
			*GetStateName(CurrentState), *GetStateName(NewState)));
		return false;
	}

	// 执行状态切换
	ForceChangeState(NewState);
	return true;
}

void UCharacterStateMachineComponent::ForceChangeState(ECharacterState NewState)
{
	if (NewState == CurrentState)
	{
		return;
	}

	ECharacterState OldState = CurrentState;

	LogStateMachine(FString::Printf(TEXT("状态切换: %s -> %s"), 
		*GetStateName(OldState), *GetStateName(NewState)));

	// 退出旧状态
	OnExitState(OldState);

	// 更新状态
	PreviousState = OldState;
	CurrentState = NewState;
	StateStartTime = GetWorld()->GetTimeSeconds();
	CurrentStateDuration = 0.0f;

	// 更新计数
	StateEnterCount.FindOrAdd(NewState)++;

	// 进入新状态
	OnEnterState(NewState);

	// 广播事件
	OnStateExit.Broadcast(OldState);
	OnStateEnter.Broadcast(NewState);
	OnStateChanged.Broadcast(OldState, NewState);
}

void UCharacterStateMachineComponent::RevertToPreviousState()
{
	if (PreviousState != ECharacterState::None)
	{
		TryChangeState(PreviousState);
	}
}

void UCharacterStateMachineComponent::ResetToIdle()
{
	ForceChangeState(ECharacterState::Idle);
}

bool UCharacterStateMachineComponent::CanTransitionTo(ECharacterState NewState) const
{
	// 死亡状态通常不能直接切换到其他状态（需要复活机制）
	if (CurrentState == ECharacterState::Dead && NewState != ECharacterState::Dead)
	{
		return false;
	}

	// 定义状态转换规则
	switch (CurrentState)
	{
	case ECharacterState::None:
		return true; // None可以转换到任何状态

	case ECharacterState::Idle:
		return true; // Idle可以转换到任何状态

	case ECharacterState::Move:
		return true; // Move可以转换到任何状态

	case ECharacterState::Attack:
		// 攻击中不能交互，其他都可以（通常有动画事件触发状态切换）
		return NewState != ECharacterState::Interact;

	case ECharacterState::Dodge:
		// 闪避中只能进入Idle或Hit（被打断）
		return NewState == ECharacterState::Idle || 
			   NewState == ECharacterState::Move || 
			   NewState == ECharacterState::Hit ||
			   NewState == ECharacterState::Stun;

	case ECharacterState::Hit:
		// 受击后可以进入Idle、Stun、Dead
		return NewState == ECharacterState::Idle || 
			   NewState == ECharacterState::Move || 
			   NewState == ECharacterState::Stun || 
			   NewState == ECharacterState::Dead;

	case ECharacterState::Stun:
		// 眩晕结束后可以进入Idle或Dead
		return NewState == ECharacterState::Idle || 
			   NewState == ECharacterState::Move || 
			   NewState == ECharacterState::Dead;

	case ECharacterState::Skill:
		// 技能中可以被打断（Hit/Stun/Dead），或正常结束（Idle/Move）
		return NewState == ECharacterState::Idle || 
			   NewState == ECharacterState::Move || 
			   NewState == ECharacterState::Hit || 
			   NewState == ECharacterState::Stun || 
			   NewState == ECharacterState::Dead;

	case ECharacterState::Interact:
		// 交互中只能进入Idle（取消交互）或Hit（被打断）
		return NewState == ECharacterState::Idle || 
			   NewState == ECharacterState::Move || 
			   NewState == ECharacterState::Hit || 
			   NewState == ECharacterState::Stun;

	case ECharacterState::Dead:
		return false; // 死亡状态不能直接切换

	default:
		return true;
	}
}

void UCharacterStateMachineComponent::OnEnterState(ECharacterState NewState)
{
	// 根据状态执行特定逻辑
	switch (NewState)
	{
	case ECharacterState::Idle:
		// Idle状态可以做一些恢复逻辑
		break;

	case ECharacterState::Attack:
		// 进入攻击状态时可以重置某些标志
		break;

	case ECharacterState::Dodge:
		// 进入闪避状态时触发闪避逻辑
		break;

	case ECharacterState::Hit:
		// 进入受击状态
		break;

	case ECharacterState::Dead:
		// 死亡状态
		break;

	default:
		break;
	}
}

void UCharacterStateMachineComponent::OnExitState(ECharacterState OldState)
{
	// 根据状态执行退出逻辑
	switch (OldState)
	{
	case ECharacterState::Attack:
		// 退出攻击状态，可以清除连招计数等
		break;

	case ECharacterState::Dodge:
		// 退出闪避状态
		break;

	default:
		break;
	}
}

void UCharacterStateMachineComponent::UpdateDebugDisplay()
{
	if (AActor* Owner = GetOwner())
	{
		FString DebugText = FString::Printf(TEXT("State: %s\nDuration: %.2f"),
			*GetStateName(CurrentState), CurrentStateDuration);
		
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, DebugText);
	}
}

void UCharacterStateMachineComponent::LogStateMachine(const FString& Message)
{
	if (bEnableDebugLog)
	{
		UE_LOG(LogCharacterStateMachine, Log, TEXT("[%s] %s"), 
			*GetOwner()->GetName(), *Message);
	}
}

FString UCharacterStateMachineComponent::GetStateName(ECharacterState State)
{
	switch (State)
	{
	case ECharacterState::None: return TEXT("None");
	case ECharacterState::Idle: return TEXT("Idle");
	case ECharacterState::Move: return TEXT("Move");
	case ECharacterState::Attack: return TEXT("Attack");
	case ECharacterState::Dodge: return TEXT("Dodge");
	case ECharacterState::Hit: return TEXT("Hit");
	case ECharacterState::Stun: return TEXT("Stun");
	case ECharacterState::Dead: return TEXT("Dead");
	case ECharacterState::Interact: return TEXT("Interact");
	case ECharacterState::Skill: return TEXT("Skill");
	default: return TEXT("Unknown");
	}
}
