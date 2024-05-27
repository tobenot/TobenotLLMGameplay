// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Chat/TAInteractionComponent.h"
#include "UObject/Object.h"
#include "TAAreaScene.generated.h"

struct FTAEventInfo;
class ATAInteractiveActor;
/**
 * 
 */
UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAAreaScene : public UObject
{
	GENERATED_BODY()
public:
	// 加载区域地图时调用，生成交互点
	void LoadAreaScene(const FTAEventInfo& EventInfo);

protected:
	// 存储所有生成的交互点actors
	TArray<ATAInteractiveActor*> InteractiveActors;
	
	TArray<FInteractableInfo> InteractablesArray;

private:
	UPROPERTY()
	class UOpenAIChat* CacheChat;
};
