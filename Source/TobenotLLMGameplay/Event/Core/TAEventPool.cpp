// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAEventPool.h"

FTAEventInfo& UTAEventPool::AddEvent(FTAEventInfo EventInfo)
{
	FTAEventInfo& EventInfoRef = AllEventInfo.Add_GetRef(EventInfo);
	EventInfoRef.EventID = AllEventInfo.Num() + 660000;
	UTAEventInstance* NewEvent = NewObject<UTAEventInstance>(this, UTAEventInstance::StaticClass());
	if (NewEvent)
	{
		// 使用生成的事件信息初始化NewEvent
		NewEvent->EventInfo = EventInfo;
		// 将事件添加到待触发的事件集合
		PendingEvents.Add(NewEvent);

		// 也可以立刻添加到活跃事件中，如果需要立即触发
		ActiveEvents.Add(NewEvent);
		NewEvent->TriggerEvent();
	}
	return EventInfoRef;
}

bool UTAEventPool::GetEventByID(int32 EventID, FTAEventInfo& OutEventInfo)
{
	for (FTAEventInfo& EventInfoRef : AllEventInfo)
	{
		if (EventInfoRef.EventID == EventID)
		{
			OutEventInfo = EventInfoRef;
			return true;
		}
	}
	
	OutEventInfo = ZeroEvent;
	return false;
}

