// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAEventInstance.h"
#include "Event/TAEventLogCategory.h"

void UTAEventInstance::TriggerEvent()
{
	// 在控制台和屏幕上打印事件信息
	if (GEngine)
	{
		FString Message = EventInfo.ToString();

		// 在屏幕上显示消息
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Message);

		// 在控制台打印消息
		UE_LOG(LogTAEventSystem, Log, TEXT("%s"), *Message);
	}
}
