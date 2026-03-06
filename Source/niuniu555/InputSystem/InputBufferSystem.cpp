// Copyright Epic Games, Inc. All Rights Reserved.

#include "InputBufferSystem.h"

void UInputBufferSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentWorldTime = 0.0f;
	InputBuffer.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("InputBufferSystem: Initialized (BufferWindow: %.2fs)"), BufferWindowTime);
}

void UInputBufferSystem::Deinitialize()
{
	InputBuffer.Empty();
	Super::Deinitialize();
	
	UE_LOG(LogTemp, Log, TEXT("InputBufferSystem: Deinitialized"));
}

void UInputBufferSystem::BufferInput(EInputActionType ActionType)
{
	if (ActionType == EInputActionType::None)
	{
		return;
	}

	// 检查队列是否已满
	if (InputBuffer.Num() >= MaxBufferSize)
	{
		// 移除最旧的输入
		InputBuffer.RemoveAt(0);
	}

	// 添加新输入到队列
	FBufferedInput NewInput(ActionType, CurrentWorldTime);
	InputBuffer.Add(NewInput);

	UE_LOG(LogTemp, Verbose, TEXT("InputBufferSystem: Buffered input %s"), 
		*StaticEnum<EInputActionType>()->GetNameStringByValue((int64)ActionType));

	// 广播事件
	OnInputBuffered.Broadcast(ActionType);
}

EInputActionType UInputBufferSystem::ConsumeNextBufferedInput()
{
	// 清理过期输入
	CleanExpiredInputs();

	// 找到第一个未消费的输入
	for (int32 i = 0; i < InputBuffer.Num(); ++i)
	{
		if (!InputBuffer[i].bConsumed)
		{
			EInputActionType ActionType = InputBuffer[i].ActionType;
			InputBuffer[i].bConsumed = true;

			UE_LOG(LogTemp, Verbose, TEXT("InputBufferSystem: Consumed input %s"),
				*StaticEnum<EInputActionType>()->GetNameStringByValue((int64)ActionType));

			// 清理已消费的输入
			InputBuffer.RemoveAt(i);

			return ActionType;
		}
	}

	return EInputActionType::None;
}

EInputActionType UInputBufferSystem::PeekNextBufferedInput() const
{
	for (const FBufferedInput& Input : InputBuffer)
	{
		if (!Input.bConsumed && !IsInputExpired(Input))
		{
			return Input.ActionType;
		}
	}
	return EInputActionType::None;
}

bool UInputBufferSystem::HasBufferedInput() const
{
	for (const FBufferedInput& Input : InputBuffer)
	{
		if (!Input.bConsumed && !IsInputExpired(Input))
		{
			return true;
		}
	}
	return false;
}

void UInputBufferSystem::ClearBuffer()
{
	InputBuffer.Empty();
	UE_LOG(LogTemp, Verbose, TEXT("InputBufferSystem: Buffer cleared"));
}

void UInputBufferSystem::CleanExpiredInputs()
{
	for (int32 i = InputBuffer.Num() - 1; i >= 0; --i)
	{
		if (IsInputExpired(InputBuffer[i]) || InputBuffer[i].bConsumed)
		{
			InputBuffer.RemoveAt(i);
		}
	}
}

void UInputBufferSystem::Update(float DeltaTime)
{
	CurrentWorldTime += DeltaTime;

	// 定期清理过期输入
	CleanExpiredInputs();
}

bool UInputBufferSystem::IsInputExpired(const FBufferedInput& Input) const
{
	return (CurrentWorldTime - Input.BufferStartTime) > BufferWindowTime;
}
