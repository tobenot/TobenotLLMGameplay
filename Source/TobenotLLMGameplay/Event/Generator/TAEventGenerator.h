// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TobenotLLMGameplay/Common/TAPromptDefinitions.h"
#include "TAEventGenerator.generated.h"

struct FChatCompletion;
class UTAChatCallback;
struct FTAEventInfo;

// 事件完成的委托声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTAEventGenerationSuccessDelegate, const TArray<FTAEventInfo>&, GeneratedEvents);
UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAEventGenerator : public UObject
{
	GENERATED_BODY()

	virtual void InitPrompt();
		
public:
	UPROPERTY(BlueprintAssignable)
	FTAEventGenerationSuccessDelegate OnEventGenerationSuccess;

	// 使用地理人文信息生成事件
	UFUNCTION(BlueprintCallable, Category = "Event|Generation")
	void RequestEventGeneration(const FString& SceneInfo);
	
protected:
	FTAPrompt PromptGenerateEvent;
	
	// 当大模型处理完成后调用
	UFUNCTION()
	void OnChatSuccess(FChatCompletion ChatCompletion);

	// 当大模型处理失败时调用
	UFUNCTION()
	void OnChatFailure();
	
private:
	// 解析大模型返回的JSON字符串并转换为事件数组
	TArray<FTAEventInfo> ParseEventsFromJson(const FString& JsonString);

	UTAChatCallback* CacheCallbackObject;
};
