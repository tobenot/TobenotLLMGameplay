// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "Chat/TAChatComponent.h"
#include "OpenAIChat.h"
#include "OpenAIDefinitions.h"
#include "Chat/TAChatCallback.h"
#include "Chat/TAFunctionInvokeComponent.h"
#include "Agent/TAAgentInterface.h"
#include "Save/TAGuidInterface.h"
#include "Common/TALLMLibrary.h"
#include "Chat/TAChatLogCategory.h"

// Sets default values for this component's properties
UTAChatComponent::UTAChatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UTAChatComponent::BeginPlay()
{
    Super::BeginPlay();
}

UTAChatComponent* UTAChatComponent::GetTAChatComponent(AActor* Actor)
{
    if (Actor)
    {
        // Try to find the component directly on the Actor first
        UTAChatComponent* ChatComponent = Actor->FindComponentByClass<UTAChatComponent>();
        if (ChatComponent)
        {
            return ChatComponent;
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

    UTAChatComponent* ChatComponent = PlayerController->FindComponentByClass<UTAChatComponent>();
    if (ChatComponent)
    {
        return ChatComponent;
    }

    // If the PlayerController doesn't have the component, try to get it from the Controller's Pawn
    APawn* ControlledPawn = PlayerController->GetPawn();
    if (ControlledPawn)
    {
        ChatComponent = ControlledPawn->FindComponentByClass<UTAChatComponent>();
    }

    return ChatComponent;
}

// Called every frame
void UTAChatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    // ActorChatHistoryMap.Empty();
}

void UTAChatComponent::SendMessageToOpenAI(AActor* OriActor, FString UserMessage, UTAChatCallback* CallbackObject, bool IsSystemMessage)
{
    if (!OriActor)
    {
        UE_LOG(LogTAChat, Warning, TEXT("OriActor is nullptr"));
        return;
    }

    // 如果当前正在处理该Actor的消息，将新消息添加到队列中
    if (ActiveActors.Contains(OriActor))
    {
        FTAActorMessageQueue& ActorQueue = ActorMessageQueueMap.FindOrAdd(OriActor);
        ActorQueue.MessageQueue.Add(UserMessage);
        ActorQueue.CallbackObject = CallbackObject;
        UE_LOG(LogTAChat, Warning, TEXT("Another message is being processed for this Actor. Your message has been added to the queue."));
        return;
    }

    // 如果没有正在处理的消息，开始处理新消息
    ProcessMessage(OriActor, UserMessage, CallbackObject, IsSystemMessage);
}

void UTAChatComponent::ProcessMessage(AActor* OriActor, const FString& UserMessage, UTAChatCallback* CallbackObject, bool IsSystemMessage)
{
    if (!OriActor)
    {
        UE_LOG(LogTAChat, Warning, TEXT("OriActor is nullptr"));
        return;
    }
    if(!bAcceptMessages)
    {
        UE_LOG(LogTAChat, Log, TEXT("Reject message from %s"), *OriActor->GetName());
        return;
    }
    
    ActiveActors.Add(OriActor);
    CallbackMap.Add(OriActor, CallbackObject);
    CallbackObject->OnSuccessWithSender.AddDynamic(this, &UTAChatComponent::HandleSuccessfulMessage);
    CallbackObject->OnFailure.AddDynamic(this, &UTAChatComponent::HandleFailedMessage);

    auto& TempMessagesList = GetChatHistoryWithActor(OriActor);
    // 构造系统提示的ChatLog对象
    const FChatLog SystemPromptLog{EOAChatRole::SYSTEM, GetSystemPromptFromOwner()};
    
    // 设置系统提示为TempMessagesList的首个元素
    if(!TempMessagesList.Num() || TempMessagesList[0].role != EOAChatRole::SYSTEM)
    {
        TempMessagesList.Insert(SystemPromptLog,0);
    }
    else if(TempMessagesList[0].role == EOAChatRole::SYSTEM)
    {
        TempMessagesList[0] = SystemPromptLog;
    }
    
    // 构造用户消息的ChatLog对象并添加到TempMessagesList
    TempMessagesList.Add({EOAChatRole::USER, UserMessage});
    
    FChatSettings ChatSettings{UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast), TempMessagesList};
    ChatSettings.jsonFormat = ChatMessageJsonFormat;

    CacheChat = UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings, [this,OriActor](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
    {
        //消息处理前移出激活，因为可能会连续激活
        ActiveActors.Remove(OriActor);
        if (Success)
        {
            auto& TempMessagesList = GetChatHistoryWithActor(OriActor);
            TempMessagesList.Add({EOAChatRole::ASSISTANT, Message.message.content});
            if(CallbackMap.Contains(OriActor))
            {
                CallbackMap[OriActor]->OnSuccess.Broadcast(Message);
                CallbackMap[OriActor]->OnSuccessWithSender.Broadcast(Message,OriActor);
            }
            CallbackMap.Remove(OriActor);
        }
        else
        {
            if(CallbackMap.Contains(OriActor))
            {
                CallbackMap[OriActor]->OnFailure.Broadcast();
            }
            CallbackMap.Remove(OriActor);
        }
        CacheChat = nullptr;
    },this->GetOwner());
}

