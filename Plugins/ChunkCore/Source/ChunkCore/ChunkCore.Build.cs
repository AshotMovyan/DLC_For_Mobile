// Copyright (C) 2017-2021 | eelDev (Dry Eel Development)

using UnrealBuildTool;
using System.IO;

public class ChunkCore : ModuleRules
{
	public ChunkCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
			);
				
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"ChunkDownloader"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"ChunkDownloader",
				"HTTP",
				"Projects",
				"PakFile"
			}
			);
	}
}
