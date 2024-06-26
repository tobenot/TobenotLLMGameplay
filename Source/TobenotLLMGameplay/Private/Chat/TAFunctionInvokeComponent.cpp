// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "Chat/TAFunctionInvokeComponent.h"
#include "Chat/TAChatLogCategory.h"
#include "Chat/Shout/TAShoutComponent.h"
#include "Event/Core/TAEventSubsystem.h"

DEFINE_LOG_CATEGORY(LogFunctionInvoke);

UTAFunctionInvokeComponent::UTAFunctionInvokeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UTAFunctionInvokeComponent::TryDeserializeJson(const FString& Response, TSharedPtr<FJsonObject>& OutJsonObject)
{
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);
    return FJsonSerializer::Deserialize(Reader, OutJsonObject) && OutJsonObject.IsValid();
}

void UTAFunctionInvokeComponent::HandleFunctionInvoke(const TSharedPtr<FJsonValue>& FuncInvoke)
{
    FString FunctionName;
    FString Depict;
    if (FuncInvoke->AsObject()->TryGetStringField(TEXT("name"), FunctionName) &&
        FuncInvoke->AsObject()->TryGetStringField(TEXT("depict"), Depict))
    {
        UFunction* FunctionToCall = this->FindFunction(FName(*FunctionName));
        if (FunctionToCall)
        {
            UE_LOG(LogFunctionInvoke, Log, TEXT("FunctionInvoke: calling function %s , depict: %s."), *FunctionName, *Depict);
            this->ProcessEvent(FunctionToCall, &Depict);
        }
        else
        {
            UE_LOG(LogFunctionInvoke, Error, TEXT("FunctionInvoke: function %s does not exist. depict: %s."), *FunctionName, *Depict);
        }
    }else
    {
        UE_LOG(LogFunctionInvoke, Error, TEXT("FunctionInvoke: invaild function name or depict."));
    }
}

void UTAFunctionInvokeComponent::ParseAndTriggerFunctions(const FString& Response)
{
    TSharedPtr<FJsonObject> JsonObject;
    if (TryDeserializeJson(Response, JsonObject))
    {
        const TArray<TSharedPtr<FJsonValue>>* FuncInvokes;
        if (JsonObject->TryGetArrayField(TEXT("func_invoke"), FuncInvokes))
        {
            for (const TSharedPtr<FJsonValue>& FuncInvoke : *FuncInvokes)
            {
                UE_LOG(LogFunctionInvoke, Log, TEXT("FunctionInvoke: Call HandleFunctionInvoke"));
                HandleFunctionInvoke(FuncInvoke);
            }
        }
        else
        {
            // Try parsing as a single object instead of an array
            const TSharedPtr<FJsonObject>* FuncInvokeObject;
            if (JsonObject->TryGetObjectField(TEXT("func_invoke"), FuncInvokeObject))
            {
                UE_LOG(LogFunctionInvoke, Log, TEXT("FunctionInvoke: Call HandleFunctionInvoke"));
                HandleFunctionInvoke(MakeShareable(new FJsonValueObject(*FuncInvokeObject)));
            }
            else
            {
                UE_LOG(LogFunctionInvoke, Verbose, TEXT("FunctionInvoke: 'func_invoke' field is neither an array nor a valid object."));
            }
        }
    }
    else
    {
        UE_LOG(LogFunctionInvoke, Warning, TEXT("FunctionInvoke: JSON deserialization failed."));
    }
}

// 实现触发战斗的逻辑
void UTAFunctionInvokeComponent::TriggerBattle(const FString& Depict)
{
    UE_LOG(LogFunctionInvoke, Warning, TEXT("FunctionInvoke: TriggerBattle! Depict: %s"), *Depict);
    // TODO: 根据Depict实现具体的触发战斗逻辑
}

// 实现给玩家物品的逻辑
void UTAFunctionInvokeComponent::GiveItem(const FString& Depict)
{
    UE_LOG(LogFunctionInvoke, Warning, TEXT("FunctionInvoke: GiveItemToPlayer! Depict: %s"), *Depict);
}

void UTAFunctionInvokeComponent::FinishEvent(const FString& Depict)
{
    // 使用空格作为分隔符来从Depict分解出事件ID和结果ID
    TArray<FString> TmpArray;
    Depict.ParseIntoArray(TmpArray, TEXT(" "));
    
    if (TmpArray.Num() > 1)
    {
        int32 ParsedEventID = FCString::Atoi(*TmpArray[0]);
        int32 ParsedOutcomeID = FCString::Atoi(*TmpArray[1]);

        UE_LOG(LogFunctionInvoke, Log, TEXT("FunctionInvoke: FinishEvent called with EventID: %d, OutcomeID: %d"), ParsedEventID, ParsedOutcomeID);
        
        // 获取事件子系统实例
        UTAEventSubsystem* EventSubsystem = GetWorld()->GetSubsystem<UTAEventSubsystem>();

        if (EventSubsystem)
        {
            EventSubsystem->FinishEvent(ParsedEventID, ParsedOutcomeID);
            RequestShoutCompToSpeak("Event finished, you should say one more sentence.");
        }
        else
        {
            UE_LOG(LogFunctionInvoke, Error, TEXT("FunctionInvoke: Unable to retrieve EventSubsystem."));
        }
    }
    else
    {
        UE_LOG(LogFunctionInvoke, Error, TEXT("FunctionInvoke: Failed to parse EventID and OutcomeID from Depict."));
    }
}

void UTAFunctionInvokeComponent::SpecialEvent(const FString& Depict)
{
    OnSpecialEvent.Broadcast(Depict);
}

void UTAFunctionInvokeComponent::RequestShoutCompToSpeak(const FString& Message)
{
    UTAShoutComponent* ShoutComponent = GetOwner()->FindComponentByClass<UTAShoutComponent>();
    if(ShoutComponent)
    {
        FChatCompletion ChatCompletion;
        ChatCompletion.message.role = EOAChatRole::SYSTEM;
        ChatCompletion.message.content = Message;
        ShoutComponent->UpdateShoutHistory(ChatCompletion);
        ShoutComponent->RequestToSpeakCheckSurrounding();
    }
}
