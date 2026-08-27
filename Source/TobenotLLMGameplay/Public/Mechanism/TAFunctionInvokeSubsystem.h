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

	TMap<FString, FString> ParsedParams;

	FFunctionInvokeInfo() {}

	FFunctionInvokeInfo(const FString& InFunctionName, const FString& InParams)
		: FunctionName(InFunctionName), Params(InParams) 
	{
		ParseJsonParams();
	}

	void ParseJsonParams()
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Params);

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			for (auto& Elem : JsonObject->Values)
			{
				ParsedParams.Add(Elem.Key, Elem.Value->AsString());
			}
		}
	}
};

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAFunctionInvokeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FString GetFunctionDescription(const FString& FunctionName) const;
	FString GetAllFunctionDescriptions() const;
	void InvokeFunction(const FString& FunctionName, AActor* AgentActor, const TMap<FString, FString>& Params);
	FString GenerateFunctionDescription(const TArray<FFunctionInvokeInfo>& FunctionCalls) const;

private:
	void ItemExchange(AActor* AgentActor, const TMap<FString, FString>& Params);
	void FinishSection(AActor* AgentActor, const TMap<FString, FString>& Params);
	void MoveToLocation(AActor* AgentActor, const TMap<FString, FString>& Params);
	void InvokeImmediateDecision(AActor* AgentActor, const TMap<FString, FString>& Params);

	AActor* FindActorByName(const FString& ActorName) const;

	TMap<FString, FString> FunctionDescriptions;
};