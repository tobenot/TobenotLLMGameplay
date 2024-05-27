// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TAFunctionInvokeSubsystem.h"
#include "TAGameMasterSubsystem.generated.h"

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
	void AdjudicateDecisionAsync(const AActor* AgentActor, const FString& Decision);

	// 异步决策回调函数
	void OnAdjudicateDecisionCompleted(const AActor* AgentActor, const FString& Decision, const FString& JSONResponse);

	// 解析大模型回复的函数
	bool ParseDecisionResponse(const FString& JSONResponse, FString& Description, FString& EventDescription, bool& bIsValid, TArray<FFunctionInvokeInfo>& FunctionCalls);

	// 发回结果给NPC确认决策
	void ReturnDecisionToAgent(AActor* AgentActor, const FString& Outcome, bool bIsDecisionValid);

	// 负责分发对话并调动游戏机制
	void DistributeDialogueAndInvokeMechanisms(AActor* AgentActor, const FString& Dialogue, const TArray<FFunctionInvokeInfo>& FunctionCalls);
    
	// 调动游戏机制，并分发决策
	void ExecuteGameMechanism(const AActor* AgentActor, const FString& Decision);
};