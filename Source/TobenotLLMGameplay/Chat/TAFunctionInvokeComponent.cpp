// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAFunctionInvokeComponent.h"

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
                HandleFunctionInvoke(FuncInvoke);
            }
        }
        else
        {
            // Try parsing as a single object instead of an array
            const TSharedPtr<FJsonObject>* FuncInvokeObject;
            if (JsonObject->TryGetObjectField(TEXT("func_invoke"), FuncInvokeObject))
            {
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
    // TODO: 根据Depict实现具体的给予物品逻辑
}