// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.
#include "Agent/TANarrativeAgent.h"
#include "Chat/Shout/TAShoutComponent.h"
#include "Agent/TAAgentComponent.h"
#include "Chat/TAFunctionInvokeComponent.h"

ATANarrativeAgent::ATANarrativeAgent()
{
	// 设置默认值
	AgentInfo.AgentName = TEXT("NarrativeAgent");
	SystemPrompt = TEXT("Default System Prompt");

	ShoutComponent = CreateDefaultSubobject<UTAShoutComponent>(TEXT("ShoutComponent"));
	ShoutComponent->bEnableFunctionInvoke = false;

	AgentComponent = CreateDefaultSubobject<UTAAgentComponent>(TEXT("AgentComponent"));
	AgentComponent->bEnableScheduleShout = false;

	bIsVoiceover = true;
}

void ATANarrativeAgent::CheckAndHandleInventoryEmpty()
{
	if(AgentInfo.ItemTable.Num() == 0 && AgentInfo.bHideWhenInventoryEmpty)
	{
		HideSelf();
	}
}

void ATANarrativeAgent::BeginPlay()
{
	Super::BeginPlay();
	
	if(FunctionInvokeCompClass)
	{
		FunctionInvokeComponent = NewObject<UTAFunctionInvokeComponent>(this, FunctionInvokeCompClass);
		FunctionInvokeComponent->RegisterComponent();
		OnFunctionInvokeComponentCreated(FunctionInvokeComponent);
	}

	if(AgentID > 0)
	{
		InitAgentByID(AgentID);
	}
}

void ATANarrativeAgent::InitAgentByID(int32 NewAgentID)
{
	AgentID = NewAgentID;
	// 将AgentID转为DataTable行名
	FName DataTableRowName = FName(*FString::Printf(TEXT("%d"), AgentID)); 

	auto DataTable = GetAgentDataTable();
	if(DataTable)
	{
		// 从数据表中查找与AgentID相匹配的行
		FNarrativeAgentData* AgentData = DataTable->FindRow<FNarrativeAgentData>(DataTableRowName, TEXT("LookUpAgentData"));

		// 如果找到对应行则更新Agent信息
		if(AgentData)
		{
			AgentInfo = *AgentData;
			
			SetIdentityPositionName(FName(AgentData->AgentName));
			RegisterActorTAGuid(this, GetIdentityPositionName());
			
			// 根据系统提示模板种类和参数生成系统提示
			SystemPrompt = GenerateSystemPrompt(AgentData->SystemPromptType, AgentData->SystemPromptParameters);

			// 使Agent可以开始说话
			AgentComponent->bEnableScheduleShout = true;

			InitAgentByID_BP(AgentID);
			
			UE_LOG(LogTemp, Log, TEXT("Agent data for ID %d found."), AgentID);
		}
		else
		{
			// 处理未找到数据的情况，可能包括设置默认值，或者记录日志
			UE_LOG(LogTemp, Error, TEXT("Agent data for ID %d not found."), AgentID);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Agent data table not found."));
	}
}

UDataTable* ATANarrativeAgent::GetAgentDataTable_Implementation() const
{
	return nullptr;
}

FString ATANarrativeAgent::GetSystemPrompt()
{
	if (!TotalDesire.IsEmpty())	
	{
		return FString::Printf(TEXT("%s. Your Near Information is:[%s]"), *SystemPrompt, *TotalDesire);
	}
	return SystemPrompt + AppendSystemPrompt;
}

const FString& ATANarrativeAgent::GetAgentName() const
{
	return AgentInfo.AgentName;
}

void ATANarrativeAgent::AddOrUpdateDesire(const FGuid& DesireId, const FString& DesireDescription)
{
	DesireMap.Add(DesireId, DesireDescription);

	TotalDesire.Empty();
	for (auto& Elem : DesireMap)
	{
		TotalDesire += Elem.Value + TEXT("\n");
	}
}

void ATANarrativeAgent::RemoveDesire(const FGuid& DesireId)
{
	DesireMap.Remove(DesireId);

	TotalDesire.Empty();
	for (auto& Elem : DesireMap)
	{
		TotalDesire += Elem.Value + TEXT("\n");
	}
}

TSoftObjectPtr<UTexture2D> ATANarrativeAgent::GetAgentPortrait() const
{
	return AgentInfo.AgentPortrait;
}

TMap<FName, int32> ATANarrativeAgent::QueryInventoryItems() const
{
	return AgentInfo.ItemTable;
}

int32 ATANarrativeAgent::QueryItemAmountByName(FName ItemName) const
{
	const int32* FoundAmount = AgentInfo.ItemTable.Find(ItemName);
	return FoundAmount ? *FoundAmount : 0;
}

bool ATANarrativeAgent::ConsumeInventoryItem(FName ItemName, int32 ConsumeCount)
{
	int32* FoundAmount = AgentInfo.ItemTable.Find(ItemName);
	if (FoundAmount && *FoundAmount >= ConsumeCount)
	{
		*FoundAmount -= ConsumeCount;
		if(*FoundAmount == 0)
		{
			AgentInfo.ItemTable.Remove(ItemName);
		}
		CheckAndHandleInventoryEmpty();
		return true;
	}
	return false;
}

bool ATANarrativeAgent::IsVoiceover() const
{
	return bIsVoiceover;
}

FString ATANarrativeAgent::GenerateSystemPrompt(EPromptType PromptType, const TArray<FString>& Parameters)
{
	switch (PromptType)
	{
	case EPromptType::Simple:
		{
			FSimplePromptTemplate Template;
			return Template.BuildPrompt(Parameters);
		}
	case EPromptType::Environment:
		{
			FEnvironmentPromptTemplate Template;
			if (Parameters.Num() >= 2)
			{
				Template.Description = Parameters[0];
				Template.Hint = Parameters[1];
				return Template.BuildPrompt();
			}
			break;
		}
	case EPromptType::InteractiveObject:
		{
			FInteractiveObjectPromptTemplate Template;
			if (Parameters.Num() >= 2)
			{
				Template.ObjectName = Parameters[0];
				Template.InteractionAction = Parameters[1];
				return Template.BuildPrompt();
			}
			break;
		}
	default:
		break;
	}

	// Default fallback prompt if the type doesn't match
	return TEXT("System is unable to generate prompt due to unknown type or missing parameters.");
}
