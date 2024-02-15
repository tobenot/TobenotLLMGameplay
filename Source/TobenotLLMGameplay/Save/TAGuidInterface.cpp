// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAGuidInterface.h"

#include "TASaveGameSubsystem.h"

// Add default functionality here for any ITAGuidInterface functions that are not pure virtual.
FGuid ITAGuidInterface::GetTAGuid()
{
	if(!TAGuid.IsValid())
	{
		GenNewTAGuid();
	}
	return TAGuid;
}

void ITAGuidInterface::RegisterActorTAGuid(AActor* Actor, FName Name)
{
	if (UTASaveGameSubsystem* SaveGameSubsystem = Actor->GetGameInstance()->GetSubsystem<UTASaveGameSubsystem>())
	{
		SaveGameSubsystem->RegisterActorTAGuid(Actor, Name);
	}
}

void ITAGuidInterface::GenNewTAGuid()
{
	if(!TAGuid.IsValid())
	{
		TAGuid = FGuid::NewGuid();
	}
}

void ITAGuidInterface::SetTAGuid(FGuid NewGuid)
{
	TAGuid = NewGuid;
}
