#include "TAAgentComponent.h"
#include "TimerManager.h"
#include "Chat/Shout/TAShoutComponent.h"
#include "Shout/TAShoutManager.h"

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
	// 获取对 UTAShoutManager 的引用
	UTAShoutManager* ShoutManager = GetWorld()->GetSubsystem<UTAShoutManager>();
	if (!ShoutManager) return;

	// 获取周围范围内的 UTAShoutComponent 列表
	float ShoutRange = 700.f;
	TArray<UTAShoutComponent*> NearbyShoutComponents = ShoutManager->GetShoutComponentsInRange(GetOwner(), ShoutRange);

	// 检查除了自己之外是否有其他 UTAShoutComponent
	if (NearbyShoutComponents.Num() > 1 || (NearbyShoutComponents.Num() == 1 && NearbyShoutComponents[0] != GetOwner()->FindComponentByClass<UTAShoutComponent>()))
	{
		// 如果有其他组件，则请求发言
		UTAShoutComponent* ShoutComponent = GetOwner()->FindComponentByClass<UTAShoutComponent>();
		if (ShoutComponent)
		{
			ShoutComponent->RequestToSpeak();
		}
	}
	else
	{
		// 如果没有其他接收者，则不发言
		// 例如，你可以在这里记录一条日志或执行一些其他逻辑
	}
}

void UTAAgentComponent::ScheduleNextShout()
{
	// 在配置的最小和最大时间之间随机选择下次喊话的时间
	TimeToNextShout = FMath::RandRange(MinTimeBetweenShouts, MaxTimeBetweenShouts);
}