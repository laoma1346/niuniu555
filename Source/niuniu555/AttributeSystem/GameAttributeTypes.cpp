// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameAttributeTypes.h"

float FAttributeSet::GetAttributeValue(EAttributeType Type) const
{
	switch (Type)
	{
	case EAttributeType::Health:
		return Health.GetFinalValue();
	case EAttributeType::MaxHealth:
		return MaxHealth.GetFinalValue();
	case EAttributeType::Mana:
		return Mana.GetFinalValue();
	case EAttributeType::MaxMana:
		return MaxMana.GetFinalValue();
	case EAttributeType::Attack:
		return Attack.GetFinalValue();
	case EAttributeType::Defense:
		return Defense.GetFinalValue();
	case EAttributeType::MoveSpeed:
		return MoveSpeed.GetFinalValue();
	case EAttributeType::AttackSpeed:
		return AttackSpeed.GetFinalValue();
	case EAttributeType::CriticalRate:
		return CriticalRate.GetFinalValue();
	case EAttributeType::CriticalDamage:
		return CriticalDamage.GetFinalValue();
	case EAttributeType::GoldGainRate:
		return GoldGainRate.GetFinalValue();
	default:
		return 0.0f;
	}
}

FAttributeValue* FAttributeSet::GetAttribute(EAttributeType Type)
{
	switch (Type)
	{
	case EAttributeType::Health:
		return &Health;
	case EAttributeType::MaxHealth:
		return &MaxHealth;
	case EAttributeType::Mana:
		return &Mana;
	case EAttributeType::MaxMana:
		return &MaxMana;
	case EAttributeType::Attack:
		return &Attack;
	case EAttributeType::Defense:
		return &Defense;
	case EAttributeType::MoveSpeed:
		return &MoveSpeed;
	case EAttributeType::AttackSpeed:
		return &AttackSpeed;
	case EAttributeType::CriticalRate:
		return &CriticalRate;
	case EAttributeType::CriticalDamage:
		return &CriticalDamage;
	case EAttributeType::GoldGainRate:
		return &GoldGainRate;
	default:
		return nullptr;
	}
}
