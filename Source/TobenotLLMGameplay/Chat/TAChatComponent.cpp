// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAChatComponent.h"
#include "OpenAIChat.h"
#include "OpenAIDefinitions.h"
#include "TAChatCallback.h"
#include "Common/TAAgentInterface.h"
#include "Common/TALLMLibrary.h"

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
    ActorChatHistoryMap.Empty();
}

void UTAChatComponent::SendMessageToOpenAI(AActor* OriActor, FString UserMessage, UTAChatCallback* CallbackObject, bool IsSystemMessage)
{
    if (!OriActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("OriActor is nullptr"));
        return;
    }

    // 如果当前正在处理该Actor的消息，将新消息添加到队列中
    if (ActiveActors.Contains(OriActor))
    {
        FTAActorMessageQueue& ActorQueue = ActorMessageQueueMap.FindOrAdd(OriActor);
        ActorQueue.MessageQueue.Add(UserMessage);
        ActorQueue.CallbackObject = CallbackObject;
        UE_LOG(LogTemp, Warning, TEXT("Another message is being processed for this Actor. Your message has been added to the queue."));
        return;
    }

    // 如果没有正在处理的消息，开始处理新消息
    ProcessMessage(OriActor, UserMessage, CallbackObject, IsSystemMessage);
}

void UTAChatComponent::ProcessMessage(AActor* OriActor, FString UserMessage, UTAChatCallback* CallbackObject, bool IsSystemMessage)
{
    if (!OriActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("OriActor is nullptr"));
        return;
    }
    
    ActiveActors.Add(OriActor);
    CacheCallbackObject = CallbackObject;
    CacheCallbackObject->OnSuccess.AddDynamic(this, &UTAChatComponent::HandleSuccessfulMessage);
    CacheCallbackObject->OnFailure.AddDynamic(this, &UTAChatComponent::HandleFailedMessage);

    auto& TempMessagesList = GetChatHistoryWithActor(OriActor);
    // 构造系统提示的ChatLog对象
    const FChatLog SystemPromptLog{EOAChatRole::SYSTEM, GetSystemPromptFromOwner()};
    
    // 设置系统提示为TempMessagesList的首个元素
    if (TempMessagesList.Num() > 0)
    {
        TempMessagesList[0] = SystemPromptLog;
    }
    else
    {
        TempMessagesList.Add(SystemPromptLog);
    }

    // 构造用户消息的ChatLog对象并添加到TempMessagesList
    TempMessagesList.Add({EOAChatRole::USER, UserMessage});
    
    // 打印TempMessagesList的调试信息
    FStringBuilderBase StringBuilder;
    StringBuilder.Append(TEXT("Sending chat messages:\n"));
    for (const FChatLog& ChatEntry : TempMessagesList)
    {
        FString RoleName = UEnum::GetValueAsString(ChatEntry.role);
        StringBuilder.Append(FString::Printf(TEXT("Role: %s, Content: %s\n"), *RoleName, *ChatEntry.content));
    }
    UE_LOG(LogTemp, Warning, TEXT("%s"), StringBuilder.ToString());
    
    FChatSettings ChatSettings{UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast), TempMessagesList};
    ChatSettings.jsonFormat = ChatMessageJsonFormat;
    
    CacheChat = UOpenAIChat::Chat(ChatSettings, [this,OriActor](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
    {
        //消息处理前移出激活，因为可能会连续激活
        ActiveActors.Remove(OriActor);
        if (Success)
        {
            CacheCallbackObject->OnSuccess.Broadcast(Message);
            auto& TempMessagesList = GetChatHistoryWithActor(OriActor);
            TempMessagesList.Add({EOAChatRole::ASSISTANT, Message.message.content});
        }
        else
        {
            CacheCallbackObject->OnFailure.Broadcast();
        }
    });
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
        UE_LOG(LogTemp, Error, TEXT("我的Owner 没有实现 ITAAgentInterface"));
        return "";
    }
}

TArray<FChatLog>& UTAChatComponent::GetChatHistoryWithActor(AActor* OtherActor)
{
    return ActorChatHistoryMap.FindOrAdd(OtherActor).ChatHistory;
}

void UTAChatComponent::ClearChatHistoryWithActor(AActor* OtherActor)
{
    ActorChatHistoryMap.Remove(OtherActor);
}

void UTAChatComponent::HandleSuccessfulMessage(FChatCompletion Message)
{
    OnMessageSent.Broadcast(Message);

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