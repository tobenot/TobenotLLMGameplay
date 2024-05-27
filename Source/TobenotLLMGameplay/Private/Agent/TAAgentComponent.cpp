
#include "Agent/TAAgentComponent.h"
#include "TimerManager.h"
#include "Agent/TAAgentInterface.h"
#include "Chat/Shout/TAShoutComponent.h"
#include "Chat/Shout/TAShoutManager.h"
#include "Common/TALLMLibrary.h"
#include "Mechanism/TAGameMasterSubsystem.h"
#include "TimerManager.h"

void UTAAgentComponent::PerceiveAndDecide()
{
	// 获取Owner对象
	const ITAAgentInterface* AgentInterface = Cast<ITAAgentInterface>(GetOwner());
	if (!AgentInterface)
	{
		UE_LOG(LogTemp, Error, TEXT("Owner does not implement ITAAgentInterface."));
		return;
	}

	// 获取感知和记忆数据
	FString PerceptionData = AgentInterface->GetPerceptionData();
	FString MemoryData = AgentInterface->GetMemoryData();

	// 构造系统提示作为 ChatLog 对象
	FString SystemPrompt = "请根据以下感知数据和记忆数据做出决策: ";
	SystemPrompt += "感知数据: " + PerceptionData + ", 记忆数据: " + MemoryData;
    
	TArray<FChatLog> TempMessagesList;
	TempMessagesList.Add(FChatLog{EOAChatRole::SYSTEM, SystemPrompt});
    
	// 设置 ChatSettings，包括引擎质量、历史记录等
	FChatSettings ChatSettings{
		UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
		TempMessagesList,
		0
	};
	ChatSettings.jsonFormat = true;
    
	// 发送请求，获取决策结果
	UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings, [this](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
	{
		FString Decision;
		if(Success)
		{
			Decision = Message.message.content;
            
			// 将决策发送给 Game Master
			SendDecisionToGameMaster(Decision);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("获取决策失败: %s"), *ErrorMessage);
            
			// 如果失败，可以选择发回一个默认的或重试的决策
			Decision = "不行动";
			SendDecisionToGameMaster(Decision);
		}
	}, GetOwner());
}

void UTAAgentComponent::SendDecisionToGameMaster(const FString& Decision)
{
	UTAGameMasterSubsystem* GameMasterSubsystem = GetWorld()->GetSubsystem<UTAGameMasterSubsystem>();
	if (GameMasterSubsystem)
	{
		GameMasterSubsystem->ReceiveDecisionFromAgent(GetOwner(), Decision);
	}
}

void UTAAgentComponent::HandleDecisionResponse(const FString& Outcome, bool bIsDecisionValid)
{
	if (bIsDecisionValid)
	{
		// 决策是有效的，开始生成对话
		UTAShoutComponent* ShoutComponent = GetOwner()->FindComponentByClass<UTAShoutComponent>();
		if (ShoutComponent)
		{
			ShoutComponent->GenerateDialogueAsync(Outcome, [this](const FString& Dialogue)
			{
				// 把对话反馈给主持人，完成整个流程
				UTAGameMasterSubsystem* GameMasterSubsystem = GetWorld()->GetSubsystem<UTAGameMasterSubsystem>();
				if (GameMasterSubsystem)
				{
					GameMasterSubsystem->ReceiveDialogueGenerated(GetOwner(), Dialogue);
				}
			});
		}
	}
	else
	{
		// 决策无效，重新进行决策
		PerceiveAndDecide();
	}
}

void UTAAgentComponent::ScheduleNextShout()
{
	// 在配置的最小和最大时间之间随机选择下次喊话的时间
	TimeToNextShout = FMath::RandRange(MinTimeBetweenShouts, MaxTimeBetweenShouts);
}

UTAAgentComponent::UTAAgentComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bEnableScheduleShout = true;
    
    // 默认喊话时间间隔范围
    MinTimeBetweenShouts = 25.f; // 30秒-5秒
    MaxTimeBetweenShouts = 35.f; // 30秒+5秒

    // 没有响应时重试喊话的默认时间间隔范围
    MinTimeBetweenRetryShouts = 0.5f;
    MaxTimeBetweenRetryShouts = 2.0f;

    // 默认决策时间间隔范围
    MinTimeBetweenDecisions = 25.f; // 30秒-5秒
    MaxTimeBetweenDecisions = 35.f; // 30秒+5秒
}

void UTAAgentComponent::BeginPlay()
{
    Super::BeginPlay();
    TimeToNextShout = MaxTimeBetweenRetryShouts;

    // 调度第一次喊话和决策时间
	
    // 暂时不组织喊话了
    // ScheduleNextShout();
	
    ScheduleNextDecision();
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
            }
        }

        // 新的决策逻辑时间减少
        TimeToNextDecision -= DeltaTime;

        // 当等待时间结束时，调用RequestDecision并计划下一次决策
        if(TimeToNextDecision <= 0)
        {
            RequestDecision();
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
        ScheduleNextShout();
    }
    else
    {
        // 如果没有其他接收者，则立即计划一个短时的下次喊话。
        // 因为这样子可以营造出玩家一走过去，Agent马上说话的情形。
        TimeToNextShout = FMath::RandRange(MinTimeBetweenRetryShouts, MaxTimeBetweenRetryShouts);
    }
}

void UTAAgentComponent::RequestDecision()
{
    // 获取感知、做出决策
    PerceiveAndDecide();

    // 重新调度下一次决策时间
    ScheduleNextDecision();
}

void UTAAgentComponent::ScheduleNextDecision()
{
    // 在配置的最小和最大时间之间随机选择下次决策的时间
    TimeToNextDecision = FMath::RandRange(MinTimeBetweenDecisions, MaxTimeBetweenDecisions);
}