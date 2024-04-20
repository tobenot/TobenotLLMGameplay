// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAEventPool.h"

#include "Event/TAEventLogCategory.h"
#include "Save/TAGuidSubsystem.h"
#include "Scene/TAPlaceActor.h"

// 修改后的 AddEvent 方法
FTAEventInfo& UTAEventPool::AddEvent(FTAEventInfo EventInfo)
{
	FTAEventInfo& EventInfoRef = AllEventInfo.Add_GetRef(EventInfo);
	if(EventInfoRef.PresetData.EventID == 0)
	{
		EventInfoRef.PresetData.EventID = AllEventInfo.Num() + 660000;
	}
	PendingEventInfos.Add(&AllEventInfo.Last());

	if (!bHasStartedProximityCheck)
	{
		StartTriggerCheck();
		bHasStartedProximityCheck = true;
	}
	
	return EventInfoRef;
}

FTAEventInfo& UTAEventPool::GetEventByID(int32 EventID, bool& bSuccess)
{
	for (FTAEventInfo& EventInfoRef : AllEventInfo)
	{
		if (EventInfoRef.PresetData.EventID == EventID)
		{
			bSuccess = true;
			return EventInfoRef;
		}
	}
	
	bSuccess = false;
	return ZeroEvent;
}

// 开启周期性检查
void UTAEventPool::StartTriggerCheck() {
	UE_LOG(LogTAEventSystem, Log, TEXT("StartProximityCheck"));
	// 配置定时器代理来定时执行检查函数
	GetWorld()->GetTimerManager().SetTimer(EventTriggerTimerHandle, this, &UTAEventPool::CheckAndTriggerEvents, 1.0f, true);
}

void UTAEventPool::BeginDestroy()
{
	if(EventTriggerTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(EventTriggerTimerHandle);
	}
	UObject::BeginDestroy();
}

void UTAEventPool::CheckAndTriggerEvents()
{
	// 首先执行原来的接近性检查
	CheckPlayerProximityToEvents();

	// 接下来遍历处于待触发状态的事件，检查更复杂的条件
	for (int32 i = PendingEventInfos.Num() - 1; i >= 0; --i) {
		const auto& EventInfo = PendingEventInfos[i];
		bool bConditionsMet = true;

		// 检查每个事件的所有Agent条件
		for (auto& Condition : EventInfo->PresetData.AgentConditions) {
			if (!CheckAgentCondition(Condition)) {
				bConditionsMet = false;
				break; // 如果任何条件失败了，跳过剩余的检查
			}
		}

		if (bConditionsMet) {
			// 若所有条件都满足，则触发事件
			UE_LOG(LogTAEventSystem, Log, TEXT("CheckAndTriggerEvents, trigger %s") , *EventInfo->PresetData.EventName);
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

bool UTAEventPool::CheckAgentCondition(const FTAAgentCondition& Condition)
{
	// 对Agent条件进行检查的逻辑...
	return true; // 示例代码，默认返回true
}