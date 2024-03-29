// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Chat/TAChatComponent.h"
#include "GameFramework/SaveGame.h"
#include "TASaveGame.generated.h"

/**
 * 
 */
UCLASS()
class TOBENOTLLMGAMEPLAY_API UTASaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UTASaveGame();

public:
	UPROPERTY(VisibleAnywhere, Category = "TAGuid")
	TMap<FName, FGuid> NameGuidMap;
	
	UPROPERTY(VisibleAnywhere, Category = "Serialization")
	TMap<FGuid, FString> SerializedDataMap;
	
	UPROPERTY(VisibleAnywhere, Category = "Chat")
	TMap<FGuid, FTAChatComponentSaveData> TAChatDataMap;
};
