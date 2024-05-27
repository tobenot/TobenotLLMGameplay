// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#include "Event/Data/TAEventInfo.h"

FString FTAEventInfo::ToString() const
{
	FString EventTypeStr;
	switch (PresetData.EventType)
	{
	case ETAEventType::Combat: EventTypeStr = TEXT("Normal Combat"); break;
	case ETAEventType::BossFight: EventTypeStr = TEXT("Boss Fight"); break;
	case ETAEventType::Exploration: EventTypeStr = TEXT("Exploration"); break;
	case ETAEventType::Story: EventTypeStr = TEXT("Story"); break;
	case ETAEventType::Other: EventTypeStr = TEXT("Other"); break;
	default: EventTypeStr = TEXT("Unknown"); break;
	}
	return FString::Printf(TEXT("{\"EventID\": %d, \"Description\": \"%s\", \"Type\": \"%s\", \"Weight\": %d, \"LocationGuid: %s\"}"),
		PresetData.EventID,
		*PresetData.Description,
		*EventTypeStr,
		PresetData.Weight
	,	*LocationGuid.ToString()
	);
}
