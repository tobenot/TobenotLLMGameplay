#include "Mechanism/TAGameMasterSubsystem.h"
#include "Agent/TAAgentComponent.h"
#include "Chat/Shout/TAShoutManager.h"
#include "Common/TALLMLibrary.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UTAGameMasterSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UTAGameMasterSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UTAGameMasterSubsystem::ReceiveDecisionFromAgent(AActor* AgentActor, const FString& Decision)
{
    if (!AgentActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("无效的Agent Actor"));
        return;
    }
    
    FString AgentName = AgentActor->GetName();
    UE_LOG(LogTemp, Log, TEXT("%s 提交了决策: %s"), *AgentName, *Decision);

    // 异步裁定合理性
    AdjudicateDecisionAsync(AgentActor, Decision);
}

void UTAGameMasterSubsystem::AdjudicateDecisionAsync(const AActor* AgentActor, const FString& Decision)
{
    if (!AgentActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("无效的Agent Actor"));
        return;
    }

    FString AgentName = AgentActor->GetName();

    // 获取 FunctionInvokeSubsystem
    UTAFunctionInvokeSubsystem* FunctionInvokeSubsystem = GetWorld()->GetSubsystem<UTAFunctionInvokeSubsystem>();
    if (!FunctionInvokeSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("无法找到 UTAFunctionInvokeSubsystem"));
        return;
    }

    FString FunctionDescriptions = FunctionInvokeSubsystem->GetAllFunctionDescriptions();

    // 构造实际的大模型请求数据
    FString SystemPrompt = FString::Printf(
    TEXT(
        R""""(
        请判断以下决策是否合理，并以JSON格式回复包括以下字段：
        {
        "description": "对决策的详细说明", 
        "explanation": "主持人对发起者角色私下的说明",
        "is_valid": 布尔值（是否合理）,
        "event_description": "对实际发生的事件描述",
        "function_calls": [
            {
                "name": "调用的函数名",
                "depict": "函数调用说明"
            }
        ]
        }。

        如果合理，请详细说明并向角色二次确认是否要这么做。
        如果不合理，请具体说明不合理之处并要求其重新决策。

        函数说明:
        %s

        Agent %s 的决策是：%s
        )""""),
        *FunctionDescriptions,
        *AgentName, *Decision);

    TArray<FChatLog> TempMessagesList;
    TempMessagesList.Add(FChatLog{EOAChatRole::SYSTEM, SystemPrompt});

    // 设置 ChatSettings，包括引擎质量、历史记录等
    FChatSettings ChatSettings{
        UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
        TempMessagesList,
        0
    };
    ChatSettings.jsonFormat = true;

    // 发送请求
    UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings, [this, AgentActor, Decision](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
    {
        if (Success)
        {
            this->OnAdjudicateDecisionCompleted(AgentActor, Decision, Message.message.content);
        }
        else
        {
            this->OnAdjudicateDecisionCompleted(AgentActor, Decision, FString::Printf(TEXT("{\"is_valid\": false, \"description\": \"Error: %s\", \"explanation\": \"模型请求失败，请重新尝试决策。\"}"), *ErrorMessage));
        }
    }, this);
}

void UTAGameMasterSubsystem::ReturnDecisionToAgent(AActor* AgentActor, const FString& Outcome, bool bIsDecisionValid)
{
    if (!AgentActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("无效的Agent Actor"));
        return;
    }
    
    FString AgentName = AgentActor->GetName();
    UE_LOG(LogTemp, Log, TEXT("发送结果给 %s: %s"), *AgentName, *Outcome);

    UTAAgentComponent* AgentComponent = AgentActor->FindComponentByClass<UTAAgentComponent>();
    if (AgentComponent)
    {
        AgentComponent->HandleDecisionResponse(Outcome, bIsDecisionValid);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("%s 没有找到相应的 UTAAgentComponent"), *AgentName);
    }
}

void UTAGameMasterSubsystem::ReceiveDialogueGenerated(AActor* AgentActor, const FString& Dialogue)
{
    if (!AgentActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("无效的Agent Actor"));
        return;
    }
    
    // 动态数组存储函数调用信息
    TArray<FFunctionInvokeInfo> EmptyFunctionCalls;
    DistributeDialogueAndInvokeMechanisms(AgentActor, Dialogue, EmptyFunctionCalls);
}

void UTAGameMasterSubsystem::ExecuteGameMechanism(const AActor* AgentActor, const FString& Decision)
{
    if (!AgentActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("无效的Agent Actor"));
        return;
    }

    FString AgentName = AgentActor->GetName();
    UE_LOG(LogTemp, Log, TEXT("执行 %s 的决策: %s"), *AgentName, *Decision);
}

