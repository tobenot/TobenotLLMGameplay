// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "Chat/Dialogue/TADialogueComponent.h"

#include "Chat/Dialogue/TADialogueInstance.h"
#include "Chat/Dialogue/TADialogueManager.h"
#include "OpenAIDefinitions.h"
#include "Chat/TAFunctionInvokeComponent.h"
#include "Agent/TAAgentInterface.h"
#include "Common/TALLMLibrary.h"
#include "Common/TASystemLibrary.h"
#include "Chat/TAChatLogCategory.h"

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

void UTADialogueComponent::SendMessageToDialogue(const FChatCompletion& Message)
{
	if (CurrentDialogueInstance != nullptr)
	{
		// 添加聊天记录到对话实例
		CurrentDialogueInstance->ReceiveMessage(Message, GetOwner());
	}
	else
	{
		// 可以适当提供反馈
		UE_LOG(LogTAChat, Warning, TEXT("No dialogue instance is set or messages are not being accepted."));
	}
}

void UTADialogueComponent::RequestToSpeak()
{
	if(!CurrentDialogueInstance)
	{
		UE_LOG(LogTAChat, Warning, TEXT("No dialogue instance is set"));
		return;
	}
	IsRequestingMessage = true;
	if(IsPlayer)
	{
		return;
	}
	auto& TempMessagesList = DialogueHistory;
	// 构造系统提示的ChatLog对象
	const FString SystemPrompt = FString::Printf(
		TEXT(
			"Now you are in a group conversation with [%s], You remember the conversation just now and the one before, which summary as [%s], please speak according to the conversation history."
			)
		, *CurrentDialogueInstance->GetParticipantsNamesStringFromAgents(), *DialogueHistoryCompressedStr) + GetSystemPromptFromOwner();
	const FChatLog SystemPromptLog{EOAChatRole::SYSTEM, SystemPrompt};

	/*const FChatLog DialogueLog{EOAChatRole::SYSTEM, FString::Printf(
		TEXT("Now you are in a group conversation with [%s], please speak according to the conversation history.")
		, *CurrentDialogueInstance->GetParticipantsNamesStringFromAgents())};*/
	
	// 设置系统提示为TempMessagesList的首个元素
	if(!TempMessagesList.Num() || TempMessagesList[0].role != EOAChatRole::SYSTEM)
	{
		TempMessagesList.Insert(SystemPromptLog,0);
	}
	else if(TempMessagesList[0].role == EOAChatRole::SYSTEM)
	{
		TempMessagesList[0] = SystemPromptLog;
	}
	
	// 设置对话请求的配置
	FChatSettings ChatSettings{
		UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
		TempMessagesList,
		0.8,
	};
	ChatSettings.jsonFormat = true;
	
	CacheChat = UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings, [this](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
	{
		if (Success)
		{
			SendMessageToDialogue(Message);
			if (bEnableFunctionInvoke)
			{
				PerformFunctionInvokeBasedOnResponse(Message.message.content);
			}
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
		CurrentDialogueInstance->RefuseToSay(GetOwner());
	}
}

void UTADialogueComponent::UpdateDialogueHistory(const FChatCompletion& NewChatCompletion)
{
	DialogueHistory.Add(NewChatCompletion.message);
	FullDialogueHistory.Add(NewChatCompletion.message);
	
	if (bEnableCompressDialogue && NewChatCompletion.totalTokens > 2400)
	{
		RequestDialogueCompression();
	}
}

void UTADialogueComponent::RequestDialogueCompression()
{
	// Implement your compression logic here. As an example, you might:
	// - Aggregate similar messages
	// - Remove older messages from the history
	// - Summarize certain parts of the dialogue
	if(bIsCompressingDialogue)
	{
		return;
	}
	bIsCompressingDialogue = true;
	LastCompressedIndex = DialogueHistory.Num() - 1;
	LastCompressedIndex -= 3;
	if(LastCompressedIndex < 1)
	{
		LastCompressedIndex = 1;
	}
	const FString DialogueHistoryString = DialogueHistoryCompressedStr + JoinDialogueHistory();

	// Prepare the chat message to send to OpenAI for dialogue compression
	TArray<FChatLog> TempMessagesList;
	const FString FormattedPrompt = UTALLMLibrary::PromptToStr(PromptCompressDialogueHistory);
	TempMessagesList.Add({EOAChatRole::SYSTEM, FormattedPrompt});
	TempMessagesList.Add({EOAChatRole::USER, DialogueHistoryString});

	FChatSettings ChatSettings{
		UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
		TempMessagesList
	};
	ChatSettings.jsonFormat = true;

	// Send the request asynchronously
	UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings, 
	[this](const FChatCompletion& Message, const FString& ErrorMessage, bool bWasSuccessful)
		{
		if(bWasSuccessful)
		{
			// 在清空当前对话历史之前，保留新的历史记录
			TArray<FChatLog> NewHistory;
			if (DialogueHistory.Num() > LastCompressedIndex + 1)
			{
				NewHistory.Append(DialogueHistory.GetData() + LastCompressedIndex + 1, DialogueHistory.Num() - LastCompressedIndex - 1);
			}
		        
			DialogueHistory.Empty();
			DialogueHistoryCompressedStr = Message.message.content;
		        
			// 将新历史记录加回到当前对话历史
			DialogueHistory.Append(NewHistory);

			UE_LOG(LogTAChat, Log, TEXT("Dialogue compression successful: %s"), *Message.message.content);
		}
		else
		{
			UE_LOG(LogTAChat, Error, TEXT("Dialogue compression failed: %s"), *ErrorMessage);
		}
		bIsCompressingDialogue = false;
		},GetOwner());
}

FString UTADialogueComponent::JoinDialogueHistory()
{
	FString Result;
	for (const FChatLog& LogEntry : DialogueHistory)
	{
		if(LogEntry.role != EOAChatRole::SYSTEM)
		{
			Result += LogEntry.content + TEXT(" ");
		}
	}
	// Trim and remove any excess whitespace if necessary
	Result = Result.TrimStartAndEnd();
	return Result;
}

void UTADialogueComponent::HandleReceivedMessage(const FChatCompletion& ReceivedMessage, AActor* Sender)
{
	// 更新对话历史记录
	UpdateDialogueHistory(ReceivedMessage);

	// 向UI通知更新
	NotifyUIOfDialogueHistoryUpdate(ReceivedMessage, Sender);
}

void UTADialogueComponent::NotifyUIOfDialogueHistoryUpdate(const FChatCompletion& ReceivedMessage, AActor* Sender)
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
		UE_LOG(LogTAChat, Error, TEXT("我的Owner 没有实现 ITAAgentInterface"));
		return "";
	}
}

