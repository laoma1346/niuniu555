// Copyright Epic Games, Inc. All Rights Reserved.

#include "ComboTypes.h"

FString FWeaponComboSequence::GetSequenceString() const
{
    FString Result;
    for (int32 i = 0; i < InputSequence.Num(); ++i)
    {
        switch (InputSequence[i])
        {
        case EWeaponComboInputType::Light:
            Result += TEXT("L");
            break;
        case EWeaponComboInputType::Heavy:
            Result += TEXT("H");
            break;
        default:
            Result += TEXT("?");
            break;
        }
        
        if (i < InputSequence.Num() - 1)
        {
            Result += TEXT("-");
        }
    }
    return Result;
}

bool FWeaponComboSequence::IsValidConfig() const
{
    // 检查是否有输入序列
    if (InputSequence.Num() == 0)
    {
        return false;
    }
    
    // 检查输入序列是否包含无效输入
    for (const EWeaponComboInputType& Input : InputSequence)
    {
        if (Input == EWeaponComboInputType::None)
        {
            return false;
        }
    }
    
    // 检查阶段配置数量是否匹配
    if (StageConfigs.Num() != InputSequence.Num())
    {
        return false;
    }
    
    // 检查每个阶段配置的索引是否正确
    for (int32 i = 0; i < StageConfigs.Num(); ++i)
    {
        if (StageConfigs[i].StageIndex != i)
        {
            return false;
        }
    }
    
    return true;
}
