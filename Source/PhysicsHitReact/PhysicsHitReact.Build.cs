// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using EpicGames.Core;

public class PhysicsHitReact : ModuleRules
{
	public PhysicsHitReact(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayTags",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"PhysicsCore",
			}
			);
		
		if (Target.bBuildEditor)
		{
			// FNotificationInfo & FSlateNotificationManager
			PrivateDependencyModuleNames.Add("Slate");
		}

		// Add pre-processor macros for the GameplayAbilities plugin based on enabled state (optional plugin)
		PublicDefinitions.Add("WITH_GAMEPLAY_ABILITIES=0");
		if (JsonObject.TryRead(Target.ProjectFile, out var rawObject))
		{
			if (rawObject.TryGetObjectArrayField("Plugins", out var pluginObjects))
			{
				foreach (JsonObject pluginObject in pluginObjects)
				{
					pluginObject.TryGetStringField("Name", out var pluginName);
		
					pluginObject.TryGetBoolField("Enabled", out var pluginEnabled);
		
					if (pluginName == "GameplayAbilities" && pluginEnabled)
					{
						PrivateDependencyModuleNames.Add("GameplayAbilities");
						PublicDefinitions.Add("WITH_GAMEPLAY_ABILITIES=1");
						PublicDefinitions.Remove("WITH_GAMEPLAY_ABILITIES=0");
					}
				}
			}
		}
	}
}
