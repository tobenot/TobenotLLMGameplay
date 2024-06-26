// Copyright (c) 2024 tobenot
// This code is licensed under the MIT License. See LICENSE in the project root for license information.

using UnrealBuildTool;

public class TobenotLLMGameplay : ModuleRules
{
	public TobenotLLMGameplay(ReadOnlyTargetRules Target) : base(Target)
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
				"OpenAIAPI",
				"UMG",
				"Json",
				"Http",
				"ApplicationCore",
				"DeveloperSettings",
				
				// 下载图片
				"RHI",
				"RenderCore", 
				
				"TobenotToolkit",
				"NavigationSystem", // 场景系统用
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		
		PublicIncludePaths.AddRange(
			new string[] {
				"TobenotLLMGameplay",
				"TobenotLLMGameplay/Agent",
				"TobenotLLMGameplay/Chat",
				"TobenotLLMGameplay/Common",
				"TobenotLLMGameplay/Event",
                "TobenotLLMGameplay/Image",
                "TobenotLLMGameplay/Save",
                "TobenotLLMGameplay/Scene",
                "TobenotLLMGameplay/UI",
			}
		);
                
		PrivateIncludePaths.AddRange(
			new string[] {
				"TobenotLLMGameplay",
				"TobenotLLMGameplay/Agent",
				"TobenotLLMGameplay/Chat",
				"TobenotLLMGameplay/Common",
				"TobenotLLMGameplay/Event",
				"TobenotLLMGameplay/Image",
				"TobenotLLMGameplay/Save",
				"TobenotLLMGameplay/Scene",
				"TobenotLLMGameplay/UI",
			}
		);
	}
}
