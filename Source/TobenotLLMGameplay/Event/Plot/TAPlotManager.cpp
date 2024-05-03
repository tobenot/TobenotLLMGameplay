﻿// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


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

// 获取 第一个事件的前置标签组 数组 的长度
// Events[0]->PresetData.PrecedingPlotTagGroups.Num()
// 这个是布尔值
// Events[0]->PrecedingPlotTagGroupsConditionMet = false
void UTAPlotManager::CheckEventsTagGroupCondition(TArray<FTAEventInfo>& Events)
{
    UTAEmbeddingSystem* EmbeddingSystem = GetWorld()->GetGameInstance()->GetSubsystem<UTAEmbeddingSystem>();

    for (FTAEventInfo& EventInfo : Events)
    {
        // 如果事件的前置标签组条件已经被满足，跳过这个事件
        if(EventInfo.PrecedingPlotTagGroupsConditionMet) 
        {
            continue;
        }

        // 如果没有前置标签组, 则直接标记为条件满足
        if(EventInfo.PresetData.PrecedingPlotTagGroups.Num() == 0) 
        {
            EventInfo.PrecedingPlotTagGroupsConditionMet = true;
            continue;
        }

        bool bOrGroupConditionMet = false; // 至少一个组满足（OR逻辑）
    	bool bHasOrGroup = false;
        bool bAllAndGroupConditionsMet = true; // 所有组都必须满足（AND逻辑）

    	for (const FTATagGroup& PresetGroup : EventInfo.PresetData.PrecedingPlotTagGroups)
    	{
    		bool bCurrentGroupConditionMet = false;
    		int32 TagIndex = 0;
    	
    		if(!bHasOrGroup && !PresetGroup.Flag)
            {
                bHasOrGroup = true;
            }
    		FHighDimensionalVector PresetTagEmbedding;

    		// 迭代此剧情标签组中的所有标签
    		for (int32 PlotTagIndex = 0; PlotTagIndex < PlotTagGroups.Num(); ++PlotTagIndex)
    		{
    			const FTATagGroup& PlotGroup = PlotTagGroups[PlotTagIndex];
    			// 每个事件记录独立
    			TagIndex = 0;
    			if(!EmbeddingSystem->GetTagEmbedding(PresetGroup.Tags[TagIndex], PresetTagEmbedding))
    			{
    				break;
    			}
    			
    			for (int i = 0; i < PlotGroup.Tags.Num(); ) {
    				const FName& PlotTag = PlotGroup.Tags[i];
    				FHighDimensionalVector PlotTagEmbedding;
    				if (EmbeddingSystem->GetTagEmbedding(PlotTag, PlotTagEmbedding)) {
    					float Similarity = UTAEmbeddingSystem::CalculateCosineSimilarity(PresetTagEmbedding, PlotTagEmbedding);
    					if(Similarity > 0.5)
    					{
    						UE_LOG(LogTAEventSystem, Warning,
							TEXT("大于0.5的日志: 当前的预设前置 '%s' 与剧情标签 '%s' 的嵌入向量余弦相似度为：%f"),
							*PresetGroup.Tags[TagIndex].ToString(), *PlotTag.ToString(), Similarity);
    					}
    					if (Similarity > 0.65) {
    						TagIndex++; // 移动到前置标签组中的下一个标签
    						if (TagIndex >= PresetGroup.Tags.Num()) {
    							// 如果所有标签都已匹配，则此标签组条件满足
    							bCurrentGroupConditionMet = true;
    							PlotTagIndex = PlotTagGroups.Num(); // 退出外部循环
    							break;
    						}

    						if (!EmbeddingSystem->GetTagEmbedding(PresetGroup.Tags[TagIndex], PresetTagEmbedding)) {
    							break; // 无法获取下一个预设标签的嵌入向量
    						}
    						continue; // 匹配成功，继续使用当前的剧情标签进行下一轮匹配
    					}
    				}
    				i++;  // 当前标签无匹配或匹配不成功，移动到下一个标签
    			}

    			if (bCurrentGroupConditionMet) {
    				break;  // 已找到匹配的标签组，退出循环
    			}
    		}

            // 根据Flag处理与或关系
            if(PresetGroup.Flag) // 如果是AND逻辑
            {
                bAllAndGroupConditionsMet &= bCurrentGroupConditionMet;
            	if(!bCurrentGroupConditionMet)
            	{
            		break; //AND逻辑有一个不满足可以退了
            	}
            }
            else if(bCurrentGroupConditionMet)
            {
            	bOrGroupConditionMet = true;
            	break; // OR逻辑下，只要有一个组满足即可
            }
        }

        // 检查是否所有AND组都满足，以及是否至少有一个OR组满足
        EventInfo.PrecedingPlotTagGroupsConditionMet = bAllAndGroupConditionsMet && (bOrGroupConditionMet || !bHasOrGroup);
        UE_LOG(LogTAEventSystem, Warning, TEXT("事件 '%s' 的前置标签组条件是否满足: %s"), *EventInfo.PresetData.EventName, EventInfo.PrecedingPlotTagGroupsConditionMet ? TEXT("是") : TEXT("否"));
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
	TempMessagesList.Add({EOAChatRole::SYSTEM, TEXT("Now Break new Event into tags:")});
	
	FChatSettings ChatSettings{
		UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
		TempMessagesList,
		0 // 0度，争取别搞错了
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
	TEXT(
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
		"例子 1："
		"{""\"event\": \"约翰·史密斯接受了莎拉·约翰逊的任务。\",""\"tags\": [\"约翰·史密斯\", \"接受任务\", \"寂静森林中隐藏的宝藏\"]""}"
		"例子 2："
		"{""\"event\": \"莎拉·约翰逊使用神秘的博学灯笼，揭示了古代遗迹中的秘密入口。\",""\"tags\": [\"莎拉·约翰逊\", \"使用\", \"博学灯笼\"]""}"
		"例子 3："
		"{""\"event\": \"小偷马丁·布莱克从皇家军械库偷走了魔剑埃克斯卡利伯。\",""\"tags\": [\"马丁·布莱克\", \"偷窃\", \"埃克斯卡利伯\"]""}"
		"例子 4："
		"{""\"event\": \"莎拉·约翰逊攻击约翰·史密斯。\",""\"tags\": [\"莎拉·约翰逊\", \"攻击\", \"约翰·史密斯\"]""}"
		"例子 5："
		"{""\"event\": \"约翰·史密斯把苹果给莎拉·约翰。\",""\"tags\": [\"约翰·史密斯\", \"给予苹果\", \"莎拉·约翰\"]""}"
	"Please focus only on the last message as it represents the newly occurring event. You have already processed what happened before. Showing them now is only for maintaining the context without loss."
	"Kindly reply content solely in Chinese to comply with our system protocols."
	"Avoid using English tags when responding!"
	"Proceed with the extraction and tagging as directed for the last event of following game event:"
	),
	1,
	true
};