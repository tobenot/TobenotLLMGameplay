// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#include "TAShoutManager.h"
#include "TAShoutComponent.h"
#include "OpenAIDefinitions.h"

void UTAShoutManager::Initialize(FSubsystemCollectionBase& Collection)
{
	// Initialization logic here (if necessary)
}

void UTAShoutManager::Deinitialize()
{
	// Cleanup logic here (if necessary)
}

void UTAShoutManager::RegisterShoutComponent(UTAShoutComponent* Component)
{
	if (Component && Component->GetOwner())
	{
		RegisteredShoutComponents.Add(Component);
	}
}

void UTAShoutManager::UnregisterShoutComponent(UTAShoutComponent* Component)
{
	if (Component && Component->GetOwner())
	{
		RegisteredShoutComponents.Remove(Component);
	}
}

void UTAShoutManager::BroadcastShout(const FChatCompletion& Message, AActor* Shouter, float Volume)
{
	TArray<UTAShoutComponent*> ComponentsInRange = GetShoutComponentsInRange(Shouter, Volume * 50.f);

	for (UTAShoutComponent* Listener : ComponentsInRange)
	{
		if (Listener && Listener->IsActive())
		{
			Listener->HandleShoutReceived(Message, Shouter, Volume);
		}
	}
}

TArray<UTAShoutComponent*> UTAShoutManager::GetShoutComponentsInRange(AActor* Shouter, float Range)
{
	TArray<UTAShoutComponent*> ComponentsInRange;
	for (const auto& Comp : RegisteredShoutComponents)
	{
		const AActor* ListenerActor = Comp->GetOwner();
		if (ListenerActor && Shouter && (ListenerActor->GetDistanceTo(Shouter) <= Range))
		{
			ComponentsInRange.Add(Comp);
		}
	}
	return ComponentsInRange;
}