// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class niuniu555 : ModuleRules
{
	public niuniu555(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"niuniu555",
			"niuniu555/SaveSystem",
			"niuniu555/InputSystem",
			"niuniu555/GameFlow",
			"niuniu555/ObjectPool",
			"niuniu555/AttributeSystem",
			"niuniu555/CharacterSystem",
			"niuniu555/WeaponSystem",
			"niuniu555/Variant_Platforming",
			"niuniu555/Variant_Platforming/Animation",
			"niuniu555/Variant_Combat",
			"niuniu555/Variant_Combat/AI",
			"niuniu555/Variant_Combat/Animation",
			"niuniu555/Variant_Combat/Gameplay",
			"niuniu555/Variant_Combat/Interfaces",
			"niuniu555/Variant_Combat/UI",
			"niuniu555/Variant_SideScrolling",
			"niuniu555/Variant_SideScrolling/AI",
			"niuniu555/Variant_SideScrolling/Gameplay",
			"niuniu555/Variant_SideScrolling/Interfaces",
			"niuniu555/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
