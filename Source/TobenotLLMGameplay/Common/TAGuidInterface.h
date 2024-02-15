// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TAGuidInterface.generated.h"

UINTERFACE()
class UTAGuidInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TOBENOTLLMGAMEPLAY_API ITAGuidInterface
{
	GENERATED_BODY()
private:
	FGuid TAGuid;

public:
	FGuid GetTAGuid()
	{
		if(!TAGuid.IsValid())
		{
			InitTAGuid();
		}
		return TAGuid;
	}
	
	void InitTAGuid()
	{
		if(!TAGuid.IsValid())
		{
			TAGuid = FGuid::NewGuid();
		}
	}
};
