// Copyright Epic Games, Inc. All Rights Reserved.

#include "ComboDetectorComponent.h"

UComboDetectorComponent::UComboDetectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(false);

    DefaultInputToleranceTime = 0.5f;
    DefaultMaxComboDuration = 3.0f;
    bAutoDetectBestMatch = true;
    CooldownTimer = 0.0f;
    CurrentComboIndex = -1;
}

void UComboDetectorComponent::BeginPlay()
{
    Super::BeginPlay();
    RuntimeState.Reset();
}

void UComboDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 更新冷却计时器
    if (CooldownTimer > 0.0f)
    {
        CooldownTimer -= DeltaTime;
    }

    // 检查连招超时
    if (RuntimeState.bIsInCombo && IsComboTimedOut())
    {
        BreakCombo(TEXT("连招超时"));
    }
}

void UComboDetectorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

// ========== 连招注册 ==========

void UComboDetectorComponent::RegisterCombo(const FWeaponComboSequence& ComboConfig)
{
    if (!ComboConfig.IsValidConfig())
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 注册连招失败 - 配置无效: %s"), 
            *GetName(), *ComboConfig.ComboID.ToString());
        return;
    }

    // 检查是否已存在相同ID的连招
    for (int32 i = 0; i < RegisteredCombos.Num(); ++i)
    {
        if (RegisteredCombos[i].ComboID == ComboConfig.ComboID)
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] 连招ID已存在，替换: %s"), 
                *GetName(), *ComboConfig.ComboID.ToString());
            RegisteredCombos[i] = ComboConfig;
            return;
        }
    }

    RegisteredCombos.Add(ComboConfig);
    UE_LOG(LogTemp, Log, TEXT("[%s] 注册连招成功: %s (%s)"), 
        *GetName(), *ComboConfig.ComboID.ToString(), *ComboConfig.GetSequenceString());
}

void UComboDetectorComponent::UnregisterCombo(FName ComboID)
{
    for (int32 i = RegisteredCombos.Num() - 1; i >= 0; --i)
    {
        if (RegisteredCombos[i].ComboID == ComboID)
        {
            RegisteredCombos.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("[%s] 注销连招: %s"), *GetName(), *ComboID.ToString());
            break;
        }
    }
}

void UComboDetectorComponent::ClearAllCombos()
{
    RegisteredCombos.Empty();
    ResetComboState();
    UE_LOG(LogTemp, Log, TEXT("[%s] 清空所有连招"), *GetName());
}

// ========== 输入处理 ==========

void UComboDetectorComponent::AddComboInput(EWeaponComboInputType InputType)
{
    if (InputType == EWeaponComboInputType::None)
    {
        return;
    }

    // 检查冷却
    if (CooldownTimer > 0.0f)
    {
        UE_LOG(LogTemp, Verbose, TEXT("[%s] 输入被忽略 - 冷却中"), *GetName());
        return;
    }

    // 广播输入事件
    OnComboInputReceived.Broadcast(InputType);

    float CurrentTime = GetCurrentWorldTime();

    // 如果不在连招中，尝试开始新连招
    if (!RuntimeState.bIsInCombo)
    {
        RuntimeState.CurrentInputSequence.Add(InputType);
        RuntimeState.LastInputTime = CurrentTime;
        RuntimeState.ComboStartTime = CurrentTime;
        RuntimeState.bIsInCombo = true;

        // 查找匹配的连招
        int32 MatchingIndex = FindMatchingComboIndex();
        if (MatchingIndex != -1)
        {
            FWeaponComboSequence& MatchingCombo = RegisteredCombos[MatchingIndex];
            RuntimeState.CurrentComboID = MatchingCombo.ComboID;
            CurrentComboIndex = MatchingIndex;
            RuntimeState.CurrentStageIndex = 0;
            RuntimeState.bCurrentStageCompleted = false;

            UE_LOG(LogTemp, Log, TEXT("[%s] 开始连招: %s, 阶段: %d"), 
                *GetName(), *MatchingCombo.ComboID.ToString(), 0);

            OnComboStageAdvanced.Broadcast(MatchingCombo.ComboID, 0);
        }
        else
        {
            // 没有匹配的连招，中断
            BreakCombo(TEXT("无匹配连招"));
        }
    }
    else
    {
        // 检查是否匹配当前连招的下一阶段
        FWeaponComboStageConfig CurrentStageConfig = GetCurrentStageConfig();
        
        // 检查输入时间是否在容忍时间内
        float TimeSinceLastInput = CurrentTime - RuntimeState.LastInputTime;
        if (TimeSinceLastInput > CurrentStageConfig.InputToleranceTime)
        {
            BreakCombo(TEXT("输入间隔过长"));
            return;
        }

        // 添加输入到序列
        RuntimeState.CurrentInputSequence.Add(InputType);
        RuntimeState.LastInputTime = CurrentTime;

        // 检查是否匹配当前连招的下一阶段
        int32 NextStageIndex = RuntimeState.CurrentStageIndex + 1;
        if (RegisteredCombos.IsValidIndex(CurrentComboIndex) && 
            NextStageIndex < RegisteredCombos[CurrentComboIndex].StageConfigs.Num())
        {
            EWeaponComboInputType ExpectedInput = RegisteredCombos[CurrentComboIndex].InputSequence[NextStageIndex];
            if (InputType == ExpectedInput)
            {
                // 匹配成功，推进阶段
                AdvanceToNextStage();
            }
            else
            {
                // 输入不匹配，检查是否有其他连招匹配
                if (bAutoDetectBestMatch)
                {
                    int32 NewMatchIndex = FindMatchingComboIndex();
                    if (NewMatchIndex != -1 && NewMatchIndex != CurrentComboIndex)
                    {
                        // 切换到新的连招
                        FWeaponComboSequence& NewMatch = RegisteredCombos[NewMatchIndex];
                        RuntimeState.CurrentComboID = NewMatch.ComboID;
                        CurrentComboIndex = NewMatchIndex;
                        RuntimeState.CurrentStageIndex = 0;
                        OnComboStageAdvanced.Broadcast(NewMatch.ComboID, 0);
                    }
                    else
                    {
                        BreakCombo(TEXT("输入不匹配"));
                    }
                }
                else
                {
                    BreakCombo(TEXT("输入不匹配"));
                }
            }
        }
        else
        {
            // 连招已完成
            CompleteCombo();
        }
    }
}

