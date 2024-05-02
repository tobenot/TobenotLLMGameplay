// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "TAPromptDefinitions.h"
#include "Subsystems/WorldSubsystem.h"
#include "TAPlotManager.generated.h"

struct FTAEventInfo;
struct FHighDimensionalVector;
struct FChatLog;
struct FChatCompletion;
/**
 * Structure to represent a group of FName tags.
 */
USTRUCT(BlueprintType)
struct FTATagGroup
{
    GENERATED_BODY()

    // An array of FName tags, forming the tag group.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Plot Tags")
    TArray<FName> Tags;

    // can use for logic/cache, 在事件配置里，false表示或，true表示与，一个事件要满足前置，必须全部与组匹配，且如果有或组的话，至少有一个或组匹配
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Plot Tags")
    bool Flag = false;

    FTATagGroup() {}
};

/**
 * UTAPlotManager
 *
 * This class serves as a subsystem to manage the narrative event tagging and triggering 
 * within the game world. It listens to in-game events, processes them to generate tag groups,
 * and checks for prerequisites of events that could be triggered as a consequence.
 */

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAPlotManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Constructor and destructor
    UTAPlotManager();
    virtual ~UTAPlotManager();

    // Initializes the subsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Deinitializes the subsystem
    virtual void Deinitialize() override;

    // Listens to in-game events and processes them
    // 	对话上的直接监听，监听UTAShoutManager 的回调, 监听到的是剧情内容（也就是发生了什么事）
    UFUNCTION(BlueprintCallable, Category="Plot Management")
    void ProcessShoutInGame(const FChatCompletion& Message, AActor* Shouter, float Volume);

    // Adds a new event to be tracked for potential triggering
    // 	机制上的，TAPlotManager自己提供上报接口，别的地方可以调用，以直接上报对应PlotTag
    UFUNCTION(BlueprintCallable, Category="Plot Management")
    void ReportEventTagGroups(const FTATagGroup& EventTagGroups);

    // TAPlotManager提供发起检测的接口，让别的系统定时调用。
    // Checks for event prerequisites and triggers them if satisfied
    void CheckEventsTagGroupCondition(TArray<FTAEventInfo>& Events);

protected:
    UPROPERTY()
    TArray<FTATagGroup> PlotTagGroups;
    
    // Internal function to parse events and categorize them into tag groups
    // 监听到的剧情内容，让transformer大语言模型根据上下文，拆分成多个标签组（这是一个异步过程，学习UTAShoutComponent使用SendMessageToOpenAIWithRetry的方法）
    // 	每个标签组串起来能表达结果含义（遵循先行动者（即主动方或触发方）后结果（即影响或状态变化）的原则）
    // 让gpt用数组形式回复，注意标签顺序，这些标签就是PlotTag
    /*  如果事件有多个结果，那么就产生多个记录。
        每一个记录本身应当只表达一件事。
        所有的标签组PlotTag组成事件记录（Plot记录）。
        */
    void ParseNewEventToTagGroups();

    //在标签组记录录入的时候，调用嵌入模型对所有的标签进行词嵌（缓存单个标签的词嵌结果，这样子大量的重复记录不会重复进行词嵌）
    // 获取tag的嵌入，调用它时，如果Tag还未嵌入完成，会返回false并开始嵌入过程，此时请跳过处理。
    bool GetTagEmbeddingsFromSystem(const FName& Tag, FHighDimensionalVector& OutEmbeddingVec);

    static const FTAPrompt PromptTagEvent;

private:
    // 这个就是上下文缓存 和上下文压缩相关的代码
    UPROPERTY()
    TArray<FChatLog> ShoutHistory;

    UPROPERTY()
    TArray<FChatLog> FullShoutHistory;
	
    FString ShoutHistoryCompressedStr;
    
    void RequestShoutCompression();

    bool bIsCompressingShout = false;
    int32 LastCompressedIndex;

public:
    static const FTAPrompt PromptCompressShoutHistory;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compress History")
    bool bEnableCompressShout = true;
    
    FString JoinShoutHistory();
};