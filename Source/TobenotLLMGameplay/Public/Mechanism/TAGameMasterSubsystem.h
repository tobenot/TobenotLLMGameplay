// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TAFunctionInvokeSubsystem.h"
#include "TAGameMasterSubsystem.generated.h"

// 添加一个结构体保存上下文信息
USTRUCT()
struct FDecisionContext
{
	GENERATED_BODY()
    
	UPROPERTY()
	AActor* AgentActor;
    
	UPROPERTY()
	FString OriginalDecision;
    
	FDecisionContext() : AgentActor(nullptr), OriginalDecision(TEXT("")) {}
	FDecisionContext(AActor* InAgentActor, const FString& InDecision) : AgentActor(InAgentActor), OriginalDecision(InDecision) {}
};
// 用于保存决策结果的结构体
USTRUCT()
struct FDecisionOutcome
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FFunctionInvokeInfo> FunctionCalls;

	FDecisionOutcome() {}
	FDecisionOutcome(const TArray<FFunctionInvokeInfo>& InFunctionCalls) : FunctionCalls(InFunctionCalls) {}
};

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAGameMasterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
    
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Game Master")
	void ReceiveDecisionFromAgent(AActor* AgentActor, const FString& Decision);

	// 处理从 AgentComponent 返回的确认决策过后，润色过的对话
	void ReceiveDialogueGenerated(AActor* AgentActor, const FString& Dialogue);
    
private:
	// 异步裁定合理性，检查游戏机制
	void AdjudicateDecisionAsync(const FDecisionContext& Context);

	// 异步决策回调函数
	void OnAdjudicateDecisionCompleted(const FDecisionContext& Context, const FString& JSONResponse);
    
	// 解析大模型回复的函数
	bool ParseDecisionResponse(const FString& JSONResponse, FString& Description, FString& EventDescription, bool& bIsValid, TArray<FFunctionInvokeInfo>& FunctionCalls);
    
	// 发回结果给NPC确认决策
	void ReturnDecisionToAgent(const FDecisionContext& Context, const FString& Outcome, bool bIsDecisionValid);
    
	// 负责分发对话并调动游戏机制
	void DistributeDialogueAndInvokeMechanisms(AActor* AgentActor, const FString& Dialogue, const TArray<FFunctionInvokeInfo>& FunctionCalls);
    
	// 调动游戏机制，并分发决策
	void ExecuteGameMechanism(const AActor* AgentActor, const FString& Decision);

private:
	// 存储 Function Calls，等角色确认后再调用
	UPROPERTY()
	TMap<AActor*, FDecisionOutcome> PendingFunctionCalls;
};