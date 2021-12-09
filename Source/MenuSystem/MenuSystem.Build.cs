// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MenuSystem : ModuleRules
{
	public MenuSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "OnlineSubsystemSteam", "OnlineSubsystem" });
	}
}
