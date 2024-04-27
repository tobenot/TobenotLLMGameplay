// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.
// FunctionInvoke功能
// 解析大模型回复里面的func_invoke并调用相应的函数
// 和FunctionCall不一样的是，这里的函数不需要返还给大模型，只管触发就行了！
/*
    prompt示例：
    "Please use the following JSON template for your response:"
        "Response Template: {"
            "..."
            "// Optionally add func_invokes if functions need to be triggered"
            "\"func_invoke\": ["
                "{"
                "\"name\": \"TriggerBattle\","
                "\"depict\": \"Ice Mushroom Monster\""
                "},"
                "// Include additional function calls in the array as required"
                "]"
            "}"
        "func_invoke library:"
        "- \"TriggerBattle\": Use when the player is about to enter into a combat scenario. The \"depict\" should describe enemies for the fight."
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "TAFunctionInvokeComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFunctionInvoke, Log, All);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOBENOTLLMGAMEPLAY_API UTAFunctionInvokeComponent : public UActorComponent
{
    GENERATED_BODY()
    
public:	
    // Constructors and other Unreal methods omitted for brevity...
    
    UTAFunctionInvokeComponent();

    // 解析func_invoke并调用相应的函数
    UFUNCTION(BlueprintCallable, Category = "FunctionInvoke")
    void ParseAndTriggerFunctions(const FString& Response);

protected:
    // Helper function to deserialize a JSON string into a JSON object
    bool TryDeserializeJson(const FString& Response, TSharedPtr<FJsonObject>& OutJsonObject);

    // Helper function to handle an individual function invoke
    void HandleFunctionInvoke(const TSharedPtr<FJsonValue>& FuncInvoke);
    
    // 在这里添加对应的函数执行操作。注意命名与你的具体游戏逻辑上的函数名保持一致。
    UFUNCTION()
    virtual void TriggerBattle(const FString& Depict);
    
    UFUNCTION()
    virtual void GiveItem(const FString& Depict);

    UFUNCTION()
    virtual void FinishEvent(const FString& Depict);

    UFUNCTION()
    void RequestShoutCompToSpeak(const FString& Message);
};