// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#include "TAShoutManager.h"
#include "TAShoutComponent.h"
#include "OpenAIDefinitions.h"
#include "Agent/TAAgentInterface.h"
#include "Chat/TAChatLogCategory.h"

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

	if(IsValidAgentName(Message.message.content, Shouter)){
		for (UTAShoutComponent* Listener : ComponentsInRange)
		{
			if (Listener && Listener->IsActive())
			{
				Listener->HandleShoutReceived(Message, Shouter, Volume);
			}
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
	const FString& MessageKey = TEXT("\"message\": \"");
	int32 MessageStartIndex = MessageContent.Find(MessageKey);
	if (MessageStartIndex > 0)
	{
		// 计算出实际消息内容的起始位置
		int32 ActualMessageStart = MessageStartIndex + MessageKey.Len();

		const ITAAgentInterface* AgentInterface = Cast<ITAAgentInterface>(Shouter);
		if (AgentInterface)
		{
			FString AgentName = AgentInterface->GetAgentName();
			if(AgentInterface->IsVoiceover())
			{
				UE_LOG(LogTemp, Log, TEXT("Is Voiceover, agent [%s], send message without name check."), *AgentName);
				return true;
			}
			// 截取与AgentName等长的字符串，用于比较
			FString AgentNameInContent = MessageContent.Mid(ActualMessageStart, AgentName.Len()).TrimStartAndEnd();
			if (AgentNameInContent.Equals(AgentName, ESearchCase::IgnoreCase))
			{
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid Agent name in content: '%s' Expected: '%s'"), *AgentNameInContent, *AgentName);
				return false;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find key '\"message\": \"' in content: %s"), *MessageContent);
		return true;
	}
    
	// 如果没有AgentInterface，则默认消息有效
	return true;
}