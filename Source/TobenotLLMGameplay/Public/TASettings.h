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

	// 设置要使用的交互Actor类，ATAInteractiveActor子类
	UPROPERTY(config, EditAnywhere, Category="Event")
	FSoftClassPath InteractiveActorClass;
	
	// 设置要使用的交互组件类，UTAInteractionComponent子类
	UPROPERTY(config, EditAnywhere, Category="Event")
	FSoftClassPath InteractionComponentClass;
	
	// 设置要使用的怪物类，UAActor类的子类
	UPROPERTY(config, EditAnywhere, Category = "Scene")
	FSoftClassPath MonsterClass;
};
