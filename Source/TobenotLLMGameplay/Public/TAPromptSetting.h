// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Common/TAPromptDefinitions.h"
#include "UObject/Object.h"
#include "TAPromptSetting.generated.h"

/**
 * 
 */

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAPromptSetting : public UObject
{
	GENERATED_BODY()

public:
	FTAPrompt PromptEventGenInteractables = FTAPrompt{
		"Enhance game interactivity by examining event details provided: [{EventInfo}]. Create engaging game elements with unique, witty characteristics. Use this JSON template:"
			"{"
				"\"Interactables\": ["
					"{"
					"\"Name\": \"[{Language}] Label for interactive entities like Enchanted Cauldron or Arcane Relic\","
					"\"UniqueFeature\": \"Describes distinctive quirks, from humorous dialogue to enchanting effects\","
					"\"Objective\": \"Goals for players, such as brewing a potion, defeating a foe, or solving riddles\""
					"}"
					"// Develop more interactables using this format"
				"]"
			"}"
			"Design with these guidelines: Include clever, humor-filled challenges with distinctive eccentricities that demand bravery, focusing on combat encounters, as peaceful solutions with monsters aren't an option."
			"Please generate interactive objects that are directly related to the event and will progress the event when interacted with."
		,1
		,true
	};
};
