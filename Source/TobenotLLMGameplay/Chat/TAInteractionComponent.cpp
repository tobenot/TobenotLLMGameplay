// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAInteractionComponent.h"

#include "TobenotLLMGameplay/Common/TALLMLibrary.h"
#include "TobenotLLMGameplay/Common/TASystemLibrary.h"

UTAInteractionComponent::UTAInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTAInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

FString UTAInteractionComponent::GetFullPrompt()
{
	return UTALLMLibrary::PromptToStr(InteractiveActorPromptTemplate)
	.Replace(TEXT("{Language}"), *UTASystemLibrary::GetGameLanguage())
	.Replace(TEXT("{InteractableActorProfile}"), *InteractableInfo.Name)
	.Replace(TEXT("{UniqueFeature}"), *InteractableInfo.UniqueFeature)
	.Replace(TEXT("{Objective}"), *InteractableInfo.Objective)
	.Replace(TEXT("{BelongEventDescription}"), *BelongEventDescription)
	;
}

void UTAInteractionComponent::InitPrompt()
{
	InteractiveActorPromptTemplate = FTAPrompt{
		"Become an interactive entity within a game, embodying the role of [{InteractableActorProfile}]."
		"Use the following JSON format for your responses:"

		"{"
		"    \"message\": \"A concise narrative description of the interaction's progress.\","
		"}"
		"        Employ clear and engrossing language to deepen immersion and avoid player confusion."
		"Please provide your response in [{Language}]." 
		,1
		,true
	};
}

void UTAInteractionComponent::InitializeComponent()
{
	Super::InitializeComponent();
	InitPrompt();
}
