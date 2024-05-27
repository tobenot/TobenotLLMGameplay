// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "Event/Core/TAEventInstance.h"

#include "Event/Core/TAEventSubsystem.h"
#include "Agent/TAAgentInterface.h"
#include "Event/TAEventLogCategory.h"
#include "Save/TASaveGameSubsystem.h"
#include "Scene/TASceneSubsystem.h"
#include "Chat/Shout/TAShoutComponent.h"

void UTAEventInstance::TriggerEvent()
{
	if(bTriggered)
	{
		UE_LOG(LogTAEventSystem, Warning, TEXT("TriggerEvent 尝试激活一个事件两次"));
		return;
	}
	bTriggered = true;
	
	// 在控制台和屏幕上打印事件信息
	if (GEngine)
	{
		FString Message = EventInfo.ToString();

		// 在屏幕上显示消息
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Message);

		// 在控制台打印消息
		UE_LOG(LogTAEventSystem, Log, TEXT("%s"), *Message);
	}
	
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTAEventSystem, Error, TEXT("TriggerEvent 无法获取当前世界"));
		return;
	}

	UTASceneSubsystem* SceneSubsystem = World->GetSubsystem<UTASceneSubsystem>();
	if (!SceneSubsystem)
	{
		UE_LOG(LogTAEventSystem, Error, TEXT("TriggerEvent 无法获取SceneSubsystem"));
		return;
	}
	
	if(EventInfo.ActivationType == EEventActivationType::Proximity)
	{
		SceneSubsystem->CreateAndLoadAreaScene(EventInfo);
	}

	AssignDesiresToAgent();
}

void UTAEventInstance::OnEventFinished(int32 OutcomeID)
{
	RevokeAgentDesires();
}

void UTAEventInstance::AssignDesiresToAgent()
{
	UWorld* World = GetWorld();
	if(World)
	{
		if (UTASaveGameSubsystem* SaveGameSubsystem = World->GetGameInstance()->GetSubsystem<UTASaveGameSubsystem>())
		{
			UE_LOG(LogTAEventSystem, Log, TEXT("事件 [%s] 开始分配欲望"), *EventInfo.PresetData.EventName);

			for (const FTAAgentDesire& Desire : EventInfo.PresetData.AgentDesires)
			{
				AActor* FoundActor = SaveGameSubsystem->FindActorByName(Desire.AgentName);
				if (FoundActor) 
				{
					ITAAgentInterface* AgentActor = Cast<ITAAgentInterface>(FoundActor);
					if (AgentActor)
					{
						FGuid DesireGUID = FGuid::NewGuid();

						// 这里添加了事件ID和名字到欲望描述
						FString DesireWithEventID = FString::Printf(TEXT("[EventID:%d] %s"), EventInfo.PresetData.EventID, *Desire.DesireDescription);
						AgentActor->AddOrUpdateDesire(DesireGUID, DesireWithEventID);
						
						if(Desire.ImmediatelyWantToSpeak)
						{
							UTAShoutComponent* ShoutComponent = FoundActor->FindComponentByClass<UTAShoutComponent>();
							if(ShoutComponent)
							{
								ShoutComponent->RequestToSpeakCheckSurrounding();
							}
						}
						
						DesireAgentMap.Add(DesireGUID, FoundActor);
                    
						UE_LOG(LogTAEventSystem, Log, TEXT("事件 [%s] 分配给 [%s] 的欲望 ： %s"), *EventInfo.PresetData.EventName, *Desire.AgentName.ToString(), *DesireWithEventID);
					}
				}
			}
		}
	}
}

void UTAEventInstance::RevokeAgentDesires()
{
	UE_LOG(LogTAEventSystem, Log, TEXT("事件 [%s] 开始撤销欲望"), *EventInfo.PresetData.EventName);
    
	for (const auto& Pair : DesireAgentMap)
	{
		const FGuid& DesireGUID = Pair.Key;
		AActor* FoundActor = Pair.Value;
		if (FoundActor)
		{
			ITAAgentInterface* AgentActor = Cast<ITAAgentInterface>(FoundActor);
			if (AgentActor)
			{
				AgentActor->RemoveDesire(DesireGUID);
				UE_LOG(LogTAEventSystem, Log, TEXT("事件 [%s] 分配给 [%s] 的欲望 已撤销"), *EventInfo.PresetData.EventName, *AgentActor->GetAgentName());
			}
		}
	}

	DesireAgentMap.Empty();
}