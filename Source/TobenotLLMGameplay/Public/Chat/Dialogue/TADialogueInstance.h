// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TimerManager.h"
#include "TADialogueInstance.generated.h"

struct FChatCompletion;
struct FChatLog;
// 声明一个枚举，用于描述对话的状态
UENUM(BlueprintType)
enum class EDialogueState : uint8
{
    WaitingForParticipants UMETA(DisplayName = "WaitingForParticipants"),
    Active UMETA(DisplayName = "Active"),
    WaitingForSomeOne UMETA(DisplayName = "WaitingForSomeOne"),
    End UMETA(DisplayName = "End"),
};

/**
 * DialogueInstance represents an ongoing dialogue environment with multiple participants
 */
UCLASS(Blueprintable)
class TOBENOTLLMGAMEPLAY_API UTADialogueInstance : public UObject
{
    GENERATED_BODY()

public:
    UTADialogueInstance();

    // 唯一标识符
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue Instance")
    FGuid DialogueId;

    // 参与对话的Actors列表
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue Instance")
    TArray<AActor*> Participants;

    // 对话的历史记录
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue Instance")
    TArray<FChatLog> DialogueHistory;

    // 对话的状态
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue Instance")
    EDialogueState DialogueState;

    // 添加参与者
    UFUNCTION(BlueprintCallable, Category = "Dialogue Instance")
    void AddParticipant(AActor* Participant);

    // 移除参与者
    UFUNCTION(BlueprintCallable, Category = "Dialogue Instance")
    void RemoveParticipant(AActor* Participant);

    // 尝试更新对话状态
    UFUNCTION(BlueprintCallable, Category = "Dialogue Instance")
    void TryEnterDialogueState(EDialogueState NewState);

    // 添加消息到历史记录并触发消息分发
    UFUNCTION(BlueprintCallable, Category = "Dialogue Instance")
    void ReceiveMessage(const FChatCompletion& Message, AActor* Sender);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Instance")
    void RefuseToSay(AActor* Sender);
    
    UFUNCTION()
    void CycleParticipants();
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue Instance")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue Instance")
    TArray<FString> GetParticipantNamesFromAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue Instance")
    FString GetParticipantsNamesStringFromAgents() const;
    
private:
    // 内部方法用于添加消息到历史记录和分发消息给参与者
    void AddMessageToHistory(const FChatLog& Message, AActor* Sender);
    void DistributeMessage(const FChatCompletion& Message, AActor* Sender);
    	
    FTimerHandle DialogueTimerHandle;
    int32 CurrentParticipantIndex;
protected:
    virtual void BeginDestroy() override;
};