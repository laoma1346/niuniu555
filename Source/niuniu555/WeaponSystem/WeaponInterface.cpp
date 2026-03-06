// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponInterface.h"

// IWeaponInterface 默认实现
FDamageResult IWeaponInterface::CalculateDamage_Implementation(float BaseDamage, EDamageType InDamageType)
{
    FDamageResult Result;
    Result.DamageType = InDamageType;
    Result.FinalDamage = BaseDamage;
    Result.bHit = true;
    Result.bCritical = false;
    return Result;
}
