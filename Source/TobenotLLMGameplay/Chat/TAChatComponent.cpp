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
    IsGettingAPIMessage = false;
}

// Called when the game starts
void UTAChatComponent::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void UTAChatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    ActorChatHistoryMap.Empty();
}

void UTAChatComponent::SendMessageToOpenAI(AActor* OriActor, FString UserMessage, UTAChatCallback* CallbackObject, bool IsSystemMessage)
{
    if (IsGettingAPIMessage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Another message is being processed. Please wait."));
        return;
    }
    if (!OriActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("OriActor is nullptr"));
        return;
    }

    IsGettingAPIMessage = true;
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
    IsGettingAPIMessage = false;
}

void UTAChatComponent::HandleFailedMessage()
{
    OnMessageFailed.Broadcast();
    IsGettingAPIMessage = false;
}