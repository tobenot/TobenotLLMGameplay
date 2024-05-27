#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TAFunctionInvokeSubsystem.generated.h"

USTRUCT()
struct FFunctionInvokeInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FString FunctionName;

	UPROPERTY()
	FString Params;

	FFunctionInvokeInfo() {}

	FFunctionInvokeInfo(const FString& InFunctionName, const FString& InParams)
		: FunctionName(InFunctionName), Params(InParams) {}
};

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAFunctionInvokeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static FString GetFunctionDescription(const FString& FunctionName);
	static FString GetAllFunctionDescriptions();
	void InvokeFunction(const FString& FunctionName, AActor* AgentActor, const FString& Params);
	FString GenerateFunctionDescription(const TArray<FFunctionInvokeInfo>& FunctionCalls);

private:
	static void ItemExchange(AActor* AgentActor, const FString& Params);
	static void FinishSection(AActor* AgentActor, const FString& Params);
	static void MoveToLocation(AActor* AgentActor, const FString& Params);

	static TMap<FString, FString> FunctionDescriptions;
};