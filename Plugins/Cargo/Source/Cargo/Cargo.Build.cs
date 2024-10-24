// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Cargo : ModuleRules
{
	public Cargo(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrecompileForTargets = PrecompileTargetsType.Any;
        bUsePrecompiled = true;

        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
            }
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				Path.Combine(ModuleDirectory, "Private", "cargo-io", "third_party", "readerwriterqueue"),
				Path.Combine(ModuleDirectory, "Private", "cargo-io", "third_party", "nlohmann-json", "single_include"),
				Path.Combine(ModuleDirectory, "Private", "cargo-io", "third_party")
            }
			);

		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Projects",
				"Engine",
				"CoreUObject",
				"StaticMeshDescription",
				"MeshDescription",
				"UnrealEd",
				"Kismet",
				"EditorSubsystem",
				"ImageWrapper",
				"Slate",
				"SlateCore",
				"ToolMenus",
				"AssetTools",
				"InputCore"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
			
		PublicDefinitions.AddRange(
			new string[]
			{
				"_WIN32_WINNT_WIN10_TH2",
				"_WIN32_WINNT_WIN10_RS1",
				"_WIN32_WINNT_WIN10_RS2",
				"_WIN32_WINNT_WIN10_RS3",
				"_WIN32_WINNT_WIN10_RS4",
				"_WIN32_WINNT_WIN10_RS5",
				"NTDDI_WIN10_CU"
			}
			);
    }
}