void UComboDetectorComponent::BreakCombo(const FString& Reason)
{
    if (!RuntimeState.bIsInCombo)
    {
        return;
    }

    FName BrokenComboID = RuntimeState.CurrentComboID;
    
    UE_LOG(LogTemp, Log, TEXT("[%s] 连招中断: %s, 原因: %s"), 
        *GetName(), *BrokenComboID.ToString(), *Reason);

    OnComboBroken.Broadcast(BrokenComboID, Reason);

    RuntimeState.Reset();
    CurrentComboIndex = -1;
}

void UComboDetectorComponent::ResetComboState()
{
    RuntimeState.Reset();
    CurrentComboIndex = -1;
    CooldownTimer = 0.0f;
}

// ========== 状态查询 ==========

FString UComboDetectorComponent::GetCurrentInputSequenceString() const
{
    FString Result;
    for (int32 i = 0; i < RuntimeState.CurrentInputSequence.Num(); ++i)
    {
        switch (RuntimeState.CurrentInputSequence[i])
        {
        case EWeaponComboInputType::Light:
            Result += TEXT("L");
            break;
        case EWeaponComboInputType::Heavy:
            Result += TEXT("H");
            break;
        default:
            break;
        }
        if (i < RuntimeState.CurrentInputSequence.Num() - 1)
        {
            Result += TEXT("-");
        }
    }
    return Result;
}

FWeaponComboStageConfig UComboDetectorComponent::GetCurrentStageConfig() const
{
    if (!RuntimeState.bIsInCombo || !RegisteredCombos.IsValidIndex(CurrentComboIndex))
    {
        return FWeaponComboStageConfig();
    }

    const FWeaponComboSequence& CurrentCombo = RegisteredCombos[CurrentComboIndex];
    if (CurrentCombo.StageConfigs.Num() == 0)
    {
        return FWeaponComboStageConfig();
    }

    int32 Index = FMath::Clamp(RuntimeState.CurrentStageIndex, 0, CurrentCombo.StageConfigs.Num() - 1);
    return CurrentCombo.StageConfigs[Index];
}

bool UComboDetectorComponent::DoesInputMatchNextStage(EWeaponComboInputType InputType) const
{
    if (!RuntimeState.bIsInCombo || !RegisteredCombos.IsValidIndex(CurrentComboIndex))
    {
        return false;
    }

    const FWeaponComboSequence& CurrentCombo = RegisteredCombos[CurrentComboIndex];
    int32 NextStageIndex = RuntimeState.CurrentStageIndex + 1;
    if (NextStageIndex >= CurrentCombo.InputSequence.Num())
    {
        return false;
    }

    return CurrentCombo.InputSequence[NextStageIndex] == InputType;
}

// ========== 内部方法 ==========

