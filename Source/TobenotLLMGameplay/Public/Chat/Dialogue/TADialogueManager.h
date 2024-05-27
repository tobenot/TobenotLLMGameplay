// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TADialogueInstance.h"
#include "Subsystems/WorldSubsystem.h"
#include "TADialogueManager.generated.h"

// DialogueManager.h
UCLASS()
class TOBENOTLLMGAMEPLAY_API UTADialogueManager : public UWorldSubsystem
{
	GENERATED_BODY()
    
public:
	// 创建对话实例
	UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
	UTADialogueInstance* CreateDialogueInstance(UObject* WorldContext);

	// 销毁对话实例
	UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
	void DestroyDialogueInstance(FGuid DialogueId);

	// 获取指定对话实例
	UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
	UTADialogueInstance* GetDialogueInstance(const FGuid& DialogueId);

	// 受理加入对话请求
	UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
	bool AcceptJoinRequest(AActor* JoiningActor, UTADialogueInstance* DialogueInstance);

	// 发送对话邀请（实现在后面提供）
	UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
	bool InviteToDialogue(AActor* InvitedActor, UTADialogueInstance* DialogueInstance);

private:
	UPROPERTY()
	TMap<FGuid, UTADialogueInstance*> DialogueInstances;
};