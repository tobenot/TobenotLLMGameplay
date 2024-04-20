// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// TAEventGenerator.cpp

#include "TAEventGenerator.h"

#include "OpenAIDefinitions.h"
#include "Chat/TAChatCallback.h"
#include "Common/TALLMLibrary.h"
#include "Common/TASystemLibrary.h"
#include "Event/TAEventLogCategory.h"
#include "Event/Data/TAEventInfo.h"

void UTAEventGenerator::RequestEventGeneration(const FString& SceneInfo, const int32& Num)
{
	InitPrompt();
	TArray<FChatLog> TempMessagesList;
	FString NumTag;
	if(Num>1)
	{
		NumTag = "// Add {Num} events following the same structure";
		NumTag = NumTag.Replace(TEXT("{Num}"), *FString::FromInt(Num - 1));
	}else
	{
		NumTag = "// only 1 event please";
	}
	const FString SystemPrompt = UTALLMLibrary::PromptToStr(PromptGenerateEvent)
		.Replace(TEXT("{SceneInfo}"), *SceneInfo)
		.Replace(TEXT("{Language}"), *UTASystemLibrary::GetGameLanguage())
		.Replace(TEXT("{NumTag}"), *NumTag)
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
	CacheChat = UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings, [this](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
	{
		if (Success)
		{
			CacheCallbackObject->OnSuccess.Broadcast(Message);
		}
		else
		{
			CacheCallbackObject->OnFailure.Broadcast();
		}
		CacheChat = nullptr;
	},this);
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
				EventInfo.PresetData.LocationName = EventObject->GetStringField(TEXT("LocationName"));

				// 获取并设置事件描述
				EventInfo.PresetData.Description = EventObject->GetStringField(TEXT("Description"));

				int32 EventTypeInt;
				FString EventTypeStr;
				if (EventObject->TryGetNumberField(TEXT("EventType"), EventTypeInt))
				{
					// 处理数值字段
					EventInfo.PresetData.EventType = static_cast<ETAEventType>(EventTypeInt);
				}else if (EventObject->TryGetStringField(TEXT("EventType"), EventTypeStr))
				{
					// 尝试将字符串的第一个字符转换为数字
					TCHAR FirstChar = EventTypeStr[0];
					if (FChar::IsDigit(FirstChar))
					{
						EventTypeInt = FCString::Atoi(*EventTypeStr);
						EventInfo.PresetData.EventType = static_cast<ETAEventType>(EventTypeInt);
					}
					else
					{
						// 处理非数字开始的字符串或其他情况
						UE_LOG(LogTAEventSystem, Error, TEXT("无效的事件类型格式 %s"), *EventTypeStr);
					}
				}
				else
				{
					// 处理既不是字符串也不是数字的情况
					UE_LOG(LogTAEventSystem, Error, TEXT("无EventType字段"));
				}

				// 获取并设置事件权重
				EventInfo.PresetData.Weight = EventObject->GetIntegerField(TEXT("Weight"));
				
				/*if (JsonObject->HasField(TEXT("AdventurePoint")))
				{
					EventInfo.AdventurePoint = JsonObject->GetStringField(TEXT("AdventurePoint"));
				}*/
				
				if (JsonObject->HasField(TEXT("PeculiarPoint")))
				{
					EventInfo.PresetData.PeculiarPoint = JsonObject->GetStringField(TEXT("PeculiarPoint"));
				}
				
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