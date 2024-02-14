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
	FOnMessageFailed OnMessageFailed;
	
private:
	UFUNCTION()
	void HandleSuccessfulMessage(FChatCompletion Message);
	UFUNCTION()
	void HandleFailedMessage();
	
	void ProcessMessage(AActor* OriActor, FString UserMessage, UTAChatCallback* CallbackObject, bool IsSystemMessage);
	void CheckMessageQueue();
	
	UPROPERTY()
	TMap<AActor*, FTAActorChatHistory> ActorChatHistoryMap;

	UPROPERTY()
	class UTAChatCallback* CacheCallbackObject;

	UPROPERTY()
	class UOpenAIChat* CacheChat;

	UPROPERTY()
	TSet<AActor*> ActiveActors;
};