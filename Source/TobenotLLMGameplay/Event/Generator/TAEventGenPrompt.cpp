#include "TAEventGenPrompt.h"
#include "Common/TAPromptDefinitions.h"
#include "TAEventGenerator.h"

void UTAEventGenerator::InitPrompt()
{
	PromptGenerateEvent = FTAPrompt{
		"Your role is specialized in constructing thrilling events for games. "
		"Follow the ensuing JSON template for the event's structure: "
		"{"
			"\"Events\": ["
				"{"
				"\"LocationName\": \"[{Language}] Denotes the name of the event location, displayed on the large map, such as wells, grasslands\","
				"\"Description\": \"[{Language}] A detailed narrative of the event\","
				"\"EventType\": \"A number denoting the type of event, options ranging from 0Combat, 1BossFight, 2Exploration, 3Story, to 4Other\","
				"\"Weight\": \"a numerical value (0-99) indicating the probability of the event's occurrence\""
				"},"
				"// Add 4 more events following the same structure"
			"]"
		"}"
		"Themes you could explore, but are not confined to:"
			"- SaveVillager: The player expected to rescue a villager from impending peril."
			"- FindItem: The player tasked with locating a significant item in the vicinity."
			"- EnemyEncounter: The player on the verge of an enemy encounter."
		,1
		,true
	};
}
