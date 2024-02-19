// Copyright (c) 2024 tobenot
// This code is licensed under the MIT License. See LICENSE in the project root for license information.


#include "TobenotLLMGameplay.h"

#define LOCTEXT_NAMESPACE "FTobenotLLMGameplayModule"

void FTobenotLLMGameplayModule::StartupModule()
{
}

// 模块卸载时调用
void FTobenotLLMGameplayModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTobenotLLMGameplayModule, TobenotLLMGameplay)