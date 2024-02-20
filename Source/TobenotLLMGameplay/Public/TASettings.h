// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "TAPromptSetting.h"
#include "Engine/DeveloperSettings.h"
#include "TASettings.generated.h"

/**
 * 
 */
UCLASS(config = Game, defaultconfig)
class TOBENOTLLMGAMEPLAY_API UTASettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// 设置要使用的事件生成器的类名。需要是 UTAEventGenerator 的子类
	UPROPERTY(config, EditAnywhere, Category="Event")
	FSoftClassPath EventGeneratorClass;

	// 设置要使用的ATAPlaceActor的子类的类名。
	UPROPERTY(config, EditAnywhere, Category="Scene")
	FSoftClassPath PlaceActorClass;

	// 设置要使用的Prompt模板，UTAPromptSetting子类
	UPROPERTY(config, EditAnywhere, Category="Prompt")
	FSoftClassPath TAPromptSetting;
};