void UTAGameMasterSubsystem::OnAdjudicateDecisionCompleted(const AActor* AgentActor, const FString& Decision, const FString& JSONResponse)
{
    if (!AgentActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("无效的Agent Actor"));
        return;
    }

    FString Description;
    FString EventDescription;
    bool bDecisionValid = false;
    TArray<FFunctionInvokeInfo> FunctionCalls;

    if (ParseDecisionResponse(JSONResponse, Description, EventDescription, bDecisionValid, FunctionCalls))
    {
        if (bDecisionValid)
        {
            // 获取 FunctionInvokeSubsystem
            UTAFunctionInvokeSubsystem* FunctionInvokeSubsystem = GetWorld()->GetSubsystem<UTAFunctionInvokeSubsystem>();
            if (!FunctionInvokeSubsystem)
            {
                UE_LOG(LogTemp, Warning, TEXT("无法找到 UTAFunctionInvokeSubsystem"));
                return;
            }

            // 生成函数调用描述
            FString FunctionDescription = FunctionInvokeSubsystem->GenerateFunctionDescription(FunctionCalls);
            FString Explanation = FString::Printf(TEXT("决策合理: %s。实际发生的事件是: %s。包含的额外改变有：%s。请确认你是否想继续。"), *Description, *EventDescription, *FunctionDescription);
            FString FullOutcome = FString::Printf(TEXT("%s 回应: %s"), *Decision, *Explanation);
            ReturnDecisionToAgent(const_cast<AActor*>(AgentActor), FullOutcome, true);
        }
        else
        {
            FString Explanation = Description; // 直接使用解释原因
            FString Outcome = FString::Printf(TEXT("决策被判定为不合理: %s, 请重新给出决策"), *Explanation);
            ReturnDecisionToAgent(const_cast<AActor*>(AgentActor), Outcome, false);
        }
    }
    else
    {
        FString Outcome = FString::Printf(TEXT("解析大模型回复时出错: %s"), *JSONResponse);
        ReturnDecisionToAgent(const_cast<AActor*>(AgentActor), Outcome, false);
    }
}

bool UTAGameMasterSubsystem::ParseDecisionResponse(const FString& JSONResponse, FString& Description, FString& EventDescription, bool& bIsValid, TArray<FFunctionInvokeInfo>& FunctionCalls)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONResponse);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        if (JsonObject->HasField("description") && JsonObject->HasField("is_valid") && JsonObject->HasField("event_description") && JsonObject->HasField("explanation"))
        {
            Description = JsonObject->GetStringField("description");
            EventDescription = JsonObject->GetStringField("event_description");
            bIsValid = JsonObject->GetBoolField("is_valid");

            // 解析函数调用列表
            const TArray<TSharedPtr<FJsonValue>>* Functions;
            if (JsonObject->TryGetArrayField("function_calls", Functions))
            {
                for (const auto& FunctionValue : *Functions)
                {
                    const TSharedPtr<FJsonObject>* FunctionObject;
                    if (FunctionValue->TryGetObject(FunctionObject))
                    {
                        FString FunctionName = FunctionObject->Get()->GetStringField("name");
                        FString Params = FunctionObject->Get()->GetStringField("params");
                        FunctionCalls.Add(FFunctionInvokeInfo(FunctionName, Params));
                    }
                }
            }

            return true;
        }
    }

    return false;
}

void UTAGameMasterSubsystem::DistributeDialogueAndInvokeMechanisms(AActor* AgentActor, const FString& Dialogue, const TArray<FFunctionInvokeInfo>& FunctionCalls)
{
    if (!AgentActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("无效的Agent Actor"));
        return;
    }

    // 获取 ShoutManager 并分发对话
    UTAShoutManager* ShoutManager = GetWorld()->GetSubsystem<UTAShoutManager>();
    if (ShoutManager)
    {
        FChatCompletion Message;
        Message.message.content = Dialogue;
        ShoutManager->BroadcastShout(Message, AgentActor, 700.f);
    }

    // 获取 FunctionInvokeSubsystem
    UTAFunctionInvokeSubsystem* FunctionInvokeSubsystem = GetWorld()->GetSubsystem<UTAFunctionInvokeSubsystem>();
    if (!FunctionInvokeSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("无法找到 UTAFunctionInvokeSubsystem"));
        return;
    }

    // 调用游戏机制
    for (const auto& FunctionCall : FunctionCalls)
    {
        FunctionInvokeSubsystem->InvokeFunction(FunctionCall.FunctionName, AgentActor, FunctionCall.Params);
    }

    // 记录事件（根据你的事件记录系统，可能需要调用不同的方法）
    UE_LOG(LogTemp, Log, TEXT("记录事件: %s 的操作已被调用"), *Dialogue);
}