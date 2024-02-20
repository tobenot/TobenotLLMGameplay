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

	virtual void BeginDestroy() override;
public:
	UFUNCTION(BlueprintCallable, Category = "Event")
	FTAEventInfo& AddEvent(FTAEventInfo EventInfo);

	UFUNCTION(BlueprintCallable, Category = "Event")
	FTAEventInfo& GetEventByID(int32 EventID, bool& bSuccess);

	FTAEventInfo ZeroEvent;
	
private:
	// 所有事件信息的集合，这里是唯一的最持久的保存事件信息的地方。由EventSubsystem管理。请用指针指它
	UPROPERTY(VisibleAnywhere, Category = "Event")
	TArray<FTAEventInfo> AllEventInfo;
	
	// 活跃事件的集合
	UPROPERTY(VisibleAnywhere, Category = "Event")
	TArray<UTAEventInstance*> ActiveEvents;

	TArray<FTAEventInfo*> PendingEventInfos;
	// 傻眼了吧孩子，这个结构体指针不能暴露给蓝图
private:
	bool bHasStartedProximityCheck = false;
	
	// 定义定时器句柄
	FTimerHandle EventTriggerTimerHandle;
    
	// 此函数用于开启周期性检查
	void StartProximityCheck();

public:
	// 定义检查功能函数
	UFUNCTION()
	void CheckPlayerProximityToEvents();
	
};
