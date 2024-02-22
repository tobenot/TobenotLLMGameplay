// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OpenAIDefinitions.h"
#include "TAChatCallback.h"
#include "TAChatComponent.generated.h"

USTRUCT(BlueprintType)
struct FTAActorChatHistory
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Chat")
	TArray<FChatLog> ChatHistory;
};

USTRUCT(BlueprintType)
struct FTAChatComponentSaveData
{
	GENERATED_BODY()

	// 保存所有Actor的聊天历史映射
	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	TMap<FGuid, FTAActorChatHistory> SavedActorChatHistoryMap;
};
USTRUCT()
struct FTAActorMessageQueue
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> MessageQueue;

	UPROPERTY()
	UTAChatCallback* CallbackObject;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageSent, const FChatCompletion&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageSentWithSender, const FChatCompletion&, Message, AActor*, SenderCom);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMessageFailed);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TOBENOTLLMGAMEPLAY_API UTAChatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTAChatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category="TAChatWidget")
	static UTAChatComponent* GetTAChatComponent(AActor* Actor);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLM")
	bool ChatMessageJsonFormat = true;
	
	// Called every frame
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY()
	TMap<AActor*, FTAActorMessageQueue> ActorMessageQueueMap;
	
	UFUNCTION(BlueprintCallable, Category = "TAChatComponent")
	void SendMessageToOpenAI(AActor* OriActor, FString UserMessage, UTAChatCallback* CallbackObject, bool IsSystemMessage = false);
	
	UFUNCTION(BlueprintCallable, Category = "TAChatComponent")
	FString GetSystemPromptFromOwner() const;

	UFUNCTION(BlueprintCallable, Category = "TAChatComponent")
	TArray<FChatLog>& GetChatHistoryWithActor(AActor* OtherActor);

	UFUNCTION(BlueprintCallable, Category = "TAChatComponent")
	void ClearChatHistoryWithActor(AActor* OtherActor);

	UPROPERTY(BlueprintAssignable, Category = "TAChatComponent")
	FOnMessageSent OnMessageSent;
	
	UPROPERTY(BlueprintAssignable, Category = "TAChatComponent")
	FOnMessageSentWithSender OnMessageSentWithSender;

	UPROPERTY(BlueprintAssignable, Category = "TAChatComponent")
	FOnMessageFailed OnMessageFailed;
	
private:
	UFUNCTION()
	void HandleSuccessfulMessage(FChatCompletion Message,AActor* Sender);
	UFUNCTION()
	void HandleFailedMessage();
	
	void ProcessMessage(AActor* OriActor, FString UserMessage, UTAChatCallback* CallbackObject, bool IsSystemMessage);
	void CheckMessageQueue();
	
	UPROPERTY()
	TMap<FGuid, FTAActorChatHistory> ActorChatHistoryMap;
	
	UPROPERTY()
	TMap<AActor*, class UTAChatCallback*> CallbackMap; // 用于缓存 Callback 对象

	UPROPERTY()
	class UOpenAIChat* CacheChat;

	UPROPERTY()
	TSet<AActor*> ActiveActors;
	
public:
	// 根据大语言模型的响应来执行游戏中的行为，默认关闭，需要继承UTAFunctionInvokeComponent做支持
	UPROPERTY()
	bool bEnableFunctionInvoke = false;
	
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void PerformFunctionInvokeBasedOnResponse(const FString& Response);

public:
	UFUNCTION(BlueprintCallable, Category = "ChatHistorySave")
	void SetChatHistoryData(const FTAChatComponentSaveData& NewData);
	
	UFUNCTION(BlueprintCallable, Category = "ChatHistorySave")
	FTAChatComponentSaveData GetChatHistoryData() const;
};