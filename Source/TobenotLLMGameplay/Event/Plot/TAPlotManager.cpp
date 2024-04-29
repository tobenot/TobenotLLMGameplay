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
        	UE_LOG(LogTemp, Warning, TEXT("事件 '%s' 在之前的检测中已经满足网状叙事前置"), *EventInfo->PresetData.EventName);
            continue;
        }
    	if(!EventInfo->PresetData.PrecedingPlotTagGroups.Num())
    	{
    		// 无前置，直接满足
    		UE_LOG(LogTemp, Warning, TEXT("事件 '%s' 无网状叙事前置，直接满足条件"), *EventInfo->PresetData.EventName);
    		EventInfo->PrecedingPlotTagGroupsConditionMet = true;
    		continue;
    	}
        
        bool bAllGroupConditionsMet = true;
        
        for (const FTATagGroup& PresetGroup : EventInfo->PresetData.PrecedingPlotTagGroups)
        {
            bool bCurrentGroupConditionMet = true;
            // 用于有序匹配的最后发现的索引
            int32 LastFoundIndex = -1;
            
            for (const FName& PresetTag : PresetGroup.Tags)
            {
                FHighDimensionalVector PresetTagEmbedding;
                if(EmbeddingSystem->GetTagEmbedding(PresetTag, PresetTagEmbedding))
                {
                    // 为预设标签找到了词嵌，打印它的名字
                    UE_LOG(LogTemp, Warning, TEXT("预设标签 '%s' 的嵌入向量找到"), *PresetTag.ToString());
                    bool bMatchingTagFound = false;

                    // 现在在我们的剧情标签组中寻找匹配的标签
                    for (int32 PlotIndex = LastFoundIndex + 1; PlotIndex < PlotTagGroups.Num(); ++PlotIndex)
                    {
                        const FTATagGroup& PlotGroup = PlotTagGroups[PlotIndex];
                        for (const FName& PlotTag : PlotGroup.Tags)
                        {
                            FHighDimensionalVector PlotTagEmbedding;
                            if (EmbeddingSystem->GetTagEmbedding(PlotTag, PlotTagEmbedding))
                            {
                            	float Similarity = UTAEmbeddingSystem::CalculateCosineSimilarity(PresetTagEmbedding, PlotTagEmbedding);
                            	UE_LOG(LogTemp, Warning,
                            		TEXT("剧情标签 '%s' 的嵌入向量找到 与当前的预设前置'%s'余弦相似度为：%f"),
                            		*PlotTag.ToString(), *PresetTag.ToString(), Similarity);

                            	if (Similarity > 0.7f)
                            	{
                            		// 找到匹配的标签，更新索引以进行有序检查，并退出内部循环
                            		LastFoundIndex = PlotIndex;
                            		bMatchingTagFound = true;
                            		UE_LOG(LogTemp, Warning, TEXT("找到匹配标签: '%s', 余弦相似度：%f"), *PlotTag.ToString(), Similarity);
                            		break;
                            	}
                            }
                        }
                        if (bMatchingTagFound)
                        {
                            // 如果找到匹配的标签，退出外部循环
                            break;
                        }
                    }
                    if(!bMatchingTagFound)
                    {
                        // 我们没有找到与预设标签匹配的剧情标签
                        bCurrentGroupConditionMet = false;
                        UE_LOG(LogTemp, Warning, TEXT("未找到匹配的剧情标签: '%s'"), *PresetTag.ToString());
                        break;
                    }
                }
                else
                {
                    // 预设标签的嵌入尚未计算，因此无法满足组条件
                    bCurrentGroupConditionMet = false;
                    UE_LOG(LogTemp, Warning, TEXT("预设标签 '%s' 的嵌入向量尚未找到"), *PresetTag.ToString());
                    break;
                }
            }
            if (!bCurrentGroupConditionMet)
            {
                // 组条件不满足
                bAllGroupConditionsMet = false;
                UE_LOG(LogTemp, Warning, TEXT("标签组条件未满足，无法触发事件"));
                break; // 如果一个组条件不符，提前退出以提高效率
            }
        }
        // 设置事件实例的条件满足标志
        EventInfo->PrecedingPlotTagGroupsConditionMet = bAllGroupConditionsMet;
        UE_LOG(LogTemp, Warning, TEXT("事件 '%s' 的前置标签组条件是否满足: %s"), *EventInfo->PresetData.EventName, bAllGroupConditionsMet ? TEXT("是") : TEXT("否"));
    }
}

void UTAPlotManager::ParseNewEventToTagGroups()
{
	TArray<FChatLog> TempMessagesList = ShoutHistory;
	const FString FormattedPrompt = UTALLMLibrary::PromptToStr(PromptTagEvent);
	TempMessagesList.Add({EOAChatRole::SYSTEM, FormattedPrompt});

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
					UE_LOG(LogTemp, Warning, TEXT("解析JSON失败: %s"), *Message.message.content);
				}
			}
			else
			{
				 //打印错误信息
				 UE_LOG(LogTemp, Error, TEXT("请求失败: %s"), *ErrorMessage);
			}
		},GetWorld());
}


bool UTAPlotManager::GetTagEmbeddingsFromSystem(const FName& Tag, FHighDimensionalVector& OutEmbeddingVec)
{
	return GetWorld()->GetGameInstance()->GetSubsystem<UTAEmbeddingSystem>()->GetTagEmbedding(Tag,OutEmbeddingVec);
}

void UTAPlotManager::ProcessShoutInGame(const FChatCompletion& Message, AActor* Shouter, float Volume)
{
	// 存储接收到的消息，在FullShoutHistory中始终保留完整记录
	FullShoutHistory.Add(Message.message);

	// 更新压缩后的ShoutHistory
	ShoutHistory.Add(Message.message);
  
	// 触发消息压缩逻辑（如果适用）
	if (bEnableCompressShout && ShoutHistory.Num() > 2200)
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

const FTAPrompt UTAPlotManager::PromptTagEvent = FTAPrompt{
	"Based on the narrative content of the game event provided below, we require an analysis that produces a structured set of tags. "
	"These tags should reflect key elements and actions within the event in a sequenced manner. "
	"Your task is to format the response as a JSON array of objects, where each object represents a distinct event or action sequence with a set of associated tags. "
	"Each object should have an 'event' key representing a event description, and a 'tags' key containing an array of tag strings in sequential order. "
	"If there are multiple events or outcomes within the given event description, create a separate object for each. "
	"Each tag group strung together can express the meaning of the result (following the principle of the initiator (i.e., the active party or the trigger) followed by the result (i.e., effect or status change))."
	"One tag group only represents one thing."
	//"When use someone's name as a tag, use full name please."
	"You just need to focus on the last message as it is the newly occurring incident, and what happened before you have already handled, showing it to you is only to maintain the context without loss."
	"You can use more game-like tags, such as \"accepting quests\". You do not need to break down illogical things into tags, such as pure landscape descriptions."
	"Please respond in Chinese, because our matching system only works in Chinese."
	"The response should be in the following JSON format: "
	"["
	"   {"
	"       \"event\": \"The warrior swings his sword at the evil dragon, causing damage, the dragon is injured\", "
	"       \"tags\": [\"warrior\", \"attack\", \"evil dragon\", \"injured\"]"
	"   },"
	"   {"
	"       \"event\": \"The warrior accept the mission to find the gem\", "
	"       \"tags\": [\"warrior\", \"accept mission\", \"find the gem\"]"
	"   }"
	"]"
	"Please proceed with the extraction and tagging for the following game event:",
	1,
	true
};