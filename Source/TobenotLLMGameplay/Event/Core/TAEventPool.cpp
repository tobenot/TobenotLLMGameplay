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

	// 检测网状叙事系统前置
	UTAPlotManager* PlotManager = GetWorld()->GetSubsystem<UTAPlotManager>();
	if (PlotManager)
	{
		PlotManager->CheckEventsTagGroupCondition(PendingEventInfos);
	}

	// 接下来遍历处于待触发状态的事件，检查更复杂的条件
	for (int32 i = PendingEventInfos.Num() - 1; i >= 0; --i) {
		const auto& EventInfo = PendingEventInfos[i];
		bool bDependencyMet = EventInfo->PrecedingPlotTagGroupsConditionMet;
		if(!bDependencyMet)
		{
			continue;
		}
		
		// 检查所有前置事件是否满足条件
		for (const FTAEventDependency& Dependency : EventInfo->PresetData.PrecedingEvents)
		{
			if (!IsDependencyMet(Dependency))
			{
				bDependencyMet = false; // 如果有任何一个前置事件的条件没有满足，当前事件将不会被触发
				break;
			}
		}

		if(bDependencyMet)
		{
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

void UTAEventPool::AddCompletedEvent(int32 EventID, int32 OutcomeID)
{
	// 将事件添加到已完成的事件映射中
	CompletedEventsOutcomeMap.Add(EventID, OutcomeID);
}

UTAEventInstance* UTAEventPool::GetEventInstanceByID(int32 EventID)
{
	// 遍历活动事件数组
	for (UTAEventInstance* EventInstance : ActiveEvents)
	{
		// 检查当前事件实例的事件ID是否匹配
		if (EventInstance && EventInstance->EventInfo.PresetData.EventID == EventID)
		{
			// 如果找到匹配的事件实例，返回该实例
			return EventInstance;
		}
	}

	// 如果没有找到匹配的事件实例，返回nullptr
	return nullptr;
}

bool UTAEventPool::IsDependencyMet(const FTAEventDependency& Dependency)
{
	// 通过查询映射查看依赖的事件是否已完成，并且结果符合所需的结果ID
	if (int32* FoundOutcomeID = CompletedEventsOutcomeMap.Find(Dependency.PrecedingEventID))
	{
		// 如果需要的结果ID为0，或者实际结果ID符合Dependency
		return Dependency.RequiredOutcomeID == 0 || *FoundOutcomeID == Dependency.RequiredOutcomeID;
	}

	// 如果该前置事件未完成，返回false
	return false;
}
