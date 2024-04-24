// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAAgentInterface.h"


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
