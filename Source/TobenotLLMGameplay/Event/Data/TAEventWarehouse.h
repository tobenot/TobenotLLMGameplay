// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// TAEventWarehouse.h
// 管理预设事件，通常用于读取配置表格数据

#pragma once

#include "CoreMinimal.h"
#include "TAEventInfo.h"
#include "TAEventWarehouse.generated.h"

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAEventWarehouse : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 预设事件集合
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event")
	TArray<FTAEventInfo> PresetEvents;

	// 读取预设事件到集合
	void LoadPresetEvents();
};