FString UTAChatComponent::GetSystemPromptFromOwner() const
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

TArray<FChatLog>& UTAChatComponent::GetChatHistoryWithActor(AActor* OtherActor)
{
    // 使用接口获取Actor实例
    ITAGuidInterface* GuidInterface = Cast<ITAGuidInterface>(OtherActor);
    if (!GuidInterface)
    {
        UE_LOG(LogTAChat, Error, TEXT("%s 未实现ITAGuidInterface"),*OtherActor->GetName());
        ensure(GuidInterface != nullptr);
    }
    FGuid TAGuid = GuidInterface->GetTAGuid();
    return ActorChatHistoryMap.FindOrAdd(TAGuid).ChatHistory;
}

void UTAChatComponent::ClearChatHistoryWithActor(AActor* OtherActor)
{
    ITAGuidInterface* GuidInterface = Cast<ITAGuidInterface>(OtherActor);
    if (GuidInterface)
    {
        FGuid TAGuid = GuidInterface->GetTAGuid();
        ActorChatHistoryMap.Remove(TAGuid);
    }else
    {
        UE_LOG(LogTAChat, Error, TEXT("%s 未实现ITAGuidInterface"),*OtherActor->GetName());
    }
    CallbackMap.Remove(OtherActor);
}

void UTAChatComponent::HandleSuccessfulMessage(FChatCompletion Message, AActor* Sender)
{
    OnMessageSent.Broadcast(Message);
    OnMessageSentWithSender.Broadcast(Message, Sender);
    // 尝试进行 FunctionInvoke
    if (bEnableFunctionInvoke)
    {
        PerformFunctionInvokeBasedOnResponse(Message.message.content);
    }
    // 检查是否有待处理的消息
    CheckMessageQueue();
}

void UTAChatComponent::HandleFailedMessage()
{
    OnMessageFailed.Broadcast();

    // 检查是否有待处理的消息
    CheckMessageQueue();
}

void UTAChatComponent::CheckMessageQueue()
{
    for (auto& Elem : ActorMessageQueueMap)
    {
        // 如果当前正在处理该Actor的消息，跳过
        if (ActiveActors.Contains(Elem.Key))
            continue;

        if (Elem.Value.MessageQueue.Num() > 0)
        {
            FString NextMessage = Elem.Value.MessageQueue[0];
            Elem.Value.MessageQueue.RemoveAt(0);

            // 处理下一条消息
            ProcessMessage(Elem.Key, NextMessage, Elem.Value.CallbackObject, false);
            break;
        }
    }
}

void UTAChatComponent::PerformFunctionInvokeBasedOnResponse(const FString& Response)
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

void UTAChatComponent::SetChatHistoryData(const FTAChatComponentSaveData& NewData)
{
    ActorChatHistoryMap = NewData.SavedActorChatHistoryMap;
}

FTAChatComponentSaveData UTAChatComponent::GetChatHistoryData() const
{
    FTAChatComponentSaveData ChatHistoryData;
    ChatHistoryData.SavedActorChatHistoryMap = ActorChatHistoryMap;
    return ChatHistoryData;
}

void UTAChatComponent::SetAcceptMessages(bool bInAcceptMessages)
{
    bAcceptMessages = bInAcceptMessages;
}