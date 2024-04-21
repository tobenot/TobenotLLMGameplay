// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TobenotLLMGameplay/Common/TAPromptDefinitions.h"
#include "TAShoutComponent.generated.h"

struct FChatCompletion;
class UTAShoutInstance;
struct FChatLog;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShoutReceivedMessageUpdated, const FChatCompletion&, ReceivedMessage, AActor*, Sender);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProvidePlayerChoices, const TArray<FString>&, Choices);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOBENOTLLMGAMEPLAY_API UTAShoutComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 这个委托的sender意思是真正发送ReceivedMessage的人，不是促使它发送ReceivedMessage的人，注意区分
	UPROPERTY(BlueprintAssignable, Category = "TAShoutComponent")
	FOnShoutReceivedMessageUpdated OnShoutReceivedMessage;
	
	// 委托：提供给玩家的备选回复项目
	UPROPERTY(BlueprintAssignable, Category = "TAShoutComponent")
	FOnProvidePlayerChoices OnProvidePlayerChoices;
	
	// Constructor for the Shout component
	UTAShoutComponent();

	// Static function to get Shout component from an actor
	static UTAShoutComponent* GetTAShoutComponent(AActor* Actor);
	
	UFUNCTION(BlueprintCallable, Category = "TAShoutComponent")
	void HandleShoutReceived(const FChatCompletion& Message, AActor* Shouter, float Volume);
	UFUNCTION(BlueprintCallable, Category = "TAShoutComponent")
	void HandleReceivedMessage(const FChatCompletion& ReceivedMessage, AActor* Sender);

	// Functions related to chat log history
	UFUNCTION(BlueprintCallable, Category = "TAShoutComponent")
	TArray<FChatLog> GetShoutHistory() const{return ShoutHistory;};
	UFUNCTION(BlueprintCallable, Category = "TAShoutComponent")
	FString GetShoutHistoryCompressedStr() const{return ShoutHistoryCompressedStr;};
	UFUNCTION(BlueprintCallable, Category = "TAShoutComponent")
	void ShoutMessage(const FChatCompletion& Message, float Volume = 100.f);
	UFUNCTION(BlueprintCallable, Category = "TAShoutComponent")
	FString GetNearbyAgentNames();
	UFUNCTION(BlueprintCallable, Category = "TAShoutComponent")
	void RequestToSpeak();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TAShoutComponent")
	bool IsPlayer;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TAShoutComponent")
	bool IsRequestingMessage;
protected:
	// Begins play for the component
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	// Chat log history within the component
	UPROPERTY()
	TArray<FChatLog> ShoutHistory;

	UPROPERTY()
	TArray<FChatLog> FullShoutHistory;
	
	FString ShoutHistoryCompressedStr;
	
	// Handles updating the chat history
	void UpdateShoutHistory(const FChatCompletion& NewChatLog);
	
	// Notifies UI of Shout history update
	void NotifyUIOfShoutHistoryUpdate(const FChatCompletion& ReceivedMessage, AActor* Sender);

	FString GetSystemPromptFromOwner() const;
	
private:
	UPROPERTY()
	class UOpenAIChat* CacheChat;

public:
	// 根据大语言模型的响应来执行游戏中的行为，需要UTAFunctionInvokeComponent做支持
	UPROPERTY()
	bool bEnableFunctionInvoke = true;
	
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void PerformFunctionInvokeBasedOnResponse(const FString& Response);

public:
	UFUNCTION(BlueprintCallable, Category = "Chat")
	bool GetAcceptMessages() const{return bAcceptMessages;}

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SetAcceptMessages(bool bInAcceptMessages);
	
private:
	// 是否接受消息的标志位
	bool bAcceptMessages = true;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void RequestShoutCompression();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TAShoutComponent")
	bool bEnableCompressShout = true;
	
private:
	bool bIsCompressingShout = false;
	int32 LastCompressedIndex;
	
public:
	static const FTAPrompt PromptCompressShoutHistory;
	
	FString JoinShoutHistory();

public:
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void RequestChoices();
	TArray<FString> ParseChoicesFromResponse(const FString& Response);
};