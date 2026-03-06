// Copyright Epic Games, Inc. All Rights Reserved.

#include "ComboInputBuffer.h"

void UComboInputBuffer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    BufferWindowTime = 0.2f; // 默认200ms缓冲窗口
    MaxBufferSize = 3;       // 默认最多缓冲3个输入
    CurrentWorldTime = 0.0f;
}

void UComboInputBuffer::Deinitialize()
{
    WeaponInputBuffer.Empty();
    Super::Deinitialize();
}

// ========== 缓冲配置 ==========

void UComboInputBuffer::SetBufferWindowTime(float WindowTime)
{
    BufferWindowTime = FMath::Max(0.05f, WindowTime);
}

void UComboInputBuffer::SetMaxBufferSize(int32 MaxSize)
{
    MaxBufferSize = FMath::Max(1, MaxSize);
}

// ========== 输入处理 ==========

void UComboInputBuffer::BufferInput(EWeaponComboInputType InputType)
{
    if (InputType == EWeaponComboInputType::None)
    {
        return;
    }

    // 检查缓冲是否已满
    if (WeaponInputBuffer.Num() >= MaxBufferSize)
    {
        // 移除最旧的输入
        WeaponInputBuffer.RemoveAt(0);
    }

    AddInputToBuffer(InputType);

    UE_LOG(LogTemp, Verbose, TEXT("[ComboInputBuffer] 输入已缓冲: %s, 缓冲数量: %d"),
        *UEnum::GetValueAsString(InputType), WeaponInputBuffer.Num());
}

bool UComboInputBuffer::TryProcessBufferedInput(FOnBufferedInputProcessed ProcessCallback)
{
    if (WeaponInputBuffer.Num() == 0)
    {
        return false;
    }

    // 获取并移除最旧的输入
    FWeaponBufferedInput Input = WeaponInputBuffer[0];
    WeaponInputBuffer.RemoveAt(0);

    // 执行回调
    ProcessCallback.Execute(Input.WeaponInputType);

    UE_LOG(LogTemp, Verbose, TEXT("[ComboInputBuffer] 处理缓冲输入: %s"),
        *UEnum::GetValueAsString(Input.WeaponInputType));

    return true;
}

void UComboInputBuffer::ClearBuffer()
{
    WeaponInputBuffer.Empty();
    UE_LOG(LogTemp, Verbose, TEXT("[ComboInputBuffer] 缓冲已清空"));
}

// ========== 状态查询 ==========

bool UComboInputBuffer::HasBufferedInput() const
{
    return WeaponInputBuffer.Num() > 0;
}

EWeaponComboInputType UComboInputBuffer::PeekNextInput() const
{
    if (WeaponInputBuffer.Num() > 0)
    {
        return WeaponInputBuffer[0].WeaponInputType;
    }
    return EWeaponComboInputType::None;
}

int32 UComboInputBuffer::GetBufferCount() const
{
    return WeaponInputBuffer.Num();
}

// ========== Tick更新 ==========

void UComboInputBuffer::Update(float DeltaTime)
{
    CurrentWorldTime += DeltaTime;
    RemoveExpiredInputs();
}

// ========== 内部方法 ==========

void UComboInputBuffer::RemoveExpiredInputs()
{
    for (int32 i = WeaponInputBuffer.Num() - 1; i >= 0; --i)
    {
        if (CurrentWorldTime > WeaponInputBuffer[i].ExpireTime)
        {
            WeaponInputBuffer.RemoveAt(i);
            UE_LOG(LogTemp, Verbose, TEXT("[ComboInputBuffer] 过期输入已移除"));
        }
    }
}

void UComboInputBuffer::AddInputToBuffer(EWeaponComboInputType InputType)
{
    FWeaponBufferedInput NewInput;
    NewInput.WeaponInputType = InputType;
    NewInput.InputTime = CurrentWorldTime;
    NewInput.ExpireTime = CurrentWorldTime + BufferWindowTime;

    WeaponInputBuffer.Add(NewInput);
}
