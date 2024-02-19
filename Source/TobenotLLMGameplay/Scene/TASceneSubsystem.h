// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TASceneSubsystem.generated.h"

class ATAPlaceActor;
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
	UFUNCTION(BlueprintCallable, Category = "Event|Location")
	ATAPlaceActor* QueryEventLocationByInfo(const FTAEventInfo& EventInfo);
	
	// 创建并添加新的位点到列表中，返回创建的位点Actor
	UFUNCTION(BlueprintCallable, Category = "Place")
	ATAPlaceActor* CreateAndAddPlace(const FVector& Location, float Radius, const FString& Name);
	
private:
	// 存储所有位点Actors的引用的列表
	UPROPERTY()
	TArray<ATAPlaceActor*> PlaceActors;
	
	int32 MaxQueryEventLocationRetryCount = 100;
};
