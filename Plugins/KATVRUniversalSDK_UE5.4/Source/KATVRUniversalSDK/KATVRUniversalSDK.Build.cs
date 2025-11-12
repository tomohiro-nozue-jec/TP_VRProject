// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using System;

public class KATVRUniversalSDK : ModuleRules
{
	public KATVRUniversalSDK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		//string dllDir = Path.Combine(ModuleDirectory, "ThirdParty/KAT");
		var fDir = Directory.GetParent(ModuleDirectory + "\\1.dum");
		var fstr = fDir.Parent.Parent.FullName + "\\ThirdParty\\KAT";
       
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "KAT_MobileLibs.xml"));
			PublicAdditionalLibraries.Add(fstr + "\\libKATSDKWarpper.so");
			PublicAdditionalLibraries.Add(fstr + "\\libkdroidhelper.so");
			PublicAdditionalLibraries.Add(fstr + "\\libNexusClient.so");
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64){
			//PublicAdditionalLibraries.Add(fstr + "\\KATSDKWarpper.dll");
		}

		foreach (string file in Directory.GetFiles(fstr))
		{
            string filename = Path.GetFileName(file);
			string extension = Path.GetExtension(file).ToLower();
            System.Console.WriteLine("dll Found:" + file);
            System.Console.WriteLine("dll Found extension:" + extension);
			//PublicAdditionalLibraries.Add(file);
			if ((extension.Equals(".so") && Target.Platform == UnrealTargetPlatform.Android) ||
				(extension.Equals(".dll") && Target.Platform == UnrealTargetPlatform.Win64))
			{
                RuntimeDependencies.Add(file);
                // delay load

                PublicDelayLoadDLLs.Add(filename);
            }
            
		}
	}
}
