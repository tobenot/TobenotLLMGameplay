// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "Agent/TAAgentInterface.h"


// Add default functionality here for any ITAAgentInterface functions that are not pure virtual.
int32 ITAAgentInterface::GetAgentSpeakPriority() const
{
	return 100;
}

void ITAAgentInterface::AddOrUpdateDesire(const FGuid& DesireId, const FString& DesireDescription)
{
}

void ITAAgentInterface::RemoveDesire(const FGuid& DesireId)
{
}

bool ITAAgentInterface::IsVoiceover() const
{
	return false;
}

TMap<FName, int32> ITAAgentInterface::QueryInventoryItems() const
{
	TMap<FName, int32> InventoryItems;
	return InventoryItems;
}
int32 ITAAgentInterface::QueryItemAmountByName(FName ItemName) const
{
	return 0;
}

bool ITAAgentInterface::ConsumeInventoryItem(FName ItemName, int32 ConsumeCount)
{
	return false;
}

FString ITAAgentInterface::GetPerceptionData() const
{
	return "未实现感知接口";
}

FString ITAAgentInterface::GetMemoryData() const
{
	return "未实现记忆接口";
}
