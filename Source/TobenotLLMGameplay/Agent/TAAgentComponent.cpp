#include "TAAgentComponent.h"
#include "TimerManager.h"
#include "Chat/Shout/TAShoutComponent.h"

UTAAgentComponent::UTAAgentComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bEnableScheduleShout = true;
	// 默认喊话时间间隔范围
	MinTimeBetweenShouts = 10.f;
	MaxTimeBetweenShouts = 30.f;
}

void UTAAgentComponent::BeginPlay()
{
	Super::BeginPlay();
	ScheduleNextShout();
}

void UTAAgentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		if(bEnableScheduleShout)
		{
			// 减少等待时间
			TimeToNextShout -= DeltaTime;

			// 当等待时间结束时，调用RequestSpeak并计划下一次喊话
			if(TimeToNextShout <= 0)
			{
				RequestSpeak();
				ScheduleNextShout();
			}
		}
	}
}

void UTAAgentComponent::RequestSpeak()
{
	UTAShoutComponent* ShoutComponent = GetOwner()->FindComponentByClass<UTAShoutComponent>();

	if(ShoutComponent)
	{
		ShoutComponent->RequestToSpeak();
	}
}

void UTAAgentComponent::ScheduleNextShout()
{
	// 在配置的最小和最大时间之间随机选择下次喊话的时间
	TimeToNextShout = FMath::RandRange(MinTimeBetweenShouts, MaxTimeBetweenShouts);
}