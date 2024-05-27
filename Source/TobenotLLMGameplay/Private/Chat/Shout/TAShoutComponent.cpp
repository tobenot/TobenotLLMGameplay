// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "Chat/Shout/TAShoutComponent.h"

#include "OpenAIChat.h"
#include "Chat/Shout/TAShoutManager.h"
#include "OpenAIDefinitions.h"
#include "Chat/TAFunctionInvokeComponent.h"
#include "Agent/TAAgentInterface.h"
#include "Common/TALLMLibrary.h"
#include "Chat/TAChatLogCategory.h"
#include "Save/TAGuidInterface.h"

UTAShoutComponent::UTAShoutComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	IsPlayer = false;
	IsRequestingMessage = false;
}

void UTAShoutComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UTAShoutManager* ShoutManager = GetWorld()->GetSubsystem<UTAShoutManager>();
	if (ShoutManager)
	{
		ShoutManager->RegisterShoutComponent(this);
	}

	ShoutHistory.Add(FChatLog{EOAChatRole::SYSTEM,""});
	
	SetActive(true);
}

void UTAShoutComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(DelayRequestToSpeakTimerHandle);
	UTAShoutManager* ShoutManager = GetWorld()->GetSubsystem<UTAShoutManager>();
	if (ShoutManager)
	{
		ShoutManager->UnregisterShoutComponent(this);
	}
}

void UTAShoutComponent::ShoutMessage(const FChatCompletion& Message, float Volume)
{
	if (GetWorld())
	{
		UTAShoutManager* ShoutManager = GetWorld()->GetSubsystem<UTAShoutManager>();
		if (ShoutManager)
		{
			ShoutManager->BroadcastShout(Message, GetOwner(), Volume);
		}
	}
}

FString UTAShoutComponent::GetNearbyAgentNames()
{
    FString NearbyAgentNames;

    // 获取当前Agent的名字
    FString CurrentAgentName;
    const ITAAgentInterface* CurrentAgentInterface = Cast<ITAAgentInterface>(GetOwner());
    if (CurrentAgentInterface)
    {
        CurrentAgentName = CurrentAgentInterface->GetAgentName();
    }

    // 获取声音可以达到的Agent
    UTAShoutManager* ShoutManager = GetWorld()->GetSubsystem<UTAShoutManager>();
    if (ShoutManager)
    {
        TArray<UTAShoutComponent*> NearbyAgentComponents = ShoutManager->GetShoutComponentsInRange(GetOwner(), 700.f);
       
        // 遍历Agent，获取他们的名字以及特定的IdentityPositionName
        for (UTAShoutComponent* AgentComponent : NearbyAgentComponents)
        {
            if (AgentComponent && AgentComponent->GetOwner())
            {
                const ITAAgentInterface* AgentInterface = Cast<ITAAgentInterface>(AgentComponent->GetOwner());
                const ITAGuidInterface* GuidInterface = Cast<ITAGuidInterface>(AgentComponent->GetOwner());

                if (AgentInterface && GuidInterface)
                {
                    FString AgentName = AgentInterface->GetAgentName();
                    const FName& IdentityPositionName = GuidInterface->GetIdentityPositionName();
                    
                    // 判断IdentityPositionName是否是"player's partner"或"player"，并在名字后面追加描述
                    if (IdentityPositionName == "player's partner" || IdentityPositionName == "player")
                    {
                        AgentName += " (" + IdentityPositionName.ToString() + ")";
                    }

                    // 如果AgentName是当前AgentName，则在后面追加"(你)"
                    if (AgentName == CurrentAgentName)
                    {
                        AgentName += " (You)";
                    }
                    
                    NearbyAgentNames += AgentName + ", ";
                }
            }
        }
    }
   
    // 如果存在列表，移除最后添加的", "，否则返回"No nearby agents."
    return NearbyAgentNames.IsEmpty() ? "No nearby agents" : NearbyAgentNames.LeftChop(2);
}