int32 UComboDetectorComponent::FindMatchingComboIndex()
{
    for (int32 i = 0; i < RegisteredCombos.Num(); ++i)
    {
        if (IsSequencePrefix(RegisteredCombos[i].InputSequence, RuntimeState.CurrentInputSequence))
        {
            return i;
        }
    }
    return -1;
}

bool UComboDetectorComponent::IsSequencePrefix(const TArray<EWeaponComboInputType>& Sequence, const TArray<EWeaponComboInputType>& Prefix) const
{
    if (Prefix.Num() > Sequence.Num())
    {
        return false;
    }

    for (int32 i = 0; i < Prefix.Num(); ++i)
    {
        if (Sequence[i] != Prefix[i])
        {
            return false;
        }
    }
    return true;
}

void UComboDetectorComponent::AdvanceToNextStage()
{
    if (!RuntimeState.bIsInCombo)
    {
        return;
    }

    RuntimeState.CurrentStageIndex++;
    RuntimeState.bCurrentStageCompleted = false;

    UE_LOG(LogTemp, Log, TEXT("[%s] 连招阶段推进: %s -> %d"), 
        *GetName(), *RuntimeState.CurrentComboID.ToString(), RuntimeState.CurrentStageIndex);

    OnComboStageAdvanced.Broadcast(RuntimeState.CurrentComboID, RuntimeState.CurrentStageIndex);

    // 检查是否完成所有阶段
    if (RegisteredCombos.IsValidIndex(CurrentComboIndex))
    {
        const FWeaponComboSequence& CurrentCombo = RegisteredCombos[CurrentComboIndex];
        if (RuntimeState.CurrentStageIndex >= CurrentCombo.StageConfigs.Num() - 1)
        {
            CompleteCombo();
        }
    }
}

void UComboDetectorComponent::CompleteCombo()
{
    if (!RuntimeState.bIsInCombo)
    {
        return;
    }

    FName CompletedComboID = RuntimeState.CurrentComboID;
    int32 CompletedStages = RuntimeState.CurrentStageIndex + 1;

    UE_LOG(LogTemp, Log, TEXT("[%s] 连招完成: %s, 完成阶段: %d"), 
        *GetName(), *CompletedComboID.ToString(), CompletedStages);

    OnComboDetected.Broadcast(CompletedComboID, CompletedStages);

    // 设置冷却
    if (RegisteredCombos.IsValidIndex(CurrentComboIndex))
    {
        CooldownTimer = RegisteredCombos[CurrentComboIndex].CooldownAfterComplete;
    }

    // 根据配置决定是否重置
    bool bShouldReset = true;
    if (RegisteredCombos.IsValidIndex(CurrentComboIndex))
    {
        bShouldReset = RegisteredCombos[CurrentComboIndex].bResetAfterComplete;
    }
    
    if (bShouldReset)
    {
        RuntimeState.Reset();
        CurrentComboIndex = -1;
    }
    else
    {
        // 保持最后一击状态，允许继续派生
        RuntimeState.bCurrentStageCompleted = true;
    }
}

bool UComboDetectorComponent::IsComboTimedOut() const
{
    if (!RuntimeState.bIsInCombo)
    {
        return false;
    }

    float CurrentTime = GetCurrentWorldTime();
    
    // 检查总时长
    if (RegisteredCombos.IsValidIndex(CurrentComboIndex))
    {
        float ElapsedTime = CurrentTime - RuntimeState.ComboStartTime;
        if (ElapsedTime > RegisteredCombos[CurrentComboIndex].MaxTotalDuration)
        {
            return true;
        }
    }

    // 检查最后输入时间
    float TimeSinceLastInput = CurrentTime - RuntimeState.LastInputTime;
    FWeaponComboStageConfig StageConfig = GetCurrentStageConfig();
    if (TimeSinceLastInput > StageConfig.InputToleranceTime)
    {
        return true;
    }

    return false;
}

bool UComboDetectorComponent::IsCurrentStageCancellable() const
{
    if (!RuntimeState.bIsInCombo)
    {
        return false;
    }

    FWeaponComboStageConfig StageConfig = GetCurrentStageConfig();
    if (!StageConfig.bCanCancel)
    {
        return false;
    }

    float CurrentTime = GetCurrentWorldTime();
    float StageElapsedTime = CurrentTime - RuntimeState.LastInputTime;

    return StageElapsedTime >= StageConfig.CancelWindowStart && 
           StageElapsedTime <= StageConfig.CancelWindowEnd;
}

float UComboDetectorComponent::GetCurrentWorldTime() const
{
    return GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}
