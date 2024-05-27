// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// TAEventInfo.h
// 存储事件的基本信息

#pragma once

#include "CoreMinimal.h"
#include "Plot/TAPlotManager.h"
#include "TAEventInfo.generated.h"


// 定义事件类型枚举
UENUM(BlueprintType)
enum class ETAEventType : uint8
{
	Combat UMETA(DisplayName = "普通战"),
	BossFight UMETA(DisplayName = "BOSS战"),
	Exploration UMETA(DisplayName = "探索"),
	Story UMETA(DisplayName = "剧情"),
	Other UMETA(DisplayName = "其他"),
};

// 定义事件激活类型枚举
UENUM(BlueprintType)
enum class EEventActivationType : uint8
{
	Proximity UMETA(DisplayName = "靠近位点触发"),
	Geographic UMETA(DisplayName = "地理信息触发"),
	PlotProgress UMETA(DisplayName = "剧情进展触发"),
};

// Agent要满足的条件
USTRUCT(BlueprintType)
struct FTAAgentCondition
{
	GENERATED_BODY()

	// Agent的名称，唯一标识一个对象
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Condition")
	FName AgentName;

	// Agent需满足的状态条件，可能包括和其他Agent的距离，血量健康状态等
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Condition")
	FString RequiredState;
};

// Agent的欲望信息
USTRUCT(BlueprintType)
struct FTAAgentDesire
{
	GENERATED_BODY()

	// 欲望的唯一标识FGuid（用于追踪和取消）是在添加时决定的

	// Agent的名称，唯一标识一个对象
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desire")
	FName AgentName;

	// 欲望描述和对话内容
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desire")
	FString DesireDescription;

	// 获得这个欲望后是否马上想说话
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desire")
	bool ImmediatelyWantToSpeak = false;
};

USTRUCT(BlueprintType)
struct FTAEventDependency
{
	GENERATED_BODY()

	// 前置事件ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Dependency")
	int32 PrecedingEventID;

	// 前置事件需要达到的结果ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Dependency")
	int32 RequiredOutcomeID;
};

// 定义预设事件数据结构体
USTRUCT(BlueprintType)
struct TOBENOTLLMGAMEPLAY_API FTAPresetEventData : public FTableRowBase
{
	GENERATED_BODY()

	// 事件ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	int32 EventID;
	
	// 事件名称
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FString EventName;
	
	// 事件触发地名（如水井、草丛）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString LocationName;
	
	// 事件描述
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString Description;

	// 事件类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	ETAEventType EventType;

	// 事件权重
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	int32 Weight;

	// 事件的特殊点，用于生成交互物
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString PeculiarPoint;

	// 关联的Agent触发条件
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Dependency")
	TArray<FTAAgentCondition> AgentConditions;

	// 关联的Agent欲望列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TArray<FTAAgentDesire> AgentDesires;

	// 前置事件和结果的数组
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Dependency")
	TArray<FTAEventDependency> PrecedingEvents;

	// 前置的事件记录标签组
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Dependency")
	TArray<FTATagGroup> PrecedingPlotTagGroups;
};

// 更新FTAEventInfo结构体，包含FTAPresetEventData
USTRUCT(BlueprintType)
struct FTAEventInfo
{
	GENERATED_BODY()

	// 保存地点的唯一标识符
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FGuid LocationGuid;

	// 事件触发方式
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	EEventActivationType ActivationType;

	// LLM start

	// 包含预设事件数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FTAPresetEventData PresetData;

	// LLM end
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	bool PrecedingPlotTagGroupsConditionMet = false;
    
	// 事件转换为字符串的函数，用于调试打印输出
	FString ToString() const;
};