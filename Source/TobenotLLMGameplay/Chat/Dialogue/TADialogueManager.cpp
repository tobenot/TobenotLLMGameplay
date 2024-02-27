// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// DialogueManager.cpp
#include "TADialogueManager.h"

#include "TADialogueComponent.h"

UTADialogueInstance* UTADialogueManager::CreateDialogueInstance(UObject* WorldContext)
{
	UTADialogueInstance* NewDialogueInstance = NewObject<UTADialogueInstance>(WorldContext);
	NewDialogueInstance->DialogueId = FGuid::NewGuid();
	DialogueInstances.Add(NewDialogueInstance->DialogueId, NewDialogueInstance);
	return NewDialogueInstance;
}

void UTADialogueManager::DestroyDialogueInstance(FGuid DialogueId)
{
	UTADialogueInstance** DialogueInstance = DialogueInstances.Find(DialogueId);
	if (DialogueInstance)
	{
		DialogueInstances.Remove(DialogueId);
		(*DialogueInstance)->ConditionalBeginDestroy();  // 注意：这里要确保会话实例不被其他地方引用
	}
}

UTADialogueInstance* UTADialogueManager::GetDialogueInstance(const FGuid& DialogueId)
{
	UTADialogueInstance** DialogueInstance = DialogueInstances.Find(DialogueId);
	if (DialogueInstance)
	{
		return *DialogueInstance;
	}
	return nullptr;
}

bool UTADialogueManager::AcceptJoinRequest(AActor* JoiningActor, UTADialogueInstance* DialogueInstance)
{
	if (DialogueInstance && JoiningActor)
	{
		DialogueInstance->AddParticipant(JoiningActor);
		return true;
	}
	return false;
}

bool UTADialogueManager::InviteToDialogue(AActor* InvitedActor, UTADialogueInstance* DialogueInstance)
{
	if (DialogueInstance && InvitedActor)
	{
		UTADialogueComponent* DialogueComp = UTADialogueComponent::GetTADialogueComponent(InvitedActor);
		if(DialogueComp)
		{
			DialogueComp->SetCurrentDialogue(DialogueInstance);
			DialogueInstance->AddParticipant(InvitedActor);
		}
		return true;
	}
	return false;
}