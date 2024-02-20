// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAAreaScene.h"
#include "TAInteractiveActor.h"
#include "TASceneLogCategory.h"
#include "TASettings.h"
#include "Common/TALLMLibrary.h"
#include "Common/TASystemLibrary.h"
#include "Event/Data/TAEventInfo.h"

void UTAAreaScene::LoadAreaScene(const FTAEventInfo& EventInfo)
{
	if (!GetWorld())
	{
		UE_LOG(LogTASceneSystem, Error, TEXT("LoadAreaScene 无法获取world"));
		return;
	}

	TArray<FChatLog> TempMessagesList;
	FString SystemPrompt;
	
	const UTASettings* Settings = GetDefault<UTASettings>();
	if (Settings)
	{
		if (UClass* TAPromptSettingClass = Settings->TAPromptSetting.TryLoadClass<UObject>())
		{
			const UTAPromptSetting* DefaultPromptSettings = GetDefault<UTAPromptSetting>(TAPromptSettingClass);
			if (DefaultPromptSettings)
			{
				const FString EventInfoDes = EventInfo.Description;
				SystemPrompt =	UTALLMLibrary::PromptToStr(DefaultPromptSettings->PromptEventGenInteractables)
					.Replace(TEXT("{EventInfo}"), *EventInfoDes)
					.Replace(TEXT("{Language}"), *UTASystemLibrary::GetGameLanguage())
					;
			}
		}
	}
	if(SystemPrompt.IsEmpty())
	{
		UE_LOG(LogTASceneSystem, Error, TEXT("LoadAreaScene 生成交互物失败"));
		return;
	}
	
	TempMessagesList.Add({EOAChatRole::SYSTEM, SystemPrompt});
	FChatSettings ChatSettings{
		UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
		TempMessagesList
	};
	ChatSettings.jsonFormat = true;
	
	// 异步发送消息
	CacheChat = UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings, [this](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
	{
		if (Success)
		{
			TSharedPtr<FJsonObject> JsonObject;
			const TArray<TSharedPtr<FJsonValue>>* InteractablesArrayJson;
			InteractablesArray.Empty();
			if (JsonObject->TryGetArrayField(TEXT("Interactables"), InteractablesArrayJson))
			{
				// 遍历JSON数组
				for (int32 Index = 0; Index < InteractablesArrayJson->Num(); ++Index)
				{
					// 获取每个交互物的JSON对象
					TSharedPtr<FJsonObject> InteractableJson = (*InteractablesArrayJson)[Index]->AsObject();
					if (InteractableJson.IsValid())
					{
						// 创建结构体实例并填充数据
						FInteractableInfo InteractableInfo;
						InteractableInfo.Name = InteractableJson->GetStringField(TEXT("Name"));
						InteractableInfo.UniqueFeature = InteractableJson->GetStringField(TEXT("UniqueFeature"));
						InteractableInfo.Objective = InteractableJson->GetStringField(TEXT("Objective"));

						// 将填充好的结构体添加到数组中
						InteractablesArray.Add(InteractableInfo);
					}
				}

				// 生成交互物
				for (const FInteractableInfo& Interactable : InteractablesArray)
				{
					// 这里你可以访问Interactable.Name, Interactable.UniqueFeature, 和 Interactable.Objective
					ATAInteractiveActor* NewActor = GetWorld()->SpawnActor<ATAInteractiveActor>();
					if (NewActor)
					{
						UTAInteractionComponent* InteractionCom = NewActor->GetInteractionComponent();
						if (InteractionCom)
						{
							InteractionCom->InteractableInfo = Interactable;
						}
						InteractiveActors.Add(NewActor);
					}
				}
			}
		}
		CacheChat = nullptr;
	});
}
