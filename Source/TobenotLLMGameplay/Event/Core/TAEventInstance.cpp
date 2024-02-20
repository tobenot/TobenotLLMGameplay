// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAEventInstance.h"
#include "Event/TAEventLogCategory.h"
#include "Scene/TASceneSubsystem.h"

void UTAEventInstance::TriggerEvent()
{
	if(bTriggered)
	{
		UE_LOG(LogTAEventSystem, Warning, TEXT("TriggerEvent 尝试激活一个事件两次"));
		return;
	}
	bTriggered = true;
	
	// 在控制台和屏幕上打印事件信息
	if (GEngine)
	{
		FString Message = EventInfo.ToString();

		// 在屏幕上显示消息
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Message);

		// 在控制台打印消息
		UE_LOG(LogTAEventSystem, Log, TEXT("%s"), *Message);
	}
	
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTAEventSystem, Error, TEXT("TriggerEvent 无法获取当前世界"));
		return;
	}

	UTASceneSubsystem* SceneSubsystem = World->GetSubsystem<UTASceneSubsystem>();
	if (!SceneSubsystem)
	{
		UE_LOG(LogTAEventSystem, Error, TEXT("TriggerEvent 无法获取SceneSubsystem"));
		return;
	}

	SceneSubsystem->CreateAndLoadAreaScene(EventInfo);
}
