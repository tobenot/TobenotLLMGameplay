// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TASceneSubsystem.generated.h"

struct FTAEventInfo;
/**
 * 
 */
UCLASS()
class TOBENOTLLMGAMEPLAY_API UTASceneSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 查询当前关卡的人文、地理信息
	FString QuerySceneMapInfo();
	
	// 查询指定位置所在的区域地点
	FString QueryLocationInfo(const FVector& Location);

	// 根据事件信息查询推荐的事件位置
	// @param EventInfo 事件的详细信息，用于确定事件合适的地理位置
	// @return 返回事件推荐的Location
	UFUNCTION(BlueprintCallable, Category = "Event|Location")
	FVector QueryEventLocationByInfo(const FTAEventInfo& EventInfo);
};
