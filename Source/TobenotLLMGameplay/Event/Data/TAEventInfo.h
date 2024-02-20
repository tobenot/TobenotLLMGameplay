// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// TAEventInfo.h
// 存储事件的基本信息

#pragma once

#include "CoreMinimal.h"
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

USTRUCT(BlueprintType)
struct FTAEventInfo
{
	GENERATED_BODY()

public:
	// 事件ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	int32 EventID;

	// 保存地点的唯一标识符
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FGuid LocationGuid;

	// 事件触发方式
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	EEventActivationType ActivationType;
	
	// LLM start

	// 事件触发地名（如水井、草丛）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString LocationName;
	
	// 事件描述
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString Description;

	// 事件类型，现在使用枚举类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	ETAEventType EventType;

	// 事件权重
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	int32 Weight;
	
	// LLM end
	
	FString ToString() const;
};