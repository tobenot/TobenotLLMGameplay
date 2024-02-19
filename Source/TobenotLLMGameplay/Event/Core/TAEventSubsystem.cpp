// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAEventSubsystem.h"

#include "TASettings.h"
#include "TAEventInstance.h"
#include "TAEventPool.h"
#include "Event/TAEventLogCategory.h"
#include "Event/Generator/TAEventGenerator.h"
#include "Image/TAImageGenerator.h"
#include "Scene/TASceneSubsystem.h"

void UTAEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

// HandleGeneratedEvents 方法，用于处理生成器成功生成的事件
void UTAEventSubsystem::HandleGeneratedEvents(const TArray<FTAEventInfo>& GeneratedEvents)
{
	for (const FTAEventInfo& EventInfo : GeneratedEvents)
	{
		auto& Info = EventPool->AddEvent(EventInfo);
		GenerateImageForEvent(Info);
	}
}

// Deinitialize 方法，负责资源的清理
void UTAEventSubsystem::Deinitialize()
{
	// 清理事件池里面的事件实例等
	// ...

	Super::Deinitialize();
}

void UTAEventSubsystem::Start()
{
	// 创建事件池实例
	EventPool = NewObject<UTAEventPool>(this, UTAEventPool::StaticClass());
    
	// 确保事件池创建成功
	if (EventPool)
	{
		UClass* EventGeneratorClass;
		const UTASettings* Settings = GetDefault<UTASettings>();
		if (Settings)
		{
			FString ClassPath = Settings->EventGeneratorClass.ToString();
			EventGeneratorClass = LoadClass<UTAEventGenerator>(nullptr, *ClassPath);
			if (EventGeneratorClass != nullptr)
			{
				// 创建事件生成器实例
				UTAEventGenerator* EventGenerator = NewObject<UTAEventGenerator>(this, EventGeneratorClass);
				if (EventGenerator)
				{
					// 给生成成功的委托绑定一个lambda函数，用以处理生成的事件
					EventGenerator->OnEventGenerationSuccess.AddDynamic(this, &UTAEventSubsystem::HandleGeneratedEvents);
			
					UTASceneSubsystem* SceneSubsystem = GetWorld()->GetSubsystem<UTASceneSubsystem>();
					if (SceneSubsystem)
					{
						FString CurrentSceneInfo = SceneSubsystem->QuerySceneMapInfo();
                
						// 请求生成事件，传入场景信息
						EventGenerator->RequestEventGeneration(CurrentSceneInfo);
					}
				}
			}else
			{
				UE_LOG(LogTAEventSystem, Error, TEXT("EventGeneratorClass 未配置，你可以在项目设置里找到设置位置，建议自己继承一个事件生成器类UTAEventGenerator"));
			}
		}
		
	}
}


// UTAEventSubsystem.cpp

void UTAEventSubsystem::GenerateImageForEvent(const FTAEventInfo& GeneratedEvent)
{
	UTAImageGenerator* ImageGenerator = NewObject<UTAImageGenerator>(this);
	if (ImageGenerator)
	{
		ImageGenerator->RequestGenerateImage(GeneratedEvent);
	}
}