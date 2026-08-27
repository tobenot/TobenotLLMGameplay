#include "Mechanism/TAFunctionInvokeSubsystem.h"
#include "Agent/TAAgentComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Save/TASaveGameSubsystem.h"

void UTAFunctionInvokeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    FunctionDescriptions.Add("ItemExchange", 
        TEXT(
            R""""(
            在角色之间移动物品。
            调用说明：{
                "params": {
                    "ItemName": "物品名称",
                    "Number": "数量",
                    "SourceCharacter": "源角色",
                    "TargetCharacter": "目标角色"
                }
            }
            )""""));

    FunctionDescriptions.Add("FinishSection", 
        TEXT(
            R""""(
            结束本桥段。
            调用说明：{
                "params": {}
            }
            无参数。
            )""""));

    FunctionDescriptions.Add("MoveToLocation", 
        TEXT(
            R""""(
            移动角色到特定地点。
            调用说明：{
                "params": {
                    "Character": "角色",
                    "LocationName": "目标地点"
                }
            }
            )""""));

    FunctionDescriptions.Add("InvokeImmediateDecision",
        TEXT(
            R"""(
            令角色做出反应。一般用于被对话所指向的若干角色。
            调用说明：{
                "params": {
                    "Character": "角色"
                }
            }
            )"""));
}

void UTAFunctionInvokeSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

FString UTAFunctionInvokeSubsystem::GetFunctionDescription(const FString& FunctionName) const
{
    const FString* Description = FunctionDescriptions.Find(FunctionName);
    if (Description)
    {
        return *Description;
    }
    return FString("未知函数");
}

FString UTAFunctionInvokeSubsystem::GetAllFunctionDescriptions() const
{
    FString AllDescriptions;
    for (const auto& Pair : FunctionDescriptions)
    {
        AllDescriptions += FString::Printf(TEXT("%s: %s\n\n"), *Pair.Key, *Pair.Value);
    }
    return AllDescriptions;
}

void UTAFunctionInvokeSubsystem::InvokeFunction(const FString& FunctionName, AActor* AgentActor, const TMap<FString, FString>& Params)
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
    else if (FunctionName == "InvokeImmediateDecision")
    {
        InvokeImmediateDecision(AgentActor, Params);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("未识别的函数调用: %s"), *FunctionName);
    }
}

void UTAFunctionInvokeSubsystem::ItemExchange(AActor* AgentActor, const TMap<FString, FString>& Params)
{
    // 使用 Params 来进行参数解析和后续逻辑
    FString ItemName = Params.FindRef("ItemName");
    FString Number = Params.FindRef("Number");
    FString SourceCharacter = Params.FindRef("SourceCharacter");
    FString TargetCharacter = Params.FindRef("TargetCharacter");

    if (!ItemName.IsEmpty() && !Number.IsEmpty() && !SourceCharacter.IsEmpty() && !TargetCharacter.IsEmpty())
    {
        AActor* SourceActor = FindActorByName(SourceCharacter);
        AActor* TargetActor = FindActorByName(TargetCharacter);
        
        if (SourceActor && TargetActor)
        {
            // 实现 ItemExchange 的实际逻辑
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("未能找到源角色或目标角色: %s 或 %s"), *SourceCharacter, *TargetCharacter);
        }
    }   
}

void UTAFunctionInvokeSubsystem::FinishSection(AActor* AgentActor, const TMap<FString, FString>& Params)
{
    // 使用 Params 来进行参数解析和后续逻辑
}

void UTAFunctionInvokeSubsystem::MoveToLocation(AActor* AgentActor, const TMap<FString, FString>& Params)
{
    // 使用 Params 来进行参数解析和后续逻辑
    FString CharacterName = Params.FindRef("Character");
    FString LocationName = Params.FindRef("LocationName");

    if (!CharacterName.IsEmpty() && !LocationName.IsEmpty())
    {
        AActor* CharacterActor = FindActorByName(CharacterName);
        if (CharacterActor)
        {
            // 实现 MoveToLocation 的实际逻辑
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("未能找到角色: %s"), *CharacterName);
        }
    }
}

void UTAFunctionInvokeSubsystem::InvokeImmediateDecision(AActor* AgentActor, const TMap<FString, FString>& Params)
{
    FString CharacterName = Params.FindRef("Character");
    
    if (!CharacterName.IsEmpty())
    {
        AActor* CharacterActor = FindActorByName(CharacterName);
        if (CharacterActor)
        {
            UTAAgentComponent* AgentComponent = CharacterActor->FindComponentByClass<UTAAgentComponent>();
            if (AgentComponent)
            {
                AgentComponent->PerceiveAndDecide();
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("%s 没有找到相应的 UTAAgentComponent"), *CharacterName);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("未能找到角色: %s"), *CharacterName);
        }
    }
}

FString UTAFunctionInvokeSubsystem::GenerateFunctionDescription(const TArray<FFunctionInvokeInfo>& FunctionCalls) const
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

AActor* UTAFunctionInvokeSubsystem::FindActorByName(const FString& ActorName) const
{
    UWorld* World = GetWorld();
    if (World)
    {
        if (UTASaveGameSubsystem* SaveGameSubsystem = World->GetGameInstance()->GetSubsystem<UTASaveGameSubsystem>())
        {
            return SaveGameSubsystem->FindActorByName(FName(ActorName));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("未能找到 SaveGameSubsystem"));
        }
    }
    return nullptr;
}