void UTADialogueComponent::PerformFunctionInvokeBasedOnResponse(const FString& Response)
{
	UTAFunctionInvokeComponent* FunctionInvokeComp = GetOwner()->FindComponentByClass<UTAFunctionInvokeComponent>();
    
	if(FunctionInvokeComp)
	{
		// 如果成功获取到组件，则用获得的响应调用ParseAndTriggerFunctions方法
		FunctionInvokeComp->ParseAndTriggerFunctions(Response);
	}
	else
	{
		if (bEnableFunctionInvoke)
		{
			UE_LOG(LogTAChat, Error, TEXT("bEnableFunctionInvoke is true, but UTAFunctionInvokeComponent not found on the Owner of UTAChatComponent."));
		}
	}
}

void UTADialogueComponent::SetAcceptMessages(bool bInAcceptMessages)
{
	bAcceptMessages = bInAcceptMessages;
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

const FTAPrompt UTADialogueComponent::PromptCompressDialogueHistory = FTAPrompt{
	"In the adventure game application, my message list token has exceeded the limit, I need to compress a bit, "
	"but keep the important memories, especially the more recent dialogue information, "
	"while unimportant information can be appropriately deleted. "
	"You do not have to maintain the alternating response format of user and system, "
	"you only need to output a summary in a descriptive manner. "
	"Write your summary in English. "
	"You need to summarize extremely briefly, only retaining the most essential key words and sentences. "
	"Use the following JSON format for your responses: "
	"{ "
	"\"compress_history\": \"...\", "
	"} "
	"Please extract the important content from the following message sent by USER."
	,1
	,true
};