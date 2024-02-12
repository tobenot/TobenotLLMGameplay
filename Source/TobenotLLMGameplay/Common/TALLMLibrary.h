// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "OpenAIDefinitions.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TALLMLibrary.generated.h"

UENUM(BlueprintType)
enum class ELLMChatEngineQuality : uint8
{
	Fast UMETA(DisplayName = "Fast"),
	Moderate UMETA(DisplayName = "Moderate"),
	HighQuality UMETA(DisplayName = "High Quality")
};

/**
 * 
 */
UCLASS()
class TOBENOTLLMGAMEPLAY_API UTALLMLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Chat Engine")
	static EOAChatEngineType GetChatEngineTypeFromQuality(const ELLMChatEngineQuality Quality);
};
