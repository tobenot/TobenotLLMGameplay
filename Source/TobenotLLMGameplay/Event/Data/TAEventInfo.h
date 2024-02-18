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

USTRUCT(BlueprintType)
struct FTAEventInfo
{
	GENERATED_BODY()

public:
	// 事件ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	int32 EventID;

	// 事件描述
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString Description;

	// 事件类型，现在使用枚举类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	ETAEventType EventType;

	// 事件权重
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	int32 Weight;

	FString ToString() const;
};