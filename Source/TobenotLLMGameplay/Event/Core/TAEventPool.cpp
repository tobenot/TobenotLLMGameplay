// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAEventPool.h"

#include "Event/TAEventLogCategory.h"
#include "Save/TAGuidSubsystem.h"
#include "Scene/TAPlaceActor.h"

// 修改后的 AddEvent 方法
FTAEventInfo& UTAEventPool::AddEvent(FTAEventInfo EventInfo)
{
	FTAEventInfo& EventInfoRef = AllEventInfo.Add_GetRef(EventInfo);
	EventInfoRef.EventID = AllEventInfo.Num() + 660000;
	PendingEventInfos.Add(&AllEventInfo.Last());

	if (EventInfo.ActivationType == EEventActivationType::Proximity && !bHasStartedProximityCheck)
	{
		StartProximityCheck();
		bHasStartedProximityCheck = true;
	}
	
	return EventInfoRef;
}

FTAEventInfo& UTAEventPool::GetEventByID(int32 EventID, bool& bSuccess)
{
	for (FTAEventInfo& EventInfoRef : AllEventInfo)
	{
		if (EventInfoRef.EventID == EventID)
		{
			bSuccess = true;
			return EventInfoRef;
		}
	}
	
	bSuccess = false;
	return ZeroEvent;
}

// 开启周期性检查
void UTAEventPool::StartProximityCheck() {
	UE_LOG(LogTAEventSystem, Log, TEXT("StartProximityCheck"));
	// 配置定时器代理来定时执行检查函数
	GetWorld()->GetTimerManager().SetTimer(EventTriggerTimerHandle, this, &UTAEventPool::CheckPlayerProximityToEvents, 1.0f, true);
}

void UTAEventPool::BeginDestroy()
{
	if(EventTriggerTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(EventTriggerTimerHandle);
	}
	UObject::BeginDestroy();
}

// 定义检查函数
void UTAEventPool::CheckPlayerProximityToEvents() {
	// 循环遍历待激活事件信息
	for (int32 i = PendingEventInfos.Num() - 1; i >= 0; --i) {
		const auto& EventInfo = PendingEventInfos[i];
        
		// 获取玩家位置
		const APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		if(PlayerPawn) {
			FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
			// 获取位点Actor
			UTAGuidSubsystem* GuidSubsystem = GetWorld()->GetSubsystem<UTAGuidSubsystem>();
			if(GuidSubsystem) {
				const ATAPlaceActor* PlaceActor = Cast<ATAPlaceActor>(GuidSubsystem->GetActorByGUID(EventInfo->LocationGuid));
                
				// 检查玩家是否在位点附近
				if(PlaceActor && FVector::Dist(PlayerLocation, PlaceActor->GetActorLocation()) <= PlaceActor->PlaceRadius) {
					// 玩家在范围内，激活事件并从待激活列表移除
					UE_LOG(LogTAEventSystem, Log, TEXT("CheckPlayerProximityToEvents, trigger one , remain pending %d") , PendingEventInfos.Num() - 1);
					UTAEventInstance* NewEventInstance = NewObject<UTAEventInstance>(this, UTAEventInstance::StaticClass());
					if(NewEventInstance) {
						// 使用生成的事件信息初始化NewEvent
						NewEventInstance->EventInfo = *EventInfo;
						ActiveEvents.Add(NewEventInstance);
						NewEventInstance->TriggerEvent();
					}					
                    
					PendingEventInfos.RemoveAt(i);
				}
			}
		}
	}
}

bool UTAEventPool::HasAnyEvents() const
{
	return AllEventInfo.Num() > 0;
}