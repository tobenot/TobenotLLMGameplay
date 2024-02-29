// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#include "TADialogueInstance.h"

#include "OpenAIDefinitions.h"
#include "TADialogueComponent.h"
#include "Common/TAAgentInterface.h"
#include "Engine/World.h"


// 帮助根据说话优先级排序的结构体
struct FParticipantPriority
{
	AActor* Participant;
	int32 Priority;

	FParticipantPriority(AActor* InParticipant, int32 InPriority)
		: Participant(InParticipant)
		, Priority(InPriority)
	{
	}

	// 排序规则，优先级高的在前
	bool operator<(const FParticipantPriority& Other) const
	{
		return Priority > Other.Priority;
	}
};

UTADialogueInstance::UTADialogueInstance()
{
	DialogueState = EDialogueState::WaitingForParticipants;
	CurrentParticipantIndex = 0;
}

void UTADialogueInstance::AddParticipant(AActor* Participant)
{
	if (Participant && !Participants.Contains(Participant))
	{
		// 如果Actor不在参与者列表中，添加到列表
		Participants.Add(Participant);
		if (Participants.Num() >= 2 && DialogueState == EDialogueState::WaitingForParticipants)
		{
			DialogueState = EDialogueState::Active;

			// Start the timer (e.g., every 5 seconds)
			GetWorld()->GetTimerManager().SetTimer(DialogueTimerHandle, this, &UTADialogueInstance::CycleParticipants,
				1.0f, true, 2.0f);
		}
	}
}

void UTADialogueInstance::RemoveParticipant(AActor* Participant)
{
	if (Participant)
	{
		// 如果Actor在参与者列表中，从列表移除
		Participants.Remove(Participant);
	}
}

void UTADialogueInstance::TryEnterDialogueState(EDialogueState NewState)
{
	// 校验权限或者其他逻辑，在适当的条件下更新对话状态
	DialogueState = NewState;
}

void UTADialogueInstance::ReceiveMessage(const FChatCompletion& Message, AActor* Sender)
{
	// 收到消息后，将其添加到历史记录并分发给所有参与者
	AddMessageToHistory(Message.message,Sender);
	DistributeMessage(Message,Sender);
	DialogueState = EDialogueState::Active;
}

void UTADialogueInstance::CycleParticipants()
{
	if(DialogueState != EDialogueState::Active)
	{
		// 如果对话状态不是激活状态，直接返回
		return;
	}

	// 遍历所有参与者以检查他们的会话组件中的接受消息标志
	for (AActor* Participant : Participants)
	{
		UTADialogueComponent* DialogueComponent = Participant ? Participant->FindComponentByClass<UTADialogueComponent>() : nullptr;
		if (DialogueComponent && !DialogueComponent->GetAcceptMessages())
		{
			// 如果有参与者不愿意接受消息，结束对话
			EndDialogue();
			return;
		}
	}

	// 创建一个数组用于存储带有优先级的参与者
	TArray<FParticipantPriority> SortedParticipants;

	// 遍历所有参与者并获取他们的优先级
	for (AActor* Participant : Participants)
	{
		if (Participant)
		{
			const ITAAgentInterface* AgentInterface = Cast<ITAAgentInterface>(Participant);
			if (AgentInterface)
			{
				// 获取参与者的优先级并创建FParticipantPriority结构体存入数组
				int32 Priority = AgentInterface->GetAgentSpeakPriority();
				SortedParticipants.Add(FParticipantPriority(Participant, Priority));
			}
		}
	}

	// 根据优先级进行排序
	SortedParticipants.Sort();

	// 根据当前参与者索引选择下一个发言者
	if (SortedParticipants.IsValidIndex(CurrentParticipantIndex))
	{
		// 获取选择的参与者
		AActor* ParticipantToSpeak = SortedParticipants[CurrentParticipantIndex].Participant;
        
		// 请求选择的参与者发言
		UTADialogueComponent* DialogueComponent = ParticipantToSpeak->FindComponentByClass<UTADialogueComponent>();
		if (DialogueComponent)
		{
			DialogueState = EDialogueState::WaitingForSomeOne;
			DialogueComponent->RequestToSpeak();
		}

		// 更新当前参与者索引
		CurrentParticipantIndex = (CurrentParticipantIndex + 1) % SortedParticipants.Num();
	}

	// 如果排序后的参与者数组为空，或者当前索引无效，则重置对话状态
	if(SortedParticipants.Num() == 0 || !SortedParticipants.IsValidIndex(CurrentParticipantIndex))
	{
		DialogueState = EDialogueState::Active;
		CurrentParticipantIndex = 0; // 重置索引
	}
}

void UTADialogueInstance::EndDialogue()
{
	DialogueState = EDialogueState::End;
	DialogueTimerHandle.Invalidate();
}

void UTADialogueInstance::RefuseToSay(AActor* Sender)
{
	// 参与者拒绝说话，重置对话状态为Active
	DialogueState = EDialogueState::Active;
}

void UTADialogueInstance::AddMessageToHistory(const FChatLog& Message, AActor* Sender)
{
	// 将消息添加到对话历史记录
	DialogueHistory.Add(Message);
}

void UTADialogueInstance::DistributeMessage(const FChatCompletion& Message, AActor* Sender)
{
	// 广播消息给参与者
	for (AActor* Participant : Participants)
	{
		UTADialogueComponent* ChatComponent = Participant->FindComponentByClass<UTADialogueComponent>();
		if (ChatComponent)
		{
			// 参与者通过其会话组件来处理接收到的消息
			ChatComponent->HandleReceivedMessage(Message, Sender);
		}
	}
}

void UTADialogueInstance::BeginDestroy()
{
	DialogueTimerHandle.Invalidate();
	UObject::BeginDestroy();
}


TArray<FString> UTADialogueInstance::GetParticipantNamesFromAgents() const
{
	TArray<FString> ParticipantNames;
	for (const AActor* Participant : Participants)
	{
		if (Participant)
		{
			const ITAAgentInterface* AgentInterface = Cast<ITAAgentInterface>(Participant);
			if (AgentInterface)
			{
				//获取接口中的Participant名字并加到数组中
				ParticipantNames.Add(AgentInterface->GetAgentName());
			}
			else
			{
				// 如果Participant没有实现ITAAgentInterface, 记录错误
				UE_LOG(LogTemp, Error, TEXT("Participant '%s' does not implement ITAAgentInterface."), *Participant->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("One of the participants is null."));
		}
	}

	return ParticipantNames;
}

FString UTADialogueInstance::GetParticipantsNamesStringFromAgents() const
{
	FString ParticipantsNamesString;
	for (const AActor* Participant : Participants)
	{
		if (Participant)
		{
			const ITAAgentInterface* AgentInterface = Cast<ITAAgentInterface>(Participant);
			if (AgentInterface)
			{
				if (!ParticipantsNamesString.IsEmpty())
				{
					ParticipantsNamesString += TEXT(", ");
				}
				ParticipantsNamesString += AgentInterface->GetAgentName();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Participant '%s' does not implement ITAAgentInterface."), *Participant->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("One of the participants is null in DialogueComponent."));
		}
	}

	return ParticipantsNamesString;
}