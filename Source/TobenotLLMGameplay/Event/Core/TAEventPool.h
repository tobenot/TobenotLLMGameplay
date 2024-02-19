// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// TAEventPool.h
// 管理游戏中的活跃/待触发事件集合

#pragma once

#include "CoreMinimal.h"
#include "TAEventInstance.h"
#include "TAEventPool.generated.h"

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAEventPool : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Event")
	FTAEventInfo& AddEvent(FTAEventInfo EventInfo);

	UFUNCTION(BlueprintCallable, Category = "Event")
	bool GetEventByID(int32 EventID, FTAEventInfo& OutEventInfo);

	FTAEventInfo ZeroEvent;
private:
	// 所有事件信息的集合，这里是最持久的保存事件信息的地方。由EventSubsystem管理。
	UPROPERTY(VisibleAnywhere, Category = "Event")
	TArray<FTAEventInfo> AllEventInfo;
	
	// 活跃事件的集合
	UPROPERTY(VisibleAnywhere, Category = "Event")
	TArray<UTAEventInstance*> ActiveEvents;

	// 待触发的事件的集合
	UPROPERTY(VisibleAnywhere, Category = "Event")
	TArray<UTAEventInstance*> PendingEvents;
};
