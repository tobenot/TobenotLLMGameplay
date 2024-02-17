// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAEventSubsystem.h"

#include "TAEventInstance.h"
#include "TAEventPool.h" // 引入事件池头文件

void UTAEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 创建事件池实例
	EventPool = NewObject<UTAEventPool>(this, UTAEventPool::StaticClass());
	if (EventPool)
	{
		// 可以在此处进行事件池的初始化配置，如果有需要

		// 创建测试事件实例
		UTAEventInstance* NewEvent = NewObject<UTAEventInstance>(this, UTAEventInstance::StaticClass());
		if (NewEvent)
		{
			// 初始化事件信息
			NewEvent->EventInfo.EventID = 1;
			NewEvent->EventInfo.Description = TEXT("这是一个测试事件");
			NewEvent->EventInfo.EventType = ETAEventType::Other;
			NewEvent->EventInfo.Weight = 1;

			// 将事件添加到事件池中
			EventPool->AddEvent(NewEvent);
		}
	}
}

// Deinitialize 方法，负责资源的清理
void UTAEventSubsystem::Deinitialize()
{
	// 清理事件池里面的事件实例等
	// ...

	Super::Deinitialize();
}