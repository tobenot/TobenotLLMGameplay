// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// 简单Agent类，实际使用可能要配合上项目自己的FunctionInvoke组件

#pragma once

#include "CoreMinimal.h"
#include "TAAgentInterface.h"
#include "TobenotLLMGameplay/Common/TASystemLibrary.h"
#include "GameFramework/Actor.h"
#include "TobenotLLMGameplay/Save/TAGuidInterface.h"
#include "TANarrativeAgent.generated.h"

class UTAShoutComponent;
class UTAFunctionInvokeComponent;
class UTAAgentComponent;

UENUM(BlueprintType)
enum class EPromptType : uint8
{
	Simple				UMETA(DisplayName = "Simple"),
	Environment			UMETA(DisplayName = "Environment"),
	InteractiveObject	UMETA(DisplayName = "InteractiveObject")
};

USTRUCT(BlueprintType)
struct FNarrativeAgentData : public FTableRowBase
{
	GENERATED_BODY()
    
	// Agent的名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AgentName;

	// 系统提示模板种类
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPromptType  SystemPromptType;

	// 提示模板使用的参数
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> SystemPromptParameters;

	//备注
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Note;
};

USTRUCT(BlueprintType)
struct FEnvironmentPromptTemplate
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Hint;

	FString BuildPrompt() const
	{
		return FString::Printf(TEXT("%s. %s"), *Description, *Hint);
	}
};

USTRUCT(BlueprintType)
struct FInteractiveObjectPromptTemplate
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ObjectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString InteractionAction;

    FString BuildPrompt() const
    {
        return FString::Printf(TEXT("This is a %s. To use it, you must %s."), *ObjectName);
    }
};

USTRUCT(BlueprintType)
struct FSimplePromptTemplate
{
	GENERATED_BODY()

	// 使用参数创建直接拼接的系统提示
	FString BuildPrompt(const TArray<FString>& Parameters) const
	{
		FString Prompt;
		for (const FString& Param : Parameters)
		{
			Prompt += Param;
		}
		Prompt += "Please use the following JSON template for your response:"
		"\"Response Template\": {"
		"\"message\": \"A string that vividly describes the ongoing scene, situation or adventurer's actions. You can use metaphoric or metaphorical phrases for immersion. Remember to keep the narration insightful and intriguing, without showing the direct speech from either the adventurer or the NPCs.\","
			"\"func_invoke\": ["
				"{"
					"\"name\": \"XXX\","
					"\"depict\": \"XXX\","
					"//Add additional function invokes to the array as per the narration's requirement."
				"}"
			"]"
		"}. Response message in "
		+ UTASystemLibrary::GetGameLanguage()
		;
		return Prompt;
	}
};

UCLASS()
class TOBENOTLLMGAMEPLAY_API ATANarrativeAgent : public AActor
	,public ITAAgentInterface
	,public ITAGuidInterface
{
	GENERATED_BODY()
    
public:
	UPROPERTY(EditDefaultsOnly, Category = "Narrative Agent")
	TSubclassOf<UTAFunctionInvokeComponent> FunctionInvokeCompClass;
	
	// 构造函数
	ATANarrativeAgent();
	
	UFUNCTION(BlueprintCallable, Category = "Narrative Agent")
	virtual void InitAgentByID(int32 NewAgentID);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Narrative Agent")
	void InitAgentByID_BP();
	
protected:
	virtual void BeginPlay();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Narrative Agent")
	int32 AgentID;
	
	// Agent的名字
	UPROPERTY(VisibleAnywhere, Category="Narrative Agent")
	FString AgentName;

	// 系统提示
	UPROPERTY(VisibleAnywhere, Category="Narrative Agent")
	FString SystemPrompt;

	// 定位是子类赋值它，比如放调用函数的列表什么的？
	UPROPERTY(VisibleAnywhere, Category="Narrative Agent")
	FString AppendSystemPrompt;
	
	// 系统提示使用的参数
	UPROPERTY(VisibleAnywhere, Category="Narrative Agent")
	TArray<FString> SystemPromptParameters;
	
	UPROPERTY()
	TMap<FGuid, FString> DesireMap;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Prompts")
	FString TotalDesire;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Narrative Agent")
	bool bIsVoiceover;
	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Narrative Agent")
	UDataTable* GetAgentDataTable() const;
	
	// 获取系统提示
	virtual FString GetSystemPrompt() override;
    
	// 获取Agent的名字
	virtual const FString& GetAgentName() const override;

	// 增加或更新Agent的欲望
	UFUNCTION(BlueprintCallable, Category = "Narrative Agent")
	virtual void AddOrUpdateDesire(const FGuid& DesireId, const FString& DesireDescription) override;

	// 移除Agent的欲望
	UFUNCTION(BlueprintCallable, Category = "Narrative Agent")
	virtual void RemoveDesire(const FGuid& DesireId) override;
	
	virtual bool IsVoiceover() const;
private:
	FString GenerateSystemPrompt(EPromptType PromptType, const TArray<FString>& Parameters);

protected:
	// Shout组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTAShoutComponent* ShoutComponent;
	
	// Agent组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTAAgentComponent* AgentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTAFunctionInvokeComponent * FunctionInvokeComponent;
};