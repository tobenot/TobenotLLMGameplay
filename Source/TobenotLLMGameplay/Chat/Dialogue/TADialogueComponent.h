// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TADialogueComponent.generated.h"

class UTADialogueInstance;
struct FChatLog;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueReceivedMessageUpdated, const FChatLog&, ReceivedMessage, AActor*, Sender);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOBENOTLLMGAMEPLAY_API UTADialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
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
	void HandleReceivedMessage(const FChatLog& ReceivedMessage, AActor* Sender);

	// Functions related to chat log history
	UFUNCTION(BlueprintCallable, Category = "TADialogueComponent")
	TArray<FChatLog> GetDialogueHistory() const{return DialogueHistory;};
	UFUNCTION(BlueprintCallable, Category = "TADialogueComponent")
	void SendMessageToDialogue(const FString& UserMessage);
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

	// Handles updating the chat history
	void UpdateDialogueHistory(const FChatLog& NewChatLog);

	// Notifies UI of dialogue history update
	void NotifyUIOfDialogueHistoryUpdate(const FChatLog& ReceivedMessage, AActor* Sender);

	FString GetSystemPromptFromOwner() const;
	
private:
	UPROPERTY()
	class UOpenAIChat* CacheChat;
};