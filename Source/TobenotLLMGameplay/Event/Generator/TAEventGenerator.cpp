// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// TAEventGenerator.cpp

#include "TAEventGenerator.h"

#include "OpenAIDefinitions.h"
#include "Chat/TAChatCallback.h"
#include "Common/TALLMLibrary.h"
#include "Common/TASystemLibrary.h"
#include "Event/TAEventLogCategory.h"
#include "Event/Data/TAEventInfo.h"

void UTAEventGenerator::RequestEventGeneration(const FString& SceneInfo)
{
	InitPrompt();
	TArray<FChatLog> TempMessagesList;
	const FString SystemPrompt = UTALLMLibrary::PromptToStr(PromptGenerateEvent)
		.Replace(TEXT("{SceneInfo}"), *SceneInfo)
		.Replace(TEXT("{Language}"), *UTASystemLibrary::GetGameLanguage())
		;
	TempMessagesList.Add({EOAChatRole::SYSTEM, SystemPrompt});
	FChatSettings ChatSettings{
		UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
		TempMessagesList
	};
	ChatSettings.jsonFormat = true;

	// 创建回调对象并注册成功和失败委托
	UTAChatCallback* CallbackObject = NewObject<UTAChatCallback>();
	CacheCallbackObject = CallbackObject;
	CallbackObject->OnSuccess.AddDynamic(this, &UTAEventGenerator::OnChatSuccess);
	CallbackObject->OnFailure.AddDynamic(this, &UTAEventGenerator::OnChatFailure);
    
	// 异步发送消息
	UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings, [this](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
	{
		if (Success)
		{
			CacheCallbackObject->OnSuccess.Broadcast(Message);
		}
		else
		{
			CacheCallbackObject->OnFailure.Broadcast();
		}
	});
}

void UTAEventGenerator::OnChatSuccess(FChatCompletion ChatCompletion)
{
	// 解析返回的消息
	TArray<FTAEventInfo> GeneratedEvents = ParseEventsFromJson(ChatCompletion.message.content);
    
	// 触发成功事件
	OnEventGenerationSuccess.Broadcast(GeneratedEvents);
}

void UTAEventGenerator::OnChatFailure()
{
	// 可以在此处处理失败的逻辑，例如重试或者提供错误信息反馈
}

TArray<FTAEventInfo> UTAEventGenerator::ParseEventsFromJson(const FString& JsonString)
{
	TArray<FTAEventInfo> ParsedEvents;
	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		// 获取事件数组
		TArray<TSharedPtr<FJsonValue>> EventsArray = JsonObject->GetArrayField(TEXT("Events"));

		for (int32 Index = 0; Index < EventsArray.Num(); Index++)
		{
			TSharedPtr<FJsonObject> EventObject = EventsArray[Index]->AsObject();
			if (EventObject.IsValid())
			{
				FTAEventInfo EventInfo;
            
				// 获取并设置地点名称
				EventInfo.LocationName = EventObject->GetStringField(TEXT("LocationName"));

				// 获取并设置事件描述
				EventInfo.Description = EventObject->GetStringField(TEXT("Description"));

				// 获取并设置事件类型
				int32 EventTypeInt = EventObject->GetIntegerField(TEXT("EventType"));
				EventInfo.EventType = static_cast<ETAEventType>(EventTypeInt);

				// 获取并设置事件权重
				EventInfo.Weight = EventObject->GetIntegerField(TEXT("Weight"));

				// 添加到结果数组中
				ParsedEvents.Add(EventInfo);
			}
		}
	}
	else
	{
		UE_LOG(LogTAEventSystem, Error, TEXT("事件生成 JSON字符串解析失败 %s"), *JsonString);
	}

	return ParsedEvents;
}