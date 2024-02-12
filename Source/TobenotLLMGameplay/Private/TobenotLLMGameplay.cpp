// Copyright (c) 2024 tobenot
// This code is licensed under the MIT License. See LICENSE in the project root for license information.


#include "TobenotLLMGameplay.h"

#define LOCTEXT_NAMESPACE "FTobenotLLMGameplayModule"

void FTobenotLLMGameplayModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FTobenotLLMGameplayModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTobenotLLMGameplayModule, TobenotLLMGameplay)