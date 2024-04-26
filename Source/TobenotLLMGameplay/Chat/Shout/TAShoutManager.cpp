// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#include "TAShoutManager.h"
#include "TAShoutComponent.h"
#include "OpenAIDefinitions.h"
#include "Agent/TAAgentInterface.h"
#include "Chat/TAChatLogCategory.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h" 

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
	TArray<UTAShoutComponent*> ComponentsInRange = GetShoutComponentsInRange(Shouter, Volume);

	// 目前发给其他人的消息只保留message字段
	FChatCompletion NewMessage;
	bool bIsNewMessageCreated = false;

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message.message.content);
    
	// 装载JSON和执行检查
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid() && JsonObject->HasField(TEXT("message")))
	{
		FString MessageContent;
		JsonObject->TryGetStringField(TEXT("message"), MessageContent);

		TSharedPtr<FJsonObject> NewJsonMessage = MakeShareable(new FJsonObject());
		NewJsonMessage->SetStringField(TEXT("message"), MessageContent);
		FString NewRawJson;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&NewRawJson);
		FJsonSerializer::Serialize(NewJsonMessage.ToSharedRef(), Writer);
  
		NewMessage.message.content = NewRawJson;
		bIsNewMessageCreated = true;
	}

	if(IsValidAgentName(Message.message.content, Shouter))
	{
		for (UTAShoutComponent* Listener : ComponentsInRange)
		{
			if (Listener && Listener->IsActive())
			{
				// 如果接收者不是发送者且新消息已被创建，则发送处理过的消息
				// 如果接收者是发送者，即使没有message字段，也应发送原始消息
				if(Listener->GetOwner() == Shouter)
				{
					Listener->HandleShoutReceived(Message, Shouter, Volume);
				}
				else if(bIsNewMessageCreated)
				{
					Listener->HandleShoutReceived(NewMessage, Shouter, Volume);
				}
				// 如果没有有效的message字段并且接收者不是发送者，不发送消息
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