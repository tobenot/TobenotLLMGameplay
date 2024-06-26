// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Common/TAPromptDefinitions.h"
#include "TAEventGenerator.generated.h"

struct FChatCompletion;
class UTAChatCallback;
struct FTAEventInfo;

// 事件完成的委托声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTAEventGenerationSuccessDelegate, TArray<FTAEventInfo>&, GeneratedEvents);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTAEventGenerationSuccessInLocationDelegate, TArray<FTAEventInfo>&, GeneratedEvents, const FVector&, InLocation);

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAEventGenerator : public UObject
{
	GENERATED_BODY()

protected:
	virtual void InitPrompt();
		
public:
	UPROPERTY(BlueprintAssignable)
	FTAEventGenerationSuccessDelegate OnEventGenerationSuccess;

	UPROPERTY(BlueprintAssignable)
	FTAEventGenerationSuccessInLocationDelegate OnEventGenerationSuccessInLocation;

	// 使用地理人文信息生成事件
	UFUNCTION(BlueprintCallable, Category = "Event|Generation")
	void RequestEventGeneration(const FString& SceneInfo, const int32& Num);

	UFUNCTION(BlueprintCallable, Category = "Event|Generation")
	void RequestEventGenerationByDescription(const FString& SceneInfo, const FString& Description, const FVector& InLocation);
	
protected:
	FTAPrompt PromptGenerateEvent;

	FTAPrompt PromptGenerateEventByDescription;
	
	// 当大模型处理完成后调用
	UFUNCTION()
	void OnChatSuccess(FChatCompletion ChatCompletion);

	// 当大模型处理失败时调用
	UFUNCTION()
	void OnChatFailure();
	
private:
	// 解析大模型返回的JSON字符串并转换为事件数组
	TArray<FTAEventInfo> ParseEventsFromJson(const FString& JsonString);
	
	void ProcessEventObject(const TSharedPtr<FJsonObject>& EventObject, TArray<FTAEventInfo>& ParsedEvents);
	
	UPROPERTY()
	UTAChatCallback* CacheCallbackObject;

	UPROPERTY()
	class UOpenAIChat* CacheChat;

	bool IsInLocation = false;
	FVector GenerateInLocation;
};
