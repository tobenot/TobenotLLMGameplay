// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TADialogueComponent.h"
#include "Chat/Dialogue/TADialogueInstance.h"
#include "Chat/Dialogue/TADialogueManager.h"
#include "OpenAIDefinitions.h"
#include "Common/TAAgentInterface.h"
#include "Common/TALLMLibrary.h"
#include "Common/TASystemLibrary.h"

UTADialogueComponent::UTADialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	IsPlayer = false;
	IsRequestingMessage = false;
}

void UTADialogueComponent::BeginPlay()
{
	Super::BeginPlay();
	DialogueHistory.Add(FChatLog{EOAChatRole::SYSTEM,""});
	//DialogueHistory.Add(FChatLog{EOAChatRole::SYSTEM,""});//留两个系统prompt
}


void UTADialogueComponent::SetCurrentDialogue(UTADialogueInstance* NewDialogueInstance)
{
	CurrentDialogueInstance = NewDialogueInstance;
}

void UTADialogueComponent::SendMessageToDialogue(const FString& UserMessage)
{
	if (CurrentDialogueInstance != nullptr)
	{
		// 添加聊天记录到对话实例
		CurrentDialogueInstance->ReceiveMessage({EOAChatRole::USER, UserMessage}, GetOwner());
	}
	else
	{
		// 可以适当提供反馈
		UE_LOG(LogTemp, Warning, TEXT("No dialogue instance is set or messages are not being accepted."));
	}
}

void UTADialogueComponent::RequestToSpeak()
{
	if(!CurrentDialogueInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("No dialogue instance is set"));
		return;
	}
	IsRequestingMessage = true;
	if(IsPlayer)
	{
		return;
	}
	auto& TempMessagesList = DialogueHistory;
	// 构造系统提示的ChatLog对象
	FString SystemPrompt = GetSystemPromptFromOwner() + FString::Printf(
		TEXT("Now you are in a group conversation with [%s], please speak according to the conversation history in [%s].")
		, *CurrentDialogueInstance->GetParticipantsNamesStringFromAgents(), *UTASystemLibrary::GetGameLanguage());
	const FChatLog SystemPromptLog{EOAChatRole::SYSTEM, };

	/*const FChatLog DialogueLog{EOAChatRole::SYSTEM, FString::Printf(
		TEXT("Now you are in a group conversation with [%s], please speak according to the conversation history.")
		, *CurrentDialogueInstance->GetParticipantsNamesStringFromAgents())};*/
	
	// 设置系统提示为TempMessagesList的首个元素
	if (TempMessagesList.Num() > 0)
	{
		TempMessagesList[0] = SystemPromptLog;
	}
	else
	{
		TempMessagesList.Add(SystemPromptLog);
	}
	/*if (TempMessagesList.Num() > 1)
	{
		TempMessagesList[1] = DialogueLog;
	}
	else
	{
		TempMessagesList.Add(DialogueLog);
	}*/
	
	// 设置对话请求的配置
	FChatSettings ChatSettings{
		UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
		TempMessagesList,
		true // 使用JSON格式
	};

	CacheChat = UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings, [this](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
	{
		if (Success)
		{
			SendMessageToDialogue(Message.message.content);
		}
		else
		{
			RefuseToSay();
		}
		IsRequestingMessage = false;
		CacheChat = nullptr;
	},GetOwner());
}

void UTADialogueComponent::RefuseToSay()
{
	if (CurrentDialogueInstance != nullptr)
	{
		// 添加聊天记录到对话实例
		CurrentDialogueInstance->RefuseToSay(GetOwner());
	}
}

void UTADialogueComponent::UpdateDialogueHistory(const FChatLog& NewChatLog)
{
	// 将新的聊天记录添加到组件内部的聊天历史记录中
	DialogueHistory.Add(NewChatLog);
}

void UTADialogueComponent::HandleReceivedMessage(const FChatLog& ReceivedMessage, AActor* Sender)
{
	// 更新对话历史记录
	UpdateDialogueHistory(ReceivedMessage);

	// 向UI通知更新
	NotifyUIOfDialogueHistoryUpdate(ReceivedMessage, Sender);
}

void UTADialogueComponent::NotifyUIOfDialogueHistoryUpdate(const FChatLog& ReceivedMessage, AActor* Sender)
{
	OnDialogueReceivedMessage.Broadcast(ReceivedMessage, Sender);
}

FString UTADialogueComponent::GetSystemPromptFromOwner() const
{
	ITAAgentInterface* MyAgentInterface = Cast<ITAAgentInterface>(GetOwner());
	if (MyAgentInterface)
	{
		return MyAgentInterface->GetSystemPrompt();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("我的Owner 没有实现 ITAAgentInterface"));
		return "";
	}
}

UTADialogueComponent* UTADialogueComponent::GetTADialogueComponent(AActor* Actor)
{
	if (Actor)
	{
		// Try to find the component directly on the Actor first
		UTADialogueComponent* DialogueComponent = Actor->FindComponentByClass<UTADialogueComponent>();
		if (DialogueComponent)
		{
			return DialogueComponent;
		}
	}

	// If not found on Actor, or Actor is null, try to get it from the Pawn's controller
	AController* PlayerController = Cast<AController>(Actor);
	if (!PlayerController)
	{
		APawn* Pawn = Cast<APawn>(Actor);
		if (!Pawn) return nullptr;
		PlayerController = Pawn->GetController();
	}
    
	if (!PlayerController) return nullptr;

	UTADialogueComponent* DialogueComponent = PlayerController->FindComponentByClass<UTADialogueComponent>();
	if (DialogueComponent)
	{
		return DialogueComponent;
	}

	// If the PlayerController doesn't have the component, try to get it from the Controller's Pawn
	APawn* ControlledPawn = PlayerController->GetPawn();
	if (ControlledPawn)
	{
		DialogueComponent = ControlledPawn->FindComponentByClass<UTADialogueComponent>();
	}

	return DialogueComponent;
}
