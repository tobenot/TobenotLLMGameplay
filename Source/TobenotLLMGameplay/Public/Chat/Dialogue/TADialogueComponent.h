// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/TAPromptDefinitions.h"
#include "TADialogueComponent.generated.h"

struct FChatCompletion;
class UTADialogueInstance;
struct FChatLog;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueReceivedMessageUpdated, const FChatCompletion&, ReceivedMessage, AActor*, Sender);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOBENOTLLMGAMEPLAY_API UTADialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 这个委托的sender意思是真正发送ReceivedMessage的人，不是促使它发送ReceivedMessage的人，注意区分
	UPROPERTY(BlueprintAssignable, Category = "TADialogueComponent")
	FOnDialogueReceivedMessageUpdated OnDialogueReceivedMessage; 

	// Constructor for the dialogue component
	UTADialogueComponent();

	// Static function to get dialogue component from an actor
	static UTADialogueComponent* GetTADialogueComponent(AActor* Actor);

	// Functions related to dialogue instance
	UFUNCTION(BlueprintCallable, Category = "TADialogueComponent")
	void SetCurrentDialogue(UTADialogueInstance* NewDialogueInstance);
	UFUNCTION(BlueprintCallable, Category = "TADialogueComponent")
	UTADialogueInstance* GetCurrentDialogueInstance() const{return CurrentDialogueInstance;};
	UFUNCTION(BlueprintCallable, Category = "TADialogueComponent")
	void HandleReceivedMessage(const FChatCompletion& ReceivedMessage, AActor* Sender);

	// Functions related to chat log history
	UFUNCTION(BlueprintCallable, Category = "TADialogueComponent")
	TArray<FChatLog> GetDialogueHistory() const{return DialogueHistory;};
	UFUNCTION(BlueprintCallable, Category = "TADialogueComponent")
	FString GetDialogueHistoryCompressedStr() const{return DialogueHistoryCompressedStr;};
	UFUNCTION(BlueprintCallable, Category = "TADialogueComponent")
	void SendMessageToDialogue(const FChatCompletion& Message);
	UFUNCTION(BlueprintCallable, Category = "TADialogueComponent")
	void RequestToSpeak();
	void RefuseToSay();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TADialogueComponent")
	bool IsPlayer;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TADialogueComponent")
	bool IsRequestingMessage;
protected:
	// Begins play for the component
	virtual void BeginPlay() override;

private:
	// Current dialogue instance
	UPROPERTY()
	UTADialogueInstance* CurrentDialogueInstance;

	// Chat log history within the component
	UPROPERTY()
	TArray<FChatLog> DialogueHistory;

	UPROPERTY()
	TArray<FChatLog> FullDialogueHistory;
	
	FString DialogueHistoryCompressedStr;
	
	// Handles updating the chat history
	void UpdateDialogueHistory(const FChatCompletion& NewChatLog);
	
	// Notifies UI of dialogue history update
	void NotifyUIOfDialogueHistoryUpdate(const FChatCompletion& ReceivedMessage, AActor* Sender);

	FString GetSystemPromptFromOwner() const;
	
private:
	UPROPERTY()
	class UOpenAIChat* CacheChat;

public:
	// 根据大语言模型的响应来执行游戏中的行为，默认关闭，需要继承UTAFunctionInvokeComponent做支持
	UPROPERTY()
	bool bEnableFunctionInvoke = false;
	
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
	void RequestDialogueCompression();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TADialogueComponent")
	bool bEnableCompressDialogue = true;
	
private:
	bool bIsCompressingDialogue = false;
	int32 LastCompressedIndex;
	
public:
	static const FTAPrompt PromptCompressDialogueHistory;
	
	FString JoinDialogueHistory();
};