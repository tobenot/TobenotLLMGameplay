// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "Event/Data/TAEventWarehouse.h"

#include "Engine/DataTable.h"
#include "Event/TAEventLogCategory.h"
#include "Event/Core/TAEventSubsystem.h"

void UTAEventWarehouse::LoadEventsFromDataTable(UDataTable* DataTable)
{
	// 检查数据表是否有效
	if (!DataTable)
	{
		UE_LOG(LogTAEventSystem, Error, TEXT("LoadEventsFromDataTable 数据表无效"));
		return;
	}

	// 获取数据表行
	TArray<FTAPresetEventData*> Events;
	DataTable->GetAllRows<FTAPresetEventData>(TEXT("查找所有预设事件数据"), Events);
	
	UE_LOG(LogTAEventSystem, Log, TEXT("导入预设事件数据：[%s]"), *DataTable->GetName());
	
	// 将事件添加到事件池中
	for (FTAPresetEventData* EventData : Events)
	{
		if (EventData)
		{
			UTAEventSubsystem* EventSubsystem = GetWorld()->GetSubsystem<UTAEventSubsystem>();
			if (EventSubsystem)
			{
				EventSubsystem->AddEventToPoolByData(*EventData);
			}
		}
	}
}
