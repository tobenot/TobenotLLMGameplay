#include "Event/Generator/TAEventGenPrompt.h"
#include "Common/TAPromptDefinitions.h"
#include "Event/Generator/TAEventGenerator.h"

void UTAEventGenerator::InitPrompt()
{
	PromptGenerateEvent = FTAPrompt{
		"Your role is specialized in constructing thrilling events for games. "
		"Themes you could explore, but are not confined to:"
			"- SaveVillager: The player expected to rescue a villager from impending peril."
			"- FindItem: The player tasked with locating a significant item in the vicinity."
			"- EnemyEncounter: The player on the verge of an enemy encounter."
		"Follow the ensuing JSON template for the event's structure: "
		"["
			"{"
				"\"AdventurePoints\": [English] As an adventure game,"
				"\"LocationName\":  Please provide in [{Language}]. A more precise geographical location used to describe the occurrence of the event. "
				"\"Description\": A detailed narrative of the event."
				"\"EventType\": A number denoting the type of event, options ranging from 0Combat, 1BossFight, 2Exploration, 3Story, to 4Other,"
				"\"Weight\": a numerical value (0-99) indicating the probability of the event's occurrence"
			"},"
		"]"
		,1
		,true
	};

	PromptGenerateEventByDescription = FTAPrompt{
		"Your role is specialized in constructing a thrilling event for games. "
		"Theme is [{Theme}]."
		"Follow the ensuing JSON template for the event's structure: "
		"["
			"{"
				"\"AdventurePoints\": [English] As an adventure game,"
				"\"LocationName\":  Please provide in [{Language}]. A more precise geographical location used to describe the occurrence of the event. "
				"\"Description\": A detailed narrative of the event. This not show to player, you can put some development notes here."
				"\"EventType\": A number denoting the type of event, options ranging from 0Combat, 1BossFight, 2Exploration, 3Story, to 4Other,"
				"\"Weight\": a numerical value (0-99) indicating the probability of the event's occurrence"
			"},"
		"]"
		,1
		,true
	};
}
