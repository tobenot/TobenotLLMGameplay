// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAEventPool.h"

void UTAEventPool::AddEvent(UTAEventInstance* Event)
{
	if (Event != nullptr)
	{
		// 将事件添加到待触发的事件集合
		PendingEvents.Add(Event);

		// 也可以立刻添加到活跃事件中，如果需要立即触发
		ActiveEvents.Add(Event);
		Event->TriggerEvent();
	}
}