void UTAShoutComponent::RequestToSpeak()
{
	if(IsPlayer)
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	// 检查是否已经通过间隔限制时间
	if (CurrentTime - LastRequestToSpeakTimestamp < RequestToSpeakInterval)
	{
		// 如果未到间隔时间且未设置计时器，则设置计时器等待剩余时间
		if(!GetWorld()->GetTimerManager().IsTimerActive(DelayRequestToSpeakTimerHandle))
		{
			float DelayTime = RequestToSpeakInterval - (CurrentTime - LastRequestToSpeakTimestamp);
			GetWorld()->GetTimerManager().SetTimer(DelayRequestToSpeakTimerHandle, this, &UTAShoutComponent::RequestToSpeak, DelayTime, false);
		}
		return;
	}
	
	if (IsPartner)
	{
		if(!bIsDelayRequestToSpeakTimerFinished)
		{
			GetWorld()->GetTimerManager().ClearTimer(DelayRequestToSpeakTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				DelayRequestToSpeakTimerHandle, this, &UTAShoutComponent::ContinueRequestToSpeak,
				PartnerDelayRequestToSpeakTime, false);
			return;
		}else
		{
			bIsDelayRequestToSpeakTimerFinished = false;
		}
	}
	
	// 如果通过了间隔时间检查和说话的延迟，则继续后续的RequestToSpeak操作
	LastRequestToSpeakTimestamp = GetWorld()->GetTimeSeconds(); // 更新时间戳
	
	// 打断之前没说完的话，因为现在有新信息入手了！
	if(CacheChat)
	{
		CacheChat->CancelRequest();
	}
	IsRequestingMessage = true;
	UE_LOG(LogTAChat, Log, TEXT("[%s] RequestToSpeak called"), *GetOwner()->GetName());
	auto& TempMessagesList = ShoutHistory;
	// 构造系统提示的ChatLog对象
	const FString SystemPrompt = GetSystemPromptFromOwner()
		+ "Your long-term memory: " + ShoutHistoryCompressedStr + "."
		//+ ". Output {\"no_response_needed\": \"No response needed because [Your_Reason_Here], and explain when you will speak again\"} when dialogue is becoming rubbish."
		//+ (IsPartner ? " or not point at you. ":". ")
		+ "Nearby agents: " + GetNearbyAgentNames();
	
	const FChatLog SystemPromptLog{EOAChatRole::SYSTEM, SystemPrompt};

	if(!TempMessagesList.Num() || TempMessagesList[0].role != EOAChatRole::SYSTEM)
	{
		TempMessagesList.Insert(SystemPromptLog,0);
	}
	else if(TempMessagesList[0].role == EOAChatRole::SYSTEM)
	{
		TempMessagesList[0] = SystemPromptLog;
	}
	
	FChatSettings ChatSettings{
		UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
		TempMessagesList,
		0
	};
	ChatSettings.jsonFormat = true;

	CacheChat = UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings, [this](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
	{
		if (Success)
		{
			UE_LOG(LogTAChat, Log, TEXT("[%s] SendMessageToOpenAIWithRetry Success"), *GetOwner()->GetName());
			if (Message.message.content.Contains(TEXT("no_response_needed")))
			{
				// 如果不需要回应，则不继续喊话
				return;
			}

			ShoutMessage(Message, 700); // 继续喊话
			if (bEnableFunctionInvoke)
			{
				PerformFunctionInvokeBasedOnResponse(Message.message.content);
			}
		}
		else
		{
			UE_LOG(LogTAChat, Log, TEXT("[%s] SendMessageToOpenAIWithRetry Fail: %s"), *GetOwner()->GetName(), *ErrorMessage);
			//RefuseToSay();
		}
		IsRequestingMessage = false;
		CacheChat = nullptr;
	},GetOwner());
}

void UTAShoutComponent::ContinueRequestToSpeak()
{
	bIsDelayRequestToSpeakTimerFinished = true;
	RequestToSpeak();
}

void UTAShoutComponent::UpdateShoutHistory(const FChatCompletion& NewChatCompletion)
{
	ShoutHistory.Add(NewChatCompletion.message);
	FullShoutHistory.Add(NewChatCompletion.message);
	
	if (bEnableCompressShout && NewChatCompletion.totalTokens > 2500)
	{
		RequestShoutCompression();
	}
}

void UTAShoutComponent::RequestShoutCompression()
{
	// Implement your compression logic here. As an example, you might:
	// - Aggregate similar messages
	// - Remove older messages from the history
	// - Summarize certain parts of the Shout
	if(bIsCompressingShout)
	{
		return;
	}
	bIsCompressingShout = true;
	LastCompressedIndex = ShoutHistory.Num() - 1;
	LastCompressedIndex -= 3;
	if(LastCompressedIndex < 1)
	{
		LastCompressedIndex = 1;
	}
	const FString ShoutHistoryString = ShoutHistoryCompressedStr + JoinShoutHistory();

	// Prepare the chat message to send to OpenAI for Shout compression
	TArray<FChatLog> TempMessagesList;
	const FString FormattedPrompt = UTALLMLibrary::PromptToStr(PromptCompressShoutHistory);
	TempMessagesList.Add({EOAChatRole::SYSTEM, FormattedPrompt});
	TempMessagesList.Add({EOAChatRole::USER, ShoutHistoryString});

	FChatSettings ChatSettings{
		UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
		TempMessagesList,
		0.8
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
			if (ShoutHistory.Num() > LastCompressedIndex + 1)
			{
				NewHistory.Append(ShoutHistory.GetData() + LastCompressedIndex + 1, ShoutHistory.Num() - LastCompressedIndex - 1);
			}
		        
			ShoutHistory.Empty();
			ShoutHistoryCompressedStr = Message.message.content;
		        
			// 将新历史记录加回到当前对话历史
			ShoutHistory.Append(NewHistory);

			UE_LOG(LogTAChat, Log, TEXT("Shout compression successful: %s"), *Message.message.content);
		}
		else
		{
			UE_LOG(LogTAChat, Error, TEXT("Shout compression failed: %s"), *ErrorMessage);
		}
		bIsCompressingShout = false;
		},GetOwner());
}

FString UTAShoutComponent::JoinShoutHistory()
{
	FString Result;
	for (const FChatLog& LogEntry : ShoutHistory)
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

void UTAShoutComponent::HandleShoutReceived(const FChatCompletion& Message, AActor* Shouter, float Volume)
{
	if(LastMessageContent == Message.message.content)
	{
		return; //重复消息当没收到，不然会引发更大的问题
	}
	LastMessageContent = Message.message.content;
	if (Shouter == GetOwner())
	{
		HandleReceivedMessage(Message, Shouter);
		return;
	}
	// 计算距离并根据声音衰减处理逻辑
	float Distance = Shouter->GetDistanceTo(GetOwner());
    
	// 假设有一个声音衰减模型来确定是否应该处理喊话
	if (Volume <= 0 || Distance < Volume)//ShouldHandleShout(Distance)) // ShouldHandleShout 为自定义逻辑函数
	{
		HandleReceivedMessage(Message, Shouter);

		// 这里可以有一些硬性条件，比如角色是否死亡，是否能说话
		if(true)
		{
			if (IsPlayer) 
			{
				// 是玩家，所以请求提供回复选项
				RequestChoices();
			}else
			{
				// 不是玩家，看看要不要回复
				RequestToSpeak();
			}
		}
	}
}

void UTAShoutComponent::HandleReceivedMessage(const FChatCompletion& ReceivedMessage, AActor* Sender)
{
	// 更新对话历史记录
	UpdateShoutHistory(ReceivedMessage);

	// 向UI通知更新
	NotifyUIOfShoutHistoryUpdate(ReceivedMessage, Sender);
}

void UTAShoutComponent::NotifyUIOfShoutHistoryUpdate(const FChatCompletion& ReceivedMessage, AActor* Sender)
{
	OnShoutReceivedMessage.Broadcast(ReceivedMessage, Sender);
}

FString UTAShoutComponent::GetSystemPromptFromOwner() const
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

void UTAShoutComponent::PerformFunctionInvokeBasedOnResponse(const FString& Response)
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

void UTAShoutComponent::SetAcceptMessages(bool bInAcceptMessages)
{
	bAcceptMessages = bInAcceptMessages;
}

UTAShoutComponent* UTAShoutComponent::GetTAShoutComponent(AActor* Actor)
{
	if (Actor)
	{
		// Try to find the component directly on the Actor first
		UTAShoutComponent* ShoutComponent = Actor->FindComponentByClass<UTAShoutComponent>();
		if (ShoutComponent)
		{
			return ShoutComponent;
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

	UTAShoutComponent* ShoutComponent = PlayerController->FindComponentByClass<UTAShoutComponent>();
	if (ShoutComponent)
	{
		return ShoutComponent;
	}

	// If the PlayerController doesn't have the component, try to get it from the Controller's Pawn
	APawn* ControlledPawn = PlayerController->GetPawn();
	if (ControlledPawn)
	{
		ShoutComponent = ControlledPawn->FindComponentByClass<UTAShoutComponent>();
	}

	return ShoutComponent;
}


void UTAShoutComponent::RequestChoices()
{
	// 暂时不要这个功能
	return;
	
	auto& TempMessagesList = ShoutHistory;
	//使用系统提示创建ChatLog对象
	/*const FString SystemPrompt = GetSystemPromptFromOwner()
		+ "But now your task is different. Now you need to know that the above information is player information. "
		+ "Please provide several choices for the player's next speech based on the dialogue history. The choices should come in a JSON string array format as shown below: "
		+ "{\"message\" : [\"You too!\", \"I don't think so.\", \"I love it.\"]}."
		+ "Please stand in the player's shoes, but don't need to output player's character's name before choices.";*/
	const FString SystemPrompt =
	TEXT(
		"Please provide several choices for the player's next speech based on the dialogue history. The choices should come in a JSON string array format as shown below: "
		"{\"message\" : [\"You too!\", \"I don't think so.\", \"I love it.\"]}."
		"Please stand in the player's shoes, but don't need to output player's character's name before choices."
		);
	const FChatLog SystemPromptLog{EOAChatRole::SYSTEM, SystemPrompt};

	// 将刚刚创建的系统提示设置为TempMessagesList的首个元素
	if (TempMessagesList.Num() > 0)
	{
		TempMessagesList[0] = SystemPromptLog;
	}
	else
	{
		TempMessagesList.Add(SystemPromptLog);
	}
	
	FChatSettings ChatSettings{
		UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
		TempMessagesList,
		0
	};
	ChatSettings.jsonFormat = true;

	//发送请求
	//CacheChat =
	UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings, [this](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
	{
		if (Success)
		{
			//按照某种定义的JSON格式解析回复选项
			TArray<FString> Choices = ParseChoicesFromResponse(Message.message.content);
            
			// 委托广播备选回复
			OnProvidePlayerChoices.Broadcast(Choices);
		}
	},GetOwner());
}

TArray<FString> UTAShoutComponent::ParseChoicesFromResponse(const FString& Response)
{
	// 接受的json格式下示例 {"choices" : ["Hello", "How are you", "Nice to meet you"]}
	TArray<FString> Choices;
	TSharedPtr<FJsonObject> JsonObject;

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		if (JsonObject->HasField("message"))
		{
			TArray<TSharedPtr<FJsonValue>> ChoicesJsonArray = JsonObject->GetArrayField("message");
			for (int32 i = 0; i < ChoicesJsonArray.Num(); i++)
			{
				Choices.Add(ChoicesJsonArray[i]->AsString());
			}
		}
	}
        
	return Choices;
}

void UTAShoutComponent::RequestToSpeakCheckSurrounding()
{
	UTAShoutManager* ShoutManager = GetWorld()->GetSubsystem<UTAShoutManager>();
	if (!ShoutManager) return;
	float ShoutRange = 700.f;
	TArray<UTAShoutComponent*> NearbyShoutComponents = ShoutManager->GetShoutComponentsInRange(GetOwner(), ShoutRange);
	if (NearbyShoutComponents.Num() > 1 || (NearbyShoutComponents.Num() == 1 && NearbyShoutComponents[0] != this))
	{
		RequestToSpeak();
	}
}

const FTAPrompt UTAShoutComponent::PromptCompressShoutHistory = FTAPrompt{
	"In the adventure game application, my message list token has exceeded the limit, I need to compress a bit, "
	"but keep the important memories, especially the more recent information, "
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