// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAEventSubsystem.h"

#include "TASettings.h"
#include "TAEventInstance.h"
#include "TAEventPool.h"
#include "Event/TAEventLogCategory.h"
#include "Event/Generator/TAEventGenerator.h"
#include "Image/TAImageGenerator.h"
#include "Save/TAGuidInterface.h"
#include "Scene/TASceneSubsystem.h"

UTAEventPool* UTAEventSubsystem::GetEventPool()
{
	if(!EventPoolRef)
	{
		EventPoolRef = NewObject<UTAEventPool>(this, UTAEventPool::StaticClass());
	}
	return EventPoolRef;
}

void UTAEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

// HandleGeneratedEvents 方法，用于处理生成器成功生成的事件
void UTAEventSubsystem::HandleGeneratedEvents(TArray<FTAEventInfo>& GeneratedEvents)
{
	UTASceneSubsystem* SceneSubsystem = GetWorld()->GetSubsystem<UTASceneSubsystem>();

	for (FTAEventInfo& EventInfo : GeneratedEvents)
	{
		// 通过场景子系统查询地点信息，获得位点Actor
		ATAPlaceActor* PlaceActor = SceneSubsystem->QueryEventLocationByInfo(EventInfo);
        
		if (PlaceActor)
		{
			// 获取位点Actor的Guid
			ITAGuidInterface* GuidInterface = Cast<ITAGuidInterface>(PlaceActor);
			FGuid LocationGuid = GuidInterface->GetTAGuid();

			// 为事件设置地点GUID
			EventInfo.LocationGuid = LocationGuid;
			EventInfo.ActivationType = EEventActivationType::Proximity;

			UTAEventPool* EventPool = GetEventPool();
			// 将事件和它的地点GUID添加到事件池
			auto& Info = EventPool->AddEvent(EventInfo);

			// 如果有必要，生成事件对应的图像或者其他资源
			GenerateImageForEvent(Info);
		}
		else
		{
			UE_LOG(LogTAEventSystem, Warning, TEXT("无法为EventID %d 找到对应的位点Actor"), EventInfo.PresetData.EventID);
		}
	}
}

void UTAEventSubsystem::AddEventToPoolByData(FTAPresetEventData EventData)
{
	UE_LOG(LogTAEventSystem, Log, TEXT("UTAEventSubsystem AddEventToPoolByData %s"), *EventData.EventName);
	FTAEventInfo EventInfo;
	EventInfo.PresetData = EventData;
	EventInfo.ActivationType = EEventActivationType::PlotProgress;
	if(UTAEventPool* EventPool = GetEventPool())
	{
		auto& Info = EventPool->AddEvent(EventInfo);
	}
}

void UTAEventSubsystem::FinishEvent(int32 EventID, int32 OutcomeID)
{
	if (UTAEventPool* EventPool = GetEventPool())
	{
		auto Event = EventPool->GetEventInstanceByID(EventID);
		if(Event)
		{
			Event->OnEventFinished(OutcomeID);
			EventPool->AddCompletedEvent(EventID, OutcomeID);
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

void UTAEventSubsystem::Start(const int32& GenEventNum)
{
	// 确保事件池创建成功
	if (UTAEventPool* EventPool = GetEventPool())
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
						EventGenerator->RequestEventGeneration(CurrentSceneInfo, GenEventNum);
					}
				}
			}else
			{
				UE_LOG(LogTAEventSystem, Error, TEXT("EventGeneratorClass 未配置，你可以在项目设置里找到设置位置，建议自己继承一个事件生成器类UTAEventGenerator"));
			}
		}
		
	}
}

bool UTAEventSubsystem::HasAnyEventsInPool() const
{
	if (EventPoolRef)
	{
		return EventPoolRef->HasAnyEvents();
	}
	else
	{
		// 如果没有初始化事件池，那么返回false
		return false;
	}
}

void UTAEventSubsystem::GenerateImageForEvent(const FTAEventInfo& GeneratedEvent)
{
	UTAImageGenerator* ImageGenerator = NewObject<UTAImageGenerator>(this);
	if (ImageGenerator)
	{
		ImageGenerator->RequestGenerateImage(GeneratedEvent);
	}
}
