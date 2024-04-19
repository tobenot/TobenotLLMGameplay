// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#include "TAShoutManager.h"
#include "TAShoutComponent.h"
#include "OpenAIDefinitions.h"
#include "Common/TAAgentInterface.h"

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

bool UTAShoutManager::IsValidAgentName(const FString& MessageContent, AActor* Shouter) const
{
	const ITAAgentInterface* AgentInterface = Cast<ITAAgentInterface>(Shouter);
	if (AgentInterface)
	{
		FString AgentName = AgentInterface->GetAgentName();
		// 截取与AgentName长度相等的消息内容前缀
		FString Prefix = MessageContent.Left(AgentName.Len()).TrimStartAndEnd();
		// 忽略大小写地进行比较
		if (Prefix.Equals(AgentName, ESearchCase::IgnoreCase))
		{
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Agent name in content: '%s' Expected: '%s'"), *Prefix, *AgentName);
			return false;
		}
	}

	// 如果没有AgentInterface接口，或者Agent名字为空，就假定无需进行Agent名字验证，这条消息有效
	return true;
}