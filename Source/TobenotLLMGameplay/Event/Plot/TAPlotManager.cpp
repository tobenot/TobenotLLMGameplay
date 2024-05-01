// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAPlotManager.h"
#include "OpenAIDefinitions.h"
#include "TAEmbeddingSystem.h"
#include "TAEventLogCategory.h"
#include "TALLMLibrary.h"
#include "Core/TAEventInstance.h"

class UTAEmbeddingSystem;

UTAPlotManager::UTAPlotManager()
{
}

UTAPlotManager::~UTAPlotManager()
{
}

void UTAPlotManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTAPlotManager::Deinitialize()
{
	Super::Deinitialize();
}

void UTAPlotManager::ReportEventTagGroups(const FTATagGroup& EventTagGroups)
{
	PlotTagGroups.Add(EventTagGroups);
}

void UTAPlotManager::CheckEventsTagGroupCondition(TArray<FTAEventInfo*> Events)
{
	// 获取 第一个事件的前置标签组 数组 的长度
	// Events[0]->PresetData.PrecedingPlotTagGroups.Num()
	// 这个是布尔值
	// Events[0]->PrecedingPlotTagGroupsConditionMet = false
	
    // 获取词嵌管理器
	UTAEmbeddingSystem* EmbeddingSystem = GetWorld()->GetGameInstance()->GetSubsystem<UTAEmbeddingSystem>();
    for (FTAEventInfo* EventInfo : Events)
    {
        if(EventInfo->PrecedingPlotTagGroupsConditionMet)
        {
            // 跳过已满足条件的事件
        	UE_LOG(LogTAEventSystem, Warning, TEXT("事件 '%s' 在之前的检测中已经满足网状叙事前置"), *EventInfo->PresetData.EventName);
            continue;
        }
    	if(!EventInfo->PresetData.PrecedingPlotTagGroups.Num())
    	{
    		// 无前置，直接满足
    		UE_LOG(LogTAEventSystem, Warning, TEXT("事件 '%s' 无网状叙事前置，直接满足条件"), *EventInfo->PresetData.EventName);
    		EventInfo->PrecedingPlotTagGroupsConditionMet = true;
    		continue;
    	}
        
        bool bAllGroupConditionsMet = true;
        
        for (const FTATagGroup& PresetGroup : EventInfo->PresetData.PrecedingPlotTagGroups)
		{
		    bool bCurrentGroupConditionMet = false;

		    for (const FTATagGroup& PlotGroup : PlotTagGroups)
		    {
		        // 初始化标志位和索引，用于遍历每个前置紧接标签
		        bool bGroupMatchFound = true; // 假设在这个PlotGroup中可以找到完整的PresetGroup
		        int32 LastFoundIndex = -1;

		        for (const FName& PresetTag : PresetGroup.Tags)
		        {
		            FHighDimensionalVector PresetTagEmbedding;
		            if (!EmbeddingSystem->GetTagEmbedding(PresetTag, PresetTagEmbedding))
		            {
		                bGroupMatchFound = false;
		                UE_LOG(LogTAEventSystem, Warning, TEXT("预设标签 '%s' 的嵌入向量尚未找到，无法匹配"), *PresetTag.ToString());
		                break; // 跳出当前PresetTag循环，因为嵌入未找到
		            }

		            bool bMatchingTagFound = false;
		            // 从上一次找到的索引之后开始搜索
		            for (int32 PlotTagIndex = LastFoundIndex + 1; PlotTagIndex < PlotGroup.Tags.Num(); ++PlotTagIndex)
		            {
		                const FName& PlotTag = PlotGroup.Tags[PlotTagIndex];
		                FHighDimensionalVector PlotTagEmbedding;
		                if(EmbeddingSystem->GetTagEmbedding(PlotTag, PlotTagEmbedding))
		                {
		                    float Similarity = UTAEmbeddingSystem::CalculateCosineSimilarity(PresetTagEmbedding, PlotTagEmbedding);
		                	UE_LOG(LogTAEventSystem, Warning,
								TEXT("当前的预设前置 '%s' 与剧情标签 '%s' 的嵌入向量余弦相似度为：%f"),
								*PresetTag.ToString(), *PlotTag.ToString(), Similarity);
		                	
		                    if (Similarity > 0.7f)
		                    {
		                        bMatchingTagFound = true;
		                        LastFoundIndex = PlotTagIndex - 1; // 更新最后找到匹配的索引位置，当前的继续匹配也行
		                        break; // 找到匹配的标签，退出PlotGroup中标签的循环
		                    }
		                }
		            }

		            if(!bMatchingTagFound)
		            {
		                bGroupMatchFound = false; // 当前PresetTag在PlotGroup中没有找到匹配项
		                break; // 跳出PresetGroup中标签的循环
		            }
		        }

		        if(bGroupMatchFound)
		        {
		            bCurrentGroupConditionMet = true; // 在这个PlotGroup中找到了完整的PresetGroup按顺序匹配
		            break; // 找到一个满足条件的PlotGroup，跳出PlotGroup的循环
		        }
		    }

		    if (!bCurrentGroupConditionMet)
		    {
		        bAllGroupConditionsMet = false; // 当前事件中有一个PresetGroup没有匹配成功
		        break; // 提前跳出PresetGroup的循环
		    }
		}

        // 设置事件实例的条件满足标志
        EventInfo->PrecedingPlotTagGroupsConditionMet = bAllGroupConditionsMet;
        UE_LOG(LogTAEventSystem, Warning, TEXT("事件 '%s' 的前置标签组条件是否满足: %s"), *EventInfo->PresetData.EventName, bAllGroupConditionsMet ? TEXT("是") : TEXT("否"));
    }
}

void UTAPlotManager::ParseNewEventToTagGroups()
{
	if(!ShoutHistory.Num())
	{
		return;
	}
	/*TArray<FChatLog> TempMessagesList = ShoutHistory;
	const FString FormattedPrompt = UTALLMLibrary::PromptToStr(PromptTagEvent);
	TempMessagesList.Insert({EOAChatRole::SYSTEM, FormattedPrompt}, 0);*/
	TArray<FChatLog> TempMessagesList = ShoutHistory;
	TempMessagesList.Insert({EOAChatRole::SYSTEM, UTALLMLibrary::PromptToStr(PromptTagEvent)}, 0);
	
	FChatSettings ChatSettings{
		UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
		TempMessagesList,
		0.8
	};
	ChatSettings.jsonFormat = PromptTagEvent.bUseJsonFormat;

	UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings,
		[this](const FChatCompletion& Message, const FString& ErrorMessage, bool bWasSuccessful)
		{
			if(bWasSuccessful)
			{
				// 转换返回的字符串为Json
				TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Message.message.content);
				TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

				if(FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
				{
					// 获取事件字符串
					FString Event = JsonObject->GetStringField("event");

					// 获取标签数组
					TArray<TSharedPtr<FJsonValue>> TagsJsonArray = JsonObject->GetArrayField("tags");

					// 将标签转为FName格式，并存入TagGroup结构体中
					FTATagGroup TagGroup;
					for (auto& JsonValue : TagsJsonArray)
					{
						FName TagName = FName(*JsonValue->AsString());
						TagGroup.Tags.Add(TagName);
					}

					// 将TagGroup加入PlotTagGroups
					PlotTagGroups.Add(TagGroup);
				}
				else
				{
					UE_LOG(LogTAEventSystem, Warning, TEXT("解析JSON失败: %s"), *Message.message.content);
				}
			}
			else
			{
				 //打印错误信息
				 UE_LOG(LogTAEventSystem, Error, TEXT("请求失败: %s"), *ErrorMessage);
			}
		},GetWorld());
}


bool UTAPlotManager::GetTagEmbeddingsFromSystem(const FName& Tag, FHighDimensionalVector& OutEmbeddingVec)
{
	return GetWorld()->GetGameInstance()->GetSubsystem<UTAEmbeddingSystem>()->GetTagEmbedding(Tag,OutEmbeddingVec);
}

void UTAPlotManager::ProcessShoutInGame(const FChatCompletion& Message, AActor* Shouter, float Volume)
{
	ShoutHistory.Add(Message.message);
	if(ShoutHistory.Num() > 5)
	{
		ShoutHistory.RemoveAt(0);
	}
	ParseNewEventToTagGroups();
	return;
	// 存储接收到的消息，在FullShoutHistory中始终保留完整记录
	FullShoutHistory.Add(Message.message);

	// 更新压缩后的ShoutHistory
	ShoutHistory.Add(Message.message);
  
	// 触发消息压缩逻辑（如果适用）
	if (bEnableCompressShout && Message.totalTokens > 2200)
	{
		RequestShoutCompression();
	}

	ParseNewEventToTagGroups();
}

void UTAPlotManager::RequestShoutCompression()
{
	// Implement your compression logic here. As an example, you might:
	// - Aggregate similar messages
	// - Remove older messages from the history
	// - Summarize certain parts of the Shout
	if(bIsCompressingShout)
	{
		return;
	}
	bIsCompressingShout = true;
	LastCompressedIndex = ShoutHistory.Num() - 1;
	LastCompressedIndex -= 3;
	if(LastCompressedIndex < 1)
	{
		LastCompressedIndex = 1;
	}
	const FString ShoutHistoryString = ShoutHistoryCompressedStr + JoinShoutHistory();

	// Prepare the chat message to send to OpenAI for Shout compression
	TArray<FChatLog> TempMessagesList;
	const FString FormattedPrompt = UTALLMLibrary::PromptToStr(PromptCompressShoutHistory);
	TempMessagesList.Add({EOAChatRole::SYSTEM, FormattedPrompt});
	TempMessagesList.Add({EOAChatRole::USER, ShoutHistoryString});

	FChatSettings ChatSettings{
		UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
		TempMessagesList,
		0.8
	};
	ChatSettings.jsonFormat = true;

	// Send the request asynchronously
	UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings, 
	[this](const FChatCompletion& Message, const FString& ErrorMessage, bool bWasSuccessful)
		{
		if(bWasSuccessful)
		{
			// 在清空当前对话历史之前，保留新的历史记录
			TArray<FChatLog> NewHistory;
			if (ShoutHistory.Num() > LastCompressedIndex + 1)
			{
				NewHistory.Append(ShoutHistory.GetData() + LastCompressedIndex + 1, ShoutHistory.Num() - LastCompressedIndex - 1);
			}
		        
			ShoutHistory.Empty();
			ShoutHistoryCompressedStr = Message.message.content;
		        
			// 将新历史记录加回到当前对话历史
			ShoutHistory.Append(NewHistory);

			UE_LOG(LogTAEventSystem, Log, TEXT("Shout compression successful: %s"), *Message.message.content);
		}
		else
		{
			UE_LOG(LogTAEventSystem, Error, TEXT("Shout compression failed: %s"), *ErrorMessage);
		}
		bIsCompressingShout = false;
		},GetWorld());
}

FString UTAPlotManager::JoinShoutHistory()
{
	FString Result;
	for (const FChatLog& LogEntry : ShoutHistory)
	{
		if(LogEntry.role != EOAChatRole::SYSTEM)
		{
			Result += LogEntry.content + TEXT(" ");
		}
	}
	// Trim and remove any excess whitespace if necessary
	Result = Result.TrimStartAndEnd();
	return Result;
}

const FTAPrompt UTAPlotManager::PromptCompressShoutHistory = FTAPrompt{
	"In the adventure game application, my message list token has exceeded the limit, I need to compress a bit, "
	"but keep the important memories, especially the more recent information, "
	"while unimportant information can be appropriately deleted. "
	"You do not have to maintain the alternating response format of user and system, "
	"you only need to output a summary in a descriptive manner. "
	"Write your summary in English. "
	"You need to summarize extremely briefly, only retaining the most essential key words and sentences. "
	"Use the following JSON format for your responses: "
	"{ "
	"\"compress_history\": \"...\", "
	"} "
	"Please extract the important content from the following message sent by USER."
	,1
	,true
};

// 这个不能热更
const FTAPrompt UTAPlotManager::PromptTagEvent = FTAPrompt{
	"Please analyze the narrative content of the provided game event and generate a structured set of tags."
	"These tags should sequentially reflect the key initiator (person/object), the specific action taken, and the individual/object affected or the specific item involved."
	"Your task is to format the response as a JSON array, with each element representing a distinct event or action sequence and an associated set of tags."
	"Each element should contain an 'event' key to describe the event, and a 'tags' key with an array of tag strings arranged in the order of \"initiator (person/object)\", \"specific action\", \"affected individual/object or specific item involved\"."
	"The response should be in the following JSON format:"
	"If there is a significant event:"
	"   {"
	"       \"event\": \"Event description\", "
	"       \"tags\": [\"Initiator (person full name/object)\", \"Specific action (accept mission, using prop)\", \"Related object (mission name), specific item (prop name) or individual (someone's full name) \"]"
	"   }"
	"Please record what actually happened, for example, if A only asked B to do something and B didn't actually do it, then you should not record B doing it. Instead, you should record \"A\", \"asking to do something\", \"B\"."
		"Example 1:"
		"{"
		"\"event\": \"John Smith has accepted the quest from Sarah Johnson.\","
		"\"tags\": [\"John Smith\", \"accept mission\", \"The Hidden Treasure of the Silent Woods\"]"
		"}"
		"Example 2:"
		"{"
		"\"event\": \"Using the mystical Lantern of Lore, Sarah Johnson revealed the secret entrance in the ancient ruins.\","
		"\"tags\": [\"Sarah Johnson\", \"using\", \"Lantern of Lore\"]"
		"}"
		"Example 3:"
		"{"
		"\"event\": \"The enchanted sword Excaliburn was stolen by the thief Martin Black from the royal armory.\","
		"\"tags\": [\"Martin Black\", \"stole\", \"Excaliburn\"]"
		"}"
		"Example 4:"
		"{"
		"\"event\": \"Sarah Johnson attack John Smith.\","
		"\"tags\": [\"Sarah Johnson\", \"attack\", \"John Smith\"]"
		"}"
	"Please focus only on the last message as it represents the newly occurring event. You have already processed what happened before. Showing them now is only for maintaining the context without loss."
	"Please respond All in Chinese, as it matches our system requirements."
	"DO NOT respond tags in English!"
	"Proceed with the extraction and tagging as directed for the last event of following game event:",
	1,
	true
};