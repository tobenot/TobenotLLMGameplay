// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "Event/Plot/TAPlotManager.h"

#include "OpenAIDefinitions.h"
#include "Common/TAEmbeddingSystem.h"
#include "Common/TALLMLibrary.h"
#include "Event/TAEventLogCategory.h"
#include "Event/Core/TAEventInstance.h"

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
    			// 每个事件记录独立 记录 当前预设组匹配到的下标
    			TagIndex = 0;
    			if(!EmbeddingSystem->GetTagEmbedding(PresetGroup.Tags[TagIndex], PresetTagEmbedding))
    			{
    				break;
    			}
    			
    			for (int i = 0; i < PlotGroup.Tags.Num(); ) {
    				if(PresetGroup.FlagIndex == TagIndex + 1)
    				{
    					// 预设里的动作tag必须和第二个剧情tag匹配
    					if(i>1)
    					{
    						break;
    					}else if(!i)
    					{
    						++i;
    						continue;
    					}
    				}
    				
    				const FName& PlotTag = PlotGroup.Tags[i];
    				bool IsMatch = PlotTag.IsEqual(PresetGroup.Tags[TagIndex]); //完全相同的直接成功
    				if(!IsMatch)
    				{
    					FHighDimensionalVector PlotTagEmbedding;
    					if (EmbeddingSystem->GetTagEmbedding(PlotTag, PlotTagEmbedding)) {
    						float Similarity = GetCachedCosineSimilarity(PresetGroup.Tags[TagIndex], PlotTag, PresetTagEmbedding, PlotTagEmbedding);
    						if(Similarity > 0.5 && Similarity < 1)
    						{
    							FString LogPairKey = PresetGroup.Tags[TagIndex].ToString() + TEXT("_") + PlotTag.ToString();
    							if (!PrintedLogPairs.Contains(LogPairKey))
    							{
    								UE_LOG(LogTAEventSystem, Warning,
										TEXT("大于0.5小于1的日志: 当前的预设前置 '%s' 与剧情标签 '%s' 的嵌入向量余弦相似度为：%f"),
										*PresetGroup.Tags[TagIndex].ToString(), *PlotTag.ToString(), Similarity);
    								PrintedLogPairs.Add(LogPairKey);
    							}
    						}
    						if (Similarity > 0.62) {
    							IsMatch = true;
    						}
    					}
    				}
    				if(IsMatch)
    				{
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
    				i++;  // 当前标签无匹配或匹配不成功，移动到下一个剧情标签
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
	//TempMessagesList.Add({EOAChatRole::SYSTEM, TEXT("Now Break new Event into tags:")});
	
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
                // 解析proactive_action并存储tags
                TSharedPtr<FJsonObject> ProactiveAction = JsonObject->GetObjectField("proactive_action");

                TArray<FName> ProactiveTags;
                ProactiveTags.Add(FName(*ProactiveAction->GetStringField("character_tag")));
                ProactiveTags.Add(FName(*ProactiveAction->GetStringField("action_tag")));
                ProactiveTags.Add(FName(*ProactiveAction->GetStringField("activity_tag")));

                TArray<TSharedPtr<FJsonValue>> ProactiveDetailTagsJsonArray = ProactiveAction->GetArrayField("activity_detail_tags");
                for (auto& JsonValue : ProactiveDetailTagsJsonArray)
                {
                    FName Tag = FName(*JsonValue->AsString());
                    if (!Tag.IsNone())
                    {
                        ProactiveTags.Add(Tag);
                    }
                }

                // 在将标签添加到标签组之前进行验证
                bool bIsValidTagGroup = true;
                for (const auto& Tag : ProactiveTags)
                {
                    if (Tag.IsNone())
                    {
                        bIsValidTagGroup = false;
                        break;
                    }
                }

                if (bIsValidTagGroup)
                {
                    // 将proactive_tags存入TagGroup结构体中
                    FTATagGroup ProactiveTagGroup;
                    ProactiveTagGroup.Tags.Append(ProactiveTags);
                    PlotTagGroups.Add(ProactiveTagGroup);
                }
                
                // 解析passive_action并存储tags
                TSharedPtr<FJsonObject> PassiveAction = JsonObject->GetObjectField("passive_action");

                TArray<FName> PassiveTags;
                PassiveTags.Add(FName(*PassiveAction->GetStringField("character_tag")));
                PassiveTags.Add(FName(*PassiveAction->GetStringField("action_tag")));
                PassiveTags.Add(FName(*PassiveAction->GetStringField("activity_tag")));

                TArray<TSharedPtr<FJsonValue>> PassiveDetailTagsJsonArray = PassiveAction->GetArrayField("activity_detail_tags");
            	for (auto& JsonValue : PassiveDetailTagsJsonArray)
            	{
					FName Tag = FName(*JsonValue->AsString());
					if (!Tag.IsNone())
					{
						PassiveTags.Add(Tag);
					}
				}

				// 在将标签添加到标签组之前进行验证
				bIsValidTagGroup = true;
				for (const auto& Tag : PassiveTags)
				{
					if (Tag.IsNone())
					{
						bIsValidTagGroup = false;
						break;
					}
				}

				if (bIsValidTagGroup)
				{
					// 将passive_tags存入TagGroup结构体中
					FTATagGroup PassiveTagGroup;
					PassiveTagGroup.Tags.Append(PassiveTags);
					PlotTagGroups.Add(PassiveTagGroup);
				}
			}
			else
			{
				// 解析JSON失败日志
				UE_LOG(LogTAEventSystem, Warning, TEXT("解析JSON失败: %s"), *Message.message.content);
			}
		}
		else
		{
			// 请求失败打印错误信息
			UE_LOG(LogTAEventSystem, Error, TEXT("请求失败: %s"), *ErrorMessage);
		}
	},GetWorld());
	
	/* 旧版本
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
		*/
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

float UTAPlotManager::GetCachedCosineSimilarity(FName TagA, FName TagB, const FHighDimensionalVector& VectorA, const FHighDimensionalVector& VectorB)
{
	const TPair<FName, FName> TagPair = TPair<FName, FName>(TagA, TagB);

	// 尝试从缓存中获取结果
	if (const float* CachedSimilarity = SimilarityCache.Find(TagPair))
	{
		return *CachedSimilarity;
	}
	else
	{
		// 如果缓存中未找到，需要调用计算相似度的函数
		// 注意：这里假设UTAEmbeddingSystem类有一个静态方法可以直接使用两个FName标签进行相似度计算
		const float Similarity = UTAEmbeddingSystem::CalculateCosineSimilarity(VectorA, VectorB);

		// 将结果添加到缓存
		SimilarityCache.Add(TagPair, Similarity);

		return Similarity;
	}
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
	R""""(
Please analyze the events and dialogue content I subsequently provide you in the game and generate a set of structured tags called action records, which will be used to detect and trigger new game events. These tags should reflect what specific actions took place. Specifically:
Character tags: Identify the subject of the action, such as Joan, Robert, etc.
Action tags: Describe the actions performed by characters, such as "open", "accept".
Activity tags: Indicate the general type of action, such as "trade", "quest", "monster".
Activity detail tags: Provide specific information about the activity, which can be further divided into several tags, such as the name of the quest, target object, who assigned the quest.
The response should use the following JSON format:
{
"guideline":"Faithfully record the actions that have actually occurred, do not conjecture actions that have not yet occurred, reply with Chinese tags, only decompose the latest message, do not leave second_analysis empty",
"analysis":"What is the latest message, what happened in this message. Analyse the details inside, which may include proactive actions. Lastly, please have second_analysis raise doubts.",
"second_analysis":"What analysis says can't actually be confirmed, because X, question",
"combine_analysis":"The doubt raised by second_analysis about X is indeed reasonable (we generally consider the point of question to be reasonable), so we cannot record X, but should instead record X. Only give proactive records here.",
"proactive_action":
{
"action": "Description of proactive action, A did what to B, character is A",
"character_tag": "Proactive character",
"action_tag": "Proactive behavior",
"activity_tag": "Proactive activity",
"activity_detail_tags": ["Proactive activity detail 1", "Proactive activity detail 2", ...]
},
"passive_reverse":"What is the action_tag of the proactive action, who is the initiator and who is the receiver, reversed, how should the passive record be written",
"passive_action":
{
"action": "The passive action corresponding to the proactive action, it's the semantic passive of the proactive action, B was done what by A, character is B",
"character_tag": "Passive character",
"action_tag": "Passive behavior",
"activity_tag": "Passive activity",
"activity_detail_tags": ["Passive activity detail 1", "Passive activity detail 2", ...]
}
}
Please focus only on the last message, as it represents the newly occurred action. You have already dealt with the actions that happened before. Showing them now is just to keep the context intact without loss.
Example:
{
"guideline":"Faithfully record the actions that have actually occurred, do not conjecture actions that have not yet occurred, reply with Chinese tags, only decompose the latest message, do not leave second_analysis empty",
"analysis":"The latest message is 'Adventurer, I've taken it, now let me take a look at this letter', here adventurer refers to Robert, in the previous message Joan had the action of giving out the letter, Robert's reply of taking it shows that he has got the letter, next he wants to check the letter. Decomposed into Joan delivering the letter and Robert checking the letter. Please have second_analysis raise doubts.",
"second_analysis":"What analysis says can't actually be confirmed, because Robert says he took it, it might just be an expectation, the letter might still be with Joan, Robert might not have reached out to take it. Is the letter definitely in Robert's hands? Does Robert's statement necessarily mean he received it?",
"combine_analysis":"The doubt raised by second_analysis that Robert might not have the letter is reasonable, because there is no action given by Joan, nor an accepting action by Robert, we need to wait until Robert actively takes the letter, or expresses that he has seen the content of the letter to be fully certain. So it cannot be decomposed into Robert obtained the letter, because we cannot be certain it was obtained, hoping for something does not mean it has already happened, we need to accurately describe the things that have happened, not conjecture about things not clearly occurred, we need to decompose it into Robert asking Joan for the letter",
"proactive_action":
{
"action": "Robert asks Joan for the letter",
"character_tag": "罗伯特",
"action_tag": "要求",
"activity_tag": "信件",
"activity_detail_tags": ["天山信件", "琼"]
},
"passive_reverse":"The action_tag of the proactive action is request, initiator is Robert, receiver is Joan, reversed, the passive record should be written as Joan being requested by Robert for the letter.",
"passive_action":
{
"action": "Joan is requested by Robert for the letter",
"character_tag": "琼",
"action_tag": "被请求",
"activity_tag": "信件",
"activity_detail_tags": ["天山信件", "罗伯特"]
}
}
Next example:
{
"guideline":"Faithfully record the actions that have actually occurred, do not conjecture actions that have not yet occurred, reply with Chinese tags, only decompose the latest message, do not leave second_analysis empty",
"analysis":"The latest message is 'I nod', Joan is nodding because in the previous message Robert was asking Joan if she accepts the task, the nodding action means Joan accepts the task of finding the sword from Robert. Decompose into accepting task proactive and passive actions. Please have second_analysis raise doubts.",
"second_analysis":"What analysis says can't actually be confirmed, because Joan might not be agreeing to what Robert said. Does Joan's nodding equal accepting the task?",
"combine_analysis":"Whether Joan definitely accepted the task as questioned by second_analysis is uncertain because Robert only mentioned the task, he didn't talk about any other topic, and Joan's nodding action is proactive, Joan's intention of nodding can only be towards Robert's task, so it cannot be decomposed into Joan nodding to Robert, we need to accurately describe the events that have taken place, we need to further refine, need to decompose it into Joan accepting the task",
"proactive_action":
{
"action": "Joan accepts the task",
"character_tag": "琼",
"action_tag": "接受",
"activity_tag": "任务",
"activity_detail_tags": ["寻找宝剑", "罗伯特"]
},
"passive_reverse":"The action_tag of the proactive action is accept, the initiator is Joan, the receiver is the task, reversed, the passive record should be written as the task being accepted by Joan",
"passive_action":
{
"action": "The task is accepted by Joan",
"character_tag": "寻找宝剑",
"action_tag": "被接受",
"activity_tag": "任务",
"activity_detail_tags": ["寻找宝剑", "罗伯特"]
}
}
Please reply with Chinese tags in the tags fields to comply with our system's protocol.
Extract and tag according to the instructions, focusing on the last content of the following game events/dialogue content:
	)""""),
	1,
	true
};

/*

旧版拆标签
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
		"Examples:"
		"{""\"event\": \"John Smith accepted Sarah Johnson's task.\",""\"tags\": [\"约翰·史密斯\", \"接受任务\", \"隐藏的宝藏\"]""}"
		"{""\"event\": \"Sarah Johnson used a mysterious and erudite lantern to reveal the secret entrance in ancient ruins.\",""\"tags\": [\"莎拉·约翰逊\", \"使用\", \"博学灯笼\"]""}"
		"{""\"event\": \"Sarah Johnson used a mysterious and erudite lantern to reveal the secret entrance in ancient ruins.\",""\"tags\": [\"莎拉·约翰逊\", \"揭示\", \"入口\"]""}"
		"{""\"event\": \"Thief Martin Blake stole the magic sword Exkeliber from the Royal Armory.\",""\"tags\": [\"马丁·布莱克\", \"偷窃\", \"埃克斯卡利伯\"]""}"
		"{""\"event\": \"Sarah Johnson attacked John Smith.\",""\"tags\": [\"莎拉·约翰逊\", \"攻击\", \"约翰·史密斯\"]""}"
		"{""\"event\": \"Sarah Johnson's attack on John Smith resulted in John Smith being injured.\",""\"tags\": [\"约翰·史密斯\", \"受伤\"]""}"
		"{""\"event\": \"John Smith gave the apple to Sarah John.\",""\"tags\": [\"约翰·史密斯\", \"给予苹果\", \"莎拉·约翰\"]""}"
	"You need to record things that are easier to match. For ambiguous things, if the context can be specific, you need to record them more specifically. For example, if A agrees to B's request, you should not record it as A agreeing to B. If you do, how can I match it? You can record it as 'A','agree to specific things', 'B', or as 'A','agree', or 'specific things'."
	"Please focus only on the last message as it represents the newly occurring event. You have already processed what happened before. Showing them now is only for maintaining the context without loss."
	"Kindly reply content solely in Chinese to comply with our system protocols."
	"Avoid using English tags when responding!"
	"Proceed with the extraction and tagging as directed for the last event of following game event:"
	),
	1,
	true
};
*/