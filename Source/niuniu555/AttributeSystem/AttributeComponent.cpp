// Copyright Epic Games, Inc. All Rights Reserved.

#include "AttributeComponent.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Attributes.InitializeDefaults();
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

float UAttributeComponent::GetAttributeValue(EAttributeType Type) const
{
	return Attributes.GetAttributeValue(Type);
}

float UAttributeComponent::GetBaseValue(EAttributeType Type) const
{
	if (const FAttributeValue* Attr = const_cast<FAttributeSet*>(&Attributes)->GetAttribute(Type))
	{
		return Attr->BaseValue;
	}
	return 0.0f;
}

void UAttributeComponent::SetBaseValue(EAttributeType Type, float Value)
{
	if (FAttributeValue* Attr = Attributes.GetAttribute(Type))
	{
		Attr->BaseValue = Value;
		OnAttributeChanged.Broadcast(Type, Attr->GetFinalValue());

		// 特殊处理生命值
		if (Type == EAttributeType::Health)
		{
			OnHealthChanged.Broadcast(GetHealth(), GetMaxHealth(), 0.0f);
		}
	}
}

float UAttributeComponent::GetHealthPercent() const
{
	float MaxHealth = GetMaxHealth();
	if (MaxHealth > 0.0f)
	{
		return GetHealth() / MaxHealth;
	}
	return 0.0f;
}

void UAttributeComponent::ModifyHealth(float Delta)
{
	if (Delta == 0.0f || IsDead())
	{
		return;
	}

	ApplyHealthChange(Delta);
}

void UAttributeComponent::FullHeal()
{
	if (IsAlive())
	{
		float HealAmount = GetMaxHealth() - GetHealth();
		ApplyHealthChange(HealAmount);
	}
}

void UAttributeComponent::ModifyMana(float Delta)
{
	if (FAttributeValue* Attr = Attributes.GetAttribute(EAttributeType::Mana))
	{
		float OldValue = Attr->GetFinalValue();
		Attr->BaseValue = FMath::Clamp(Attr->BaseValue + Delta, 0.0f, GetMaxMana());
		OnAttributeChanged.Broadcast(EAttributeType::Mana, Attr->GetFinalValue());
	}
}

void UAttributeComponent::AddAdditiveValue(EAttributeType Type, float Value)
{
	if (FAttributeValue* Attr = Attributes.GetAttribute(Type))
	{
		Attr->AdditiveValue += Value;
		OnAttributeChanged.Broadcast(Type, Attr->GetFinalValue());
	}
}

void UAttributeComponent::AddMultiplier(EAttributeType Type, float Value)
{
	if (FAttributeValue* Attr = Attributes.GetAttribute(Type))
	{
		Attr->Multiplier += Value;
		OnAttributeChanged.Broadcast(Type, Attr->GetFinalValue());
	}
}

void UAttributeComponent::RemoveAdditiveValue(EAttributeType Type, float Value)
{
	if (FAttributeValue* Attr = Attributes.GetAttribute(Type))
	{
		Attr->AdditiveValue -= Value;
		OnAttributeChanged.Broadcast(Type, Attr->GetFinalValue());
	}
}

void UAttributeComponent::RemoveMultiplier(EAttributeType Type, float Value)
{
	if (FAttributeValue* Attr = Attributes.GetAttribute(Type))
	{
		Attr->Multiplier -= Value;
		OnAttributeChanged.Broadcast(Type, Attr->GetFinalValue());
	}
}

void UAttributeComponent::ResetAllModifiers()
{
	Attributes.Health.ResetModifiers();
	Attributes.MaxHealth.ResetModifiers();
	Attributes.Mana.ResetModifiers();
	Attributes.MaxMana.ResetModifiers();
	Attributes.Attack.ResetModifiers();
	Attributes.Defense.ResetModifiers();
	Attributes.MoveSpeed.ResetModifiers();
	Attributes.AttackSpeed.ResetModifiers();
	Attributes.CriticalRate.ResetModifiers();
	Attributes.CriticalDamage.ResetModifiers();
	Attributes.GoldGainRate.ResetModifiers();
}

void UAttributeComponent::ApplyHealthChange(float Delta)
{
	if (FAttributeValue* Attr = Attributes.GetAttribute(EAttributeType::Health))
	{
		float OldHealth = Attr->GetFinalValue();
		float NewHealth = FMath::Clamp(OldHealth + Delta, 0.0f, GetMaxHealth());
		
		Attr->BaseValue = NewHealth;
		
		OnHealthChanged.Broadcast(NewHealth, GetMaxHealth(), Delta);
		OnAttributeChanged.Broadcast(EAttributeType::Health, NewHealth);

		// 检查死亡
		if (NewHealth <= 0.0f && OldHealth > 0.0f)
		{
			HandleDeath();
		}
	}
}

void UAttributeComponent::HandleDeath()
{
	OnDeath.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("AttributeComponent: %s died"), *GetOwner()->GetName());
}
