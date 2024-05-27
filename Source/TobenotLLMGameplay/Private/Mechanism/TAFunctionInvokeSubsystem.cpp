#include "Mechanism/TAFunctionInvokeSubsystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

TMap<FString, FString> UTAFunctionInvokeSubsystem::FunctionDescriptions;

void UTAFunctionInvokeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    FunctionDescriptions.Add("ItemExchange", "在角色之间移动物品。\n调用说明：\"depict\": \"ItemName number SourceCharacter TargetCharacter\"。参数说明：物品名称、数量、源角色和目标角色。\n注意：请确保目标和源均已指定，否则可能导致错误。");
    FunctionDescriptions.Add("FinishSection", "结束本桥段。\n调用说明：\"depict\": \"\"。参数说明：无。\n注意：使用此函数结束当前桥段，标志着剧情结束，进入下一章。");
    FunctionDescriptions.Add("MoveToLocation", "移动角色到特定地点。\n调用说明：\"depict\": \"Character LocationName\"。参数说明：目标地点的名称。\n注意：请确保地点存在且角色可以安全移动。");
}

void UTAFunctionInvokeSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

FString UTAFunctionInvokeSubsystem::GetFunctionDescription(const FString& FunctionName)
{
    FString* Description = FunctionDescriptions.Find(FunctionName);
    if (Description)
    {
        return *Description;
    }
    return FString("未知函数");
}

FString UTAFunctionInvokeSubsystem::GetAllFunctionDescriptions()
{
    FString AllDescriptions;
    for (const auto& Pair : FunctionDescriptions)
    {
        AllDescriptions += FString::Printf(TEXT("%s: %s\n\n"), *Pair.Key, *Pair.Value);
    }
    return AllDescriptions;
}

void UTAFunctionInvokeSubsystem::InvokeFunction(const FString& FunctionName, AActor* AgentActor, const FString& Params)
{
    if (FunctionName == "ItemExchange")
    {
        ItemExchange(AgentActor, Params);
    }
    else if (FunctionName == "FinishSection")
    {
        FinishSection(AgentActor, Params);
    }
    else if (FunctionName == "MoveToLocation")
    {
        MoveToLocation(AgentActor, Params);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("未识别的函数调用: %s"), *FunctionName);
    }
}

void UTAFunctionInvokeSubsystem::ItemExchange(AActor* AgentActor, const FString& Params)
{
    // TODO: 实现 ItemExchange 的实际逻辑
}

void UTAFunctionInvokeSubsystem::FinishSection(AActor* AgentActor, const FString& Params)
{
    // TODO: 实现 FinishSection 的实际逻辑
}

void UTAFunctionInvokeSubsystem::MoveToLocation(AActor* AgentActor, const FString& Params)
{
    // TODO: 实现 MoveToLocation 的实际逻辑
}

FString UTAFunctionInvokeSubsystem::GenerateFunctionDescription(const TArray<FFunctionInvokeInfo>& FunctionCalls)
{
    FString Description;
    for (const auto& FunctionCall : FunctionCalls)
    {
        FString FunctionName = FunctionCall.FunctionName;
        FString FunctionSpecificDescription = GetFunctionDescription(FunctionName);
        FString Params = FunctionCall.Params;
        Description += FString::Printf(TEXT("\n函数 %s: %s，参数: %s"), *FunctionName, *FunctionSpecificDescription, *Params);
    }
    return Description;
}