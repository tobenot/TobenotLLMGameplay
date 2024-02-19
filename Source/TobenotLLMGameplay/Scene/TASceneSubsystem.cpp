// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TASceneSubsystem.h"

FString UTASceneSubsystem::QuerySceneMapInfo()
{
	return "The Mushroom Village on the grassland, where the Mushroom Monsters are hostile.";
}

FString UTASceneSubsystem::QueryLocationInfo(const FVector& Location)
{
	return "High GrassLand";
}

FVector UTASceneSubsystem::QueryEventLocationByInfo(const FTAEventInfo& EventInfo)
{
	return FVector::ZeroVector;
}
