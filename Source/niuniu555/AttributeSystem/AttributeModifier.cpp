// Copyright Epic Games, Inc. All Rights Reserved.

#include "AttributeModifier.h"
#include "AttributeComponent.h"

UAttributeModifierComponent::UAttributeModifierComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAttributeModifierComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		TargetAttributeComp = Owner->FindComponentByClass<UAttributeComponent>();
	}
}

void UAttributeModifierComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateModifiers(DeltaTime);
}

void UAttributeModifierComponent::AddModifier(const FAttributeModifier& Modifier)
{
	if (!TargetAttributeComp) return;

	RemoveModifier(Modifier.ModifierID);

	FAttributeModifier NewModifier = Modifier;
	NewModifier.Initialize();
	Modifiers.Add(NewModifier);
	ApplyModifier(NewModifier);
}

void UAttributeModifierComponent::AddAttackBuff(float Value, float Duration, FName BuffID)
{
	FAttributeModifier Modifier;
	Modifier.ModifierID = BuffID;
	Modifier.TargetAttribute = EAttributeType::Attack;
	Modifier.ModifierType = EModifierType::Additive;
	Modifier.Value = Value;
	Modifier.Duration = Duration;
	AddModifier(Modifier);
}

void UAttributeModifierComponent::AddDefenseBuff(float Value, float Duration, FName BuffID)
{
	FAttributeModifier Modifier;
	Modifier.ModifierID = BuffID;
	Modifier.TargetAttribute = EAttributeType::Defense;
	Modifier.ModifierType = EModifierType::Additive;
	Modifier.Value = Value;
	Modifier.Duration = Duration;
	AddModifier(Modifier);
}

void UAttributeModifierComponent::AddSpeedBuff(float Value, float Duration, FName BuffID)
{
	FAttributeModifier Modifier;
	Modifier.ModifierID = BuffID;
	Modifier.TargetAttribute = EAttributeType::MoveSpeed;
	Modifier.ModifierType = EModifierType::Multiplier;
	Modifier.Value = Value;
	Modifier.Duration = Duration;
	AddModifier(Modifier);
}

void UAttributeModifierComponent::RemoveModifier(FName ModifierID)
{
	for (int32 i = Modifiers.Num() - 1; i >= 0; --i)
	{
		if (Modifiers[i].ModifierID == ModifierID)
		{
			UnapplyModifier(Modifiers[i]);
			Modifiers.RemoveAt(i);
			return;
		}
	}
}

void UAttributeModifierComponent::RemoveAllModifiers()
{
	for (const FAttributeModifier& Modifier : Modifiers)
	{
		UnapplyModifier(Modifier);
	}
	Modifiers.Empty();
}

void UAttributeModifierComponent::RemoveModifiersByAttribute(EAttributeType AttributeType)
{
	for (int32 i = Modifiers.Num() - 1; i >= 0; --i)
	{
		if (Modifiers[i].TargetAttribute == AttributeType)
		{
			UnapplyModifier(Modifiers[i]);
			Modifiers.RemoveAt(i);
		}
	}
}

int32 UAttributeModifierComponent::GetActiveModifierCount() const
{
	int32 Count = 0;
	for (const FAttributeModifier& Modifier : Modifiers)
	{
		if (Modifier.bIsActive) Count++;
	}
	return Count;
}

bool UAttributeModifierComponent::HasModifier(FName ModifierID) const
{
	for (const FAttributeModifier& Modifier : Modifiers)
	{
		if (Modifier.ModifierID == ModifierID && Modifier.bIsActive)
		{
			return true;
		}
	}
	return false;
}

void UAttributeModifierComponent::ApplyModifier(const FAttributeModifier& Modifier)
{
	if (!TargetAttributeComp) return;

	switch (Modifier.ModifierType)
	{
	case EModifierType::Additive:
		TargetAttributeComp->AddAdditiveValue(Modifier.TargetAttribute, Modifier.Value);
		break;
	case EModifierType::Multiplier:
		TargetAttributeComp->AddMultiplier(Modifier.TargetAttribute, Modifier.Value);
		break;
	default:
		break;
	}
}

void UAttributeModifierComponent::UnapplyModifier(const FAttributeModifier& Modifier)
{
	if (!TargetAttributeComp) return;

	switch (Modifier.ModifierType)
	{
	case EModifierType::Additive:
		TargetAttributeComp->RemoveAdditiveValue(Modifier.TargetAttribute, Modifier.Value);
		break;
	case EModifierType::Multiplier:
		TargetAttributeComp->RemoveMultiplier(Modifier.TargetAttribute, Modifier.Value);
		break;
	default:
		break;
	}
}

void UAttributeModifierComponent::UpdateModifiers(float DeltaTime)
{
	for (int32 i = Modifiers.Num() - 1; i >= 0; --i)
	{
		if (!Modifiers[i].Update(DeltaTime))
		{
			UnapplyModifier(Modifiers[i]);
			Modifiers.RemoveAt(i);
		}
	}
}
