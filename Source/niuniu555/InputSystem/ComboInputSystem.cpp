// Copyright Epic Games, Inc. All Rights Reserved.

#include "ComboInputSystem.h"

void UComboInputSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentWorldTime = 0.0f;
	LastInputTime = 0.0f;
	CurrentInputSequence.Empty();
	InputTimestamps.Empty();
	RegisteredCombos.Empty();

	UE_LOG(LogTemp, Log, TEXT("ComboInputSystem: Initialized"));
}

void UComboInputSystem::Deinitialize()
{
	RegisteredCombos.Empty();
	CurrentInputSequence.Empty();
	InputTimestamps.Empty();
	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("ComboInputSystem: Deinitialized"));
}

void UComboInputSystem::RegisterCombo(const FComboSequence& ComboConfig)
{
	if (ComboConfig.ComboID.IsNone() || ComboConfig.InputSequence.Num() == 0)
	{
		return;
	}

	// 检查是否已存在，存在则更新
	for (int32 i = 0; i < RegisteredCombos.Num(); ++i)
	{
		if (RegisteredCombos[i].ComboID == ComboConfig.ComboID)
		{
			RegisteredCombos[i] = ComboConfig;
			UE_LOG(LogTemp, Log, TEXT("ComboInputSystem: Updated combo '%s'"), *ComboConfig.ComboID.ToString());
			return;
		}
	}

	// 添加新连招
	RegisteredCombos.Add(ComboConfig);
	UE_LOG(LogTemp, Log, TEXT("ComboInputSystem: Registered combo '%s' with sequence %s"),
		*ComboConfig.ComboID.ToString(), *SequenceToString(ComboConfig.InputSequence));
}

void UComboInputSystem::UnregisterCombo(FName ComboID)
{
	for (int32 i = RegisteredCombos.Num() - 1; i >= 0; --i)
	{
		if (RegisteredCombos[i].ComboID == ComboID)
		{
			RegisteredCombos.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("ComboInputSystem: Unregistered combo '%s'"), *ComboID.ToString());
			return;
		}
	}
}

void UComboInputSystem::ClearAllCombos()
{
	RegisteredCombos.Empty();
	UE_LOG(LogTemp, Log, TEXT("ComboInputSystem: All combos cleared"));
}

TArray<FComboSequence> UComboInputSystem::GetAllCombos() const
{
	return RegisteredCombos;
}

void UComboInputSystem::AddComboInput(EComboInputType InputType)
{
	if (InputType == EComboInputType::None)
	{
		return;
	}

	// 检查序列是否超时，如果超时就清空
	if (IsSequenceTimedOut() && CurrentInputSequence.Num() > 0)
	{
		ClearCurrentSequence();
	}

	// 限制序列长度
	if (CurrentInputSequence.Num() >= MaxSequenceLength)
	{
		// 移除最旧的输入
		CurrentInputSequence.RemoveAt(0);
		InputTimestamps.RemoveAt(0);
	}

	// 添加新输入
	CurrentInputSequence.Add(InputType);
	InputTimestamps.Add(CurrentWorldTime);
	LastInputTime = CurrentWorldTime;

	UE_LOG(LogTemp, Verbose, TEXT("ComboInputSystem: Added input %s, current sequence: %s"),
		*StaticEnum<EComboInputType>()->GetNameStringByValue((int64)InputType),
		*GetCurrentSequenceString());

	// 检测是否匹配连招
	FName MatchedCombo = CheckComboMatch();
	if (!MatchedCombo.IsNone())
	{
		UE_LOG(LogTemp, Log, TEXT("ComboInputSystem: Combo '%s' detected!"), *MatchedCombo.ToString());
		OnComboDetected.Broadcast(MatchedCombo);
		ClearCurrentSequence();
	}
}

FName UComboInputSystem::CheckComboMatch() const
{
	for (const FComboSequence& Combo : RegisteredCombos)
	{
		if (DoesSequenceMatch(Combo.InputSequence))
		{
			// 检查时间间隔
			bool bWithinInterval = true;
			if (InputTimestamps.Num() >= 2)
			{
				for (int32 i = 1; i < InputTimestamps.Num(); ++i)
				{
					if ((InputTimestamps[i] - InputTimestamps[i - 1]) > Combo.MaxInterval)
					{
						bWithinInterval = false;
						break;
					}
				}
			}

			if (bWithinInterval)
			{
				return Combo.ComboID;
			}
		}
	}

	return NAME_None;
}

bool UComboInputSystem::IsComboPrefix(FName ComboID) const
{
	for (const FComboSequence& Combo : RegisteredCombos)
	{
		if (Combo.ComboID == ComboID)
		{
			return IsSequencePrefix(Combo.InputSequence);
		}
	}
	return false;
}

FString UComboInputSystem::GetCurrentSequenceString() const
{
	return SequenceToString(CurrentInputSequence);
}

void UComboInputSystem::ClearCurrentSequence()
{
	CurrentInputSequence.Empty();
	InputTimestamps.Empty();
	UE_LOG(LogTemp, Verbose, TEXT("ComboInputSystem: Sequence cleared"));
}

void UComboInputSystem::Update(float DeltaTime)
{
	CurrentWorldTime += DeltaTime;

	// 检查序列超时
	if (IsSequenceTimedOut() && CurrentInputSequence.Num() > 0)
	{
		ClearCurrentSequence();
	}
}

bool UComboInputSystem::IsSequenceTimedOut() const
{
	if (CurrentInputSequence.Num() == 0)
	{
		return false;
	}

	return (CurrentWorldTime - LastInputTime) > SequenceTimeout;
}

bool UComboInputSystem::DoesSequenceMatch(const TArray<EComboInputType>& Pattern) const
{
	if (Pattern.Num() != CurrentInputSequence.Num())
	{
		return false;
	}

	for (int32 i = 0; i < Pattern.Num(); ++i)
	{
		if (Pattern[i] != CurrentInputSequence[i])
		{
			return false;
		}
	}

	return true;
}

bool UComboInputSystem::IsSequencePrefix(const TArray<EComboInputType>& Pattern) const
{
	if (CurrentInputSequence.Num() > Pattern.Num())
	{
		return false;
	}

	for (int32 i = 0; i < CurrentInputSequence.Num(); ++i)
	{
		if (Pattern[i] != CurrentInputSequence[i])
		{
			return false;
		}
	}

	return true;
}

FString UComboInputSystem::SequenceToString(const TArray<EComboInputType>& Sequence)
{
	FString Result;
	for (int32 i = 0; i < Sequence.Num(); ++i)
	{
		switch (Sequence[i])
		{
		case EComboInputType::Light:
			Result += TEXT("L");
			break;
		case EComboInputType::Heavy:
			Result += TEXT("H");
			break;
		default:
			Result += TEXT("?");
			break;
		}

		if (i < Sequence.Num() - 1)
		{
			Result += TEXT("-");
		}
	}
	return Result.IsEmpty() ? TEXT("(空)") : Result;
}
