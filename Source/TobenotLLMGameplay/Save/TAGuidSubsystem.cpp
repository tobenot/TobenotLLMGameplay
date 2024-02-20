// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAGuidSubsystem.h"

// 在 UTAEventWarehouse.cpp 中实现相关方法

AActor* UTAGuidSubsystem::GetActorByGUID(const FGuid& Guid)
{
	AActor** FoundActor = GuidToActorMap.Find(Guid);
	return FoundActor ? *FoundActor : nullptr;
}

void UTAGuidSubsystem::RegisterActorGUID(const FGuid& Guid, AActor* Actor)
{
	if (Actor != nullptr)
	{
		GuidToActorMap.Add(Guid, Actor);
	}
}

void UTAGuidSubsystem::UnregisterActorGUID(const FGuid& Guid)
{
	GuidToActorMap.Remove(Guid